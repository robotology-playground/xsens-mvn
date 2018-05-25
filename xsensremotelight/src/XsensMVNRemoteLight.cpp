/*
 * Copyright(C) 2016 iCub Facility
 * Authors: Francesco Romano
 * CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#include "XsensMVNRemoteLight.h"

#include <thrift/XsensSegmentsFrame.h>

#include <yarp/os/BufferedPort.h>
#include <yarp/os/LockGuard.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/Port.h>
#include <yarp/os/Searchable.h>
#include <yarp/os/Value.h>
#include <yarp/sig/Vector.h>

#include <algorithm>
#include <cassert>
#include <vector>

namespace yarp {
    namespace dev {

        static bool
        parseFrameListOption(const yarp::os::Value& option,
                             std::vector<yarp::experimental::dev::FrameReference>& parsedFrames);

        class XsensMVNRemoteLight::XsensMVNRemoteLightPrivate
            : public yarp::os::TypedReaderCallback<xsens::XsensSegmentsFrame>
        {
        public:
            yarp::os::BufferedPort<xsens::XsensSegmentsFrame> m_inputPort;
            yarp::os::ConstString m_remoteStreamingPortName;

            // Buffers for read & associated mutex
            yarp::os::Mutex m_mutex;
            std::vector<yarp::sig::Vector> m_poses;
            std::vector<yarp::sig::Vector> m_velocities;
            std::vector<yarp::sig::Vector> m_accelerations;
            yarp::experimental::dev::IFrameProviderStatus m_status;
            yarp::os::Stamp m_timestamp;

            std::vector<yarp::experimental::dev::FrameReference> m_frames;

            unsigned m_segmentsCount;

            XsensMVNRemoteLightPrivate()
                : m_status(yarp::experimental::dev::IFrameProviderStatusNoData)
                , m_segmentsCount(0)
            {}

            virtual ~XsensMVNRemoteLightPrivate() {}

            virtual void onRead(xsens::XsensSegmentsFrame& frame)
            {
                yarp::os::LockGuard guard(m_mutex);
                // get timestamp
                m_inputPort.getEnvelope(m_timestamp);
                m_status = static_cast<yarp::experimental::dev::IFrameProviderStatus>(frame.status);

                // if status is != OK we should not have any data
                if (m_status != yarp::experimental::dev::IFrameProviderStatusOK)
                    return;

                for (unsigned seg = 0; seg < m_segmentsCount; ++seg) {
                    xsens::Vector3& position = frame.segmentsData[seg].position;
                    xsens::Quaternion& orientation = frame.segmentsData[seg].orientation;
                    xsens::Vector3& linVelocity = frame.segmentsData[seg].velocity;
                    xsens::Vector3& angVelocity = frame.segmentsData[seg].angularVelocity;
                    xsens::Vector3& linAcceleration = frame.segmentsData[seg].acceleration;
                    xsens::Vector3& angAcceleration = frame.segmentsData[seg].angularAcceleration;

                    yarp::sig::Vector& newPose = m_poses[seg];
                    yarp::sig::Vector& newVelocity = m_velocities[seg];
                    yarp::sig::Vector& newAcceleration = m_accelerations[seg];

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

        XsensMVNRemoteLight::XsensMVNRemoteLight()
            : m_pimpl(new XsensMVNRemoteLightPrivate())
        {}

        XsensMVNRemoteLight::~XsensMVNRemoteLight()
        {
            assert(m_pimpl);

            delete m_pimpl;
            m_pimpl = 0;
        }

        bool XsensMVNRemoteLight::open(yarp::os::Searchable& config)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);

            // as this is the light version read the frame information from a Bottle list
            if (!parseFrameListOption(config.find("segments"), m_pimpl->m_frames)) {
                yError("Error while parsing frames in configuration");
                return false;
            }

            yarp::os::ConstString deviceName =
                config
                    .check("local", yarp::os::Value("/xsens_remote_light"), "Checking device name")
                    .asString();
            if (deviceName.empty() || deviceName.at(0) != '/') {
                yError("Invalid device name '%s'", deviceName.c_str());
                return false;
            }

            if (!m_pimpl->m_inputPort.open(deviceName + "/frames:i")) {
                yError("Could not open streaming input port");
                return false;
            }

            yarp::os::ConstString remote = config.find("remote").asString();
            if (remote.empty() || remote[0] != '/') {
                yError("Invalid remote name %s", remote.c_str());
                close();
                return false;
            }
            m_pimpl->m_remoteStreamingPortName = remote + "/frames:o";

            yarp::os::ConstString carrier =
                config
                    .check("carrier",
                           yarp::os::Value("udp"),
                           "Checking streaming connection carrier. Default udp")
                    .asString();

            yarp::os::Value trueValue;
            trueValue.fromString("true");
            bool autoconnect =
                config.check("autoconnect", trueValue, "Checking autoconnect option").asBool();
            if (autoconnect) {
                if (!yarp::os::Network::connect(m_pimpl->m_remoteStreamingPortName.c_str(),
                                                m_pimpl->m_inputPort.getName().c_str(),
                                                carrier.c_str())) {
                    yError("Error while establishing connection to remote (%s) ports",
                           remote.c_str());
                    close();
                    return false;
                }
            }

            // Obtain information regarding size of data
            size_t segmentCount = m_pimpl->m_frames.size();
            m_pimpl->m_poses.reserve(segmentCount);
            m_pimpl->m_velocities.reserve(segmentCount);
            m_pimpl->m_accelerations.reserve(segmentCount);
            m_pimpl->m_segmentsCount = segmentCount;

            for (size_t i = 0; i < segmentCount; ++i) {
                m_pimpl->m_poses.push_back(yarp::sig::Vector(7, 0.0));
                m_pimpl->m_velocities.push_back(yarp::sig::Vector(6, 0.0));
                m_pimpl->m_accelerations.push_back(yarp::sig::Vector(6, 0.0));
            }

            // register for callbacks
            m_pimpl->m_inputPort.useCallback(*m_pimpl);

            return true;
        }
        bool XsensMVNRemoteLight::close()
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);

            m_pimpl->m_inputPort.disableCallback();

            yarp::os::Network::disconnect(m_pimpl->m_remoteStreamingPortName,
                                          m_pimpl->m_inputPort.getName());
            m_pimpl->m_inputPort.close();

            return true;
        }

        // IPreciselyTimed interface
        yarp::os::Stamp XsensMVNRemoteLight::getLastInputStamp()
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            return m_pimpl->m_timestamp;
            ;
        }

        // IFrameProvider interface
        std::vector<yarp::experimental::dev::FrameReference> XsensMVNRemoteLight::frames()
        {
            assert(m_pimpl);
            return m_pimpl->m_frames;
        }

        // Get Data
        yarp::experimental::dev::IFrameProviderStatus
        XsensMVNRemoteLight::getFramePoses(std::vector<yarp::sig::Vector>& segmentPoses)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentPoses = m_pimpl->m_poses;
            return m_pimpl->m_status;
        }

        yarp::experimental::dev::IFrameProviderStatus
        XsensMVNRemoteLight::getFrameVelocities(std::vector<yarp::sig::Vector>& segmentVelocities)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentVelocities = m_pimpl->m_velocities;
            return m_pimpl->m_status;
        }

        yarp::experimental::dev::IFrameProviderStatus XsensMVNRemoteLight::getFrameAccelerations(
            std::vector<yarp::sig::Vector>& segmentAccelerations)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentAccelerations = m_pimpl->m_accelerations;
            return m_pimpl->m_status;
        }

        yarp::experimental::dev::IFrameProviderStatus XsensMVNRemoteLight::getFrameInformation(
            std::vector<yarp::sig::Vector>& segmentPoses,
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

        static bool
        parseFrameListOption(const yarp::os::Value& option,
                             std::vector<yarp::experimental::dev::FrameReference>& parsedFrames)
        {
            if (option.isNull() || !option.isList() || !option.asList())
                return false;
            yarp::os::Bottle* frames = option.asList();
            parsedFrames.reserve(static_cast<size_t>(frames->size()));

            for (int i = 0; i < frames->size(); ++i) {
                yarp::experimental::dev::FrameReference frameReference;
                yarp::os::Value& reference = frames->get(i);
                if (reference.isNull() || !reference.isList() || !reference.asList()
                    || reference.asList()->size() != 2) {
                    yWarning("Malformed input segment at index %d", i);
                    continue;
                }
                frameReference.frameReference = reference.asList()->get(0).asString();
                frameReference.frameName = reference.asList()->get(1).asString();
                parsedFrames.push_back(frameReference);
            }
            return true;
        }
    } // namespace dev
} // namespace yarp
