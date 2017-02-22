/*
 * Copyright(C) 2016 iCub Facility
 * Authors: Francesco Romano
 * CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#include "XsensMVNRemote.h"

#include <thrift/XsensSegmentsFrame.h>
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
            public yarp::os::TypedReaderCallback<xsens::XsensSegmentsFrame>
        {
        public:

            XsensMVNRemotePrivate()
            : m_segmentsCount(0) {}

            virtual ~XsensMVNRemotePrivate() {}

            yarp::os::BufferedPort<xsens::XsensSegmentsFrame> m_inputPort;
            yarp::os::Port m_commandPort;
            xsens::XsensDriverService m_xsensService;
            yarp::os::ConstString m_remoteStreamingPortName;
            yarp::os::ConstString m_remoteCommandPortName;

            //Buffers for read & associated mutex
            yarp::os::Mutex m_mutex;
            std::vector<yarp::sig::Vector> m_poses;
            std::vector<yarp::sig::Vector> m_velocities;
            std::vector<yarp::sig::Vector> m_accelerations;
            yarp::experimental::dev::IFrameProviderStatus m_status;
            yarp::os::Stamp m_timestamp;

            unsigned m_segmentsCount;

            virtual void onRead(xsens::XsensSegmentsFrame& frame)//, const yarp::os::TypedReader<xsens::XsensFrame> &reader)
            {
                yarp::os::LockGuard guard(m_mutex);
                //get timestamp
                m_inputPort.getEnvelope(m_timestamp);
                m_status = static_cast<yarp::experimental::dev::IFrameProviderStatus>(frame.status);

                //if status is != OK we should not have any data
                if (m_status != yarp::experimental::dev::IFrameProviderStatusOK) return;

                for (unsigned seg = 0; seg < m_segmentsCount; ++seg) {
                    xsens::Vector3 &position = frame.segmentsData[seg].position;
                    xsens::Quaternion &orientation = frame.segmentsData[seg].orientation;
                    xsens::Vector3 &linVelocity = frame.segmentsData[seg].velocity;
                    xsens::Vector3 &angVelocity = frame.segmentsData[seg].angularVelocity;
                    xsens::Vector3 &linAcceleration = frame.segmentsData[seg].acceleration;
                    xsens::Vector3 &angAcceleration = frame.segmentsData[seg].angularAcceleration;

                    yarp::sig::Vector &newPose = m_poses[seg];
                    yarp::sig::Vector &newVelocity = m_velocities[seg];
                    yarp::sig::Vector &newAcceleration = m_accelerations[seg];

                    newPose[0] = position.x;
                    newPose[1] = position.y;
                    newPose[2] = position.z;
                    newPose[3] = orientation.w;
                    newPose[4] = orientation.imaginary.x;
                    newPose[5] = orientation.imaginary.y;
                    newPose[6] = orientation.imaginary.z;

                    newVelocity[0] = linVelocity.x;
                    newVelocity[1] = linVelocity.y;
                    newVelocity[2] = linVelocity.z;
                    newVelocity[3] = angVelocity.x;
                    newVelocity[4] = angVelocity.y;
                    newVelocity[5] = angVelocity.z;

                    newAcceleration[0] = linAcceleration.x;
                    newAcceleration[1] = linAcceleration.y;
                    newAcceleration[2] = linAcceleration.z;
                    newAcceleration[3] = angAcceleration.x;
                    newAcceleration[4] = angAcceleration.y;
                    newAcceleration[5] = angAcceleration.z;
                    
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
            yarp::os::LockGuard guard(m_pimpl->m_mutex);

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
            unsigned segmentCount = this->getFrameCount();
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
            yarp::os::LockGuard guard(m_pimpl->m_mutex);

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


        // IFrameProvider interface
        std::vector<yarp::experimental::dev::FrameReference> XsensMVNRemote::frames()
        {
            assert(m_pimpl);
            std::vector<xsens::FrameReferece> frames = m_pimpl->m_xsensService.segments();
            std::vector<yarp::experimental::dev::FrameReference> deserializedFrames;
            deserializedFrames.reserve(frames.size());
            std::for_each(frames.begin(), frames.end(), [&](xsens::FrameReferece& frame) {
                yarp::experimental::dev::FrameReference referenceFrame;
                referenceFrame.frameReference = frame.frameReference;
                referenceFrame.frameName = frame.frameName;
                deserializedFrames.push_back(referenceFrame);
            });

            return deserializedFrames;
        }

        // Get Data
        yarp::experimental::dev::IFrameProviderStatus XsensMVNRemote::getFramePoses(std::vector<yarp::sig::Vector>& segmentPoses)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentPoses = m_pimpl->m_poses;
            return m_pimpl->m_status;

        }

        yarp::experimental::dev::IFrameProviderStatus XsensMVNRemote::getFrameVelocities(std::vector<yarp::sig::Vector>& segmentVelocities)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentVelocities = m_pimpl->m_velocities;
            return m_pimpl->m_status;
        }

        yarp::experimental::dev::IFrameProviderStatus XsensMVNRemote::getFrameAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentAccelerations = m_pimpl->m_accelerations;
            return m_pimpl->m_status;
        }

        yarp::experimental::dev::IFrameProviderStatus XsensMVNRemote::getFrameInformation(std::vector<yarp::sig::Vector>& segmentPoses,
                                                 std::vector<yarp::sig::Vector>& segmentVelocities,
                                                 std::vector<yarp::sig::Vector>& segmentAccelerations)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentPoses = m_pimpl->m_poses;
            segmentVelocities = m_pimpl->m_velocities;
            segmentAccelerations = m_pimpl->m_accelerations;
            return m_pimpl->m_status;
        }
        
        // IXsensMVNInterface interface
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

        std::map<std::string, double> XsensMVNRemote::bodyDimensions()
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

    }
}
