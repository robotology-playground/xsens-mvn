/*
 * Copyright(C) 2016 iCub Facility
 * Authors: Francesco Romano
 * CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#include "XsensMVNRemote.h"

#include <thrift/XsensFrame.h>
#include <thrift/XsensDriverService.h>

#include <yarp/os/Searchable.h>
#include <yarp/os/LockGuard.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Port.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/Value.h>
#include <yarp/sig/Vector.h>

#include <vector>
#include <cassert>
#include <algorithm>

namespace yarp {
    namespace dev {

        class XsensMVNRemote::XsensMVNRemotePrivate :
            public yarp::os::TypedReaderCallback<xsens::XsensFrame>
        {
        public:

            XsensMVNRemotePrivate()
            : m_segmentsCount(0) {}

            virtual ~XsensMVNRemotePrivate() {}

            yarp::os::BufferedPort<xsens::XsensFrame> m_inputPort;
            yarp::os::Port m_commandPort;
            xsens::XsensDriverService m_xsensService;
            yarp::os::ConstString m_remoteStreamingPortName;
            yarp::os::ConstString m_remoteCommandPortName;

            //Buffers for read & associated mutex
            yarp::os::Mutex m_mutex;
            std::vector<yarp::sig::Vector> m_poses;
            std::vector<yarp::sig::Vector> m_velocities;
            std::vector<yarp::sig::Vector> m_accelerations;
            yarp::os::Stamp m_timestamp;

            unsigned m_segmentsCount;

            virtual void onRead(xsens::XsensFrame& frame)//, const yarp::os::TypedReader<xsens::XsensFrame> &reader)
            {
                yarp::os::LockGuard guard(m_mutex);
                //get timestamp
                //TODO: direi di si ma guarderei cosa fa getEnvelope per evitare casini
                m_inputPort.getEnvelope(m_timestamp);

                for (unsigned seg = 0; seg < m_segmentsCount; ++seg) {
                    xsens::Vector3 &position = frame.segmentsData[seg].position;
                    xsens::Vector4 &orientation = frame.segmentsData[seg].orientation;
                    xsens::Vector3 &linVelocity = frame.segmentsData[seg].velocity;
                    xsens::Vector3 &angVelocity = frame.segmentsData[seg].angularVelocity;
                    xsens::Vector3 &linAcceleration = frame.segmentsData[seg].acceleration;
                    xsens::Vector3 &angAcceleration = frame.segmentsData[seg].angularAcceleration;

                    yarp::sig::Vector &newPose = m_poses[seg];
                    yarp::sig::Vector &newVelocity = m_velocities[seg];
                    yarp::sig::Vector &newAcceleration = m_accelerations[seg];

                    newPose[0] = position.c1;
                    newPose[1] = position.c2;
                    newPose[2] = position.c3;
                    newPose[3] = orientation.c1;
                    newPose[4] = orientation.c2;
                    newPose[5] = orientation.c3;
                    newPose[6] = orientation.c4;

                    newVelocity[0] = linVelocity.c1;
                    newVelocity[1] = linVelocity.c2;
                    newVelocity[2] = linVelocity.c3;
                    newVelocity[3] = angVelocity.c1;
                    newVelocity[4] = angVelocity.c2;
                    newVelocity[5] = angVelocity.c3;

                    newAcceleration[0] = linAcceleration.c1;
                    newAcceleration[1] = linAcceleration.c2;
                    newAcceleration[2] = linAcceleration.c3;
                    newAcceleration[3] = angAcceleration.c1;
                    newAcceleration[4] = angAcceleration.c2;
                    newAcceleration[5] = angAcceleration.c3;
                    
                }
            }
        };

        XsensMVNRemote::XsensMVNRemote()
            : m_pimpl(new XsensMVNRemotePrivate()) {}

        XsensMVNRemote::~XsensMVNRemote()
        {
            assert(m_pimpl);

            delete m_pimpl;
            m_pimpl = 0;
        }


        bool XsensMVNRemote::open(yarp::os::Searchable &config)
        {
            assert(m_pimpl);
            yarp::os::ConstString deviceName = config.check("local", yarp::os::Value("/xsens_remote"), "Checking device name").asString();
            if (deviceName.empty() || deviceName.at(0) != '/') {
                yError("Invalid device name '%s'", deviceName.c_str());
                return false;
            }

            if (!m_pimpl->m_inputPort.open(deviceName + "/frames:i")) {
                yError("Could not open streaming input port");
                return false;
            }

            if (!m_pimpl->m_commandPort.open(deviceName + "/cmd:o")) {
                yError("Could not open command output port");
                return false;
            }

            yarp::os::ConstString remote = config.find("remote").asString();
            if (remote.empty() || remote[0] != '/') {
                yError("Invalid remote name %s", remote.c_str());
                close();
                return false;
            }
            m_pimpl->m_remoteCommandPortName = remote + "/cmd:i";
            m_pimpl->m_remoteStreamingPortName = remote + "/frames:o";

            yarp::os::ConstString carrier = config.check("carrier", yarp::os::Value("udp"), "Checking streaming connection carrier. Default udp").asString();

            bool result = yarp::os::Network::connect(m_pimpl->m_remoteStreamingPortName.c_str(), m_pimpl->m_inputPort.getName().c_str(), carrier.c_str());
            result = result && yarp::os::Network::connect(m_pimpl->m_commandPort.getName(), m_pimpl->m_remoteCommandPortName);

            if (!result) {
                yError("Error while establishing connection to remote (%s) ports", remote.c_str());
                close();
                return false;
            }

            if (!this->m_pimpl->m_xsensService.yarp().attachAsClient(m_pimpl->m_commandPort)) {
                yError("Error while connecting to remote rpc port");
                close();
                return false;
            }

            //Obtain information regarding size of data
            unsigned segmentCount = this->getSegmentCount();
            m_pimpl->m_poses.reserve(segmentCount);
            m_pimpl->m_velocities.reserve(segmentCount);
            m_pimpl->m_accelerations.reserve(segmentCount);
            m_pimpl->m_segmentsCount = segmentCount;

            for (unsigned i = 0; i < segmentCount; ++i) {
                m_pimpl->m_poses.push_back(yarp::sig::Vector(7, 0.0));
                m_pimpl->m_velocities.push_back(yarp::sig::Vector(6, 0.0));
                m_pimpl->m_accelerations.push_back(yarp::sig::Vector(6, 0.0));
            }

            //register for callbacks
            m_pimpl->m_inputPort.useCallback(*m_pimpl);

            return result;
        }
        bool XsensMVNRemote::close()
        {
            assert(m_pimpl);

            m_pimpl->m_inputPort.disableCallback();

            bool result = yarp::os::Network::disconnect(m_pimpl->m_remoteStreamingPortName, m_pimpl->m_inputPort.getName());
            result = result && yarp::os::Network::disconnect(m_pimpl->m_commandPort.getName(), m_pimpl->m_remoteCommandPortName);

            m_pimpl->m_inputPort.close();
            m_pimpl->m_commandPort.close();

            return true;
        }

        // IPreciselyTimed interface
        yarp::os::Stamp XsensMVNRemote::getLastInputStamp()
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            return m_pimpl->m_timestamp;;
        }


        // IHumanSkeleton interface
        unsigned XsensMVNRemote::getSegmentCount() const
        {
            return segmentNames().size();
        }

        std::string XsensMVNRemote::segmentNameAtIndex(unsigned segmentIndex) const
        {
            assert(m_pimpl);
            std::vector<std::string> segments = segmentNames();
            if (segmentIndex >= segments.size()) return "";
            return segments[segmentIndex];
        }

        std::vector<std::string> XsensMVNRemote::segmentNames() const
        {
            assert(m_pimpl);
            return m_pimpl->m_xsensService.segments();
        }

        int XsensMVNRemote::segmentIndexForName(const std::string& name) const
        {
            assert(m_pimpl);
            std::vector<std::string> segments = segmentNames();
            std::vector<std::string>::iterator found = std::find(segments.begin(), segments.end(), name);
            if (found == segments.end()) return -1;
            return std::distance(segments.begin(), found);
        }

        //Configuration
        bool XsensMVNRemote::setBodyDimensions(const std::map<std::string, double>& dimensions)
        {
            assert(m_pimpl);
            return m_pimpl->m_xsensService.setBodyDimensions(dimensions);
        }

        bool XsensMVNRemote::setBodyDimension(const std::string& bodyPart, const double dimension)
        {
            assert(m_pimpl);
            return m_pimpl->m_xsensService.setBodyDimension(bodyPart, dimension);
        }

        std::map<std::string, double> XsensMVNRemote::bodyDimensions() const
        {
            assert(m_pimpl);
            return m_pimpl->m_xsensService.bodyDimensions();
        }

        // Calibration methods
        bool XsensMVNRemote::calibrate(const std::string &calibrationType)
        {
            assert(m_pimpl);
            if (calibrationType.empty())
                return m_pimpl->m_xsensService.calibrate();
            return m_pimpl->m_xsensService.calibrateWithType(calibrationType);
        }

        bool XsensMVNRemote::abortCalibration()
        {
            assert(m_pimpl);
            m_pimpl->m_xsensService.abortCalibration();
            return true;
        }

        //Acquisition methods
        bool XsensMVNRemote::startAcquisition()
        {
            assert(m_pimpl);
            m_pimpl->m_xsensService.startAcquisition();
            return true;
        }

        bool XsensMVNRemote::stopAcquisition()
        {
            assert(m_pimpl);
            m_pimpl->m_xsensService.stopAcquisition();
            return true;
        }

        // Get Data
        bool XsensMVNRemote::getSegmentPoses(std::vector<yarp::sig::Vector>& segmentPoses)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentPoses = m_pimpl->m_poses;
            return true;

        }

        bool XsensMVNRemote::getSegmentVelocities(std::vector<yarp::sig::Vector>& segmentVelocities)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentVelocities = m_pimpl->m_velocities;
            return true;
        }

        bool XsensMVNRemote::getSegmentAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentAccelerations = m_pimpl->m_accelerations;
            return true;
        }

        bool XsensMVNRemote::getSegmentInformation(std::vector<yarp::sig::Vector>& segmentPoses,
                                                   std::vector<yarp::sig::Vector>& segmentVelocities,
                                                   std::vector<yarp::sig::Vector>& segmentAccelerations)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentPoses = m_pimpl->m_poses;
            segmentVelocities = m_pimpl->m_velocities;
            segmentAccelerations = m_pimpl->m_accelerations;
            return true;
        }

    }
}
