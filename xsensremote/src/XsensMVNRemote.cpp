/*
* Copyright (C) 2016-2017 iCub Facility
* Authors: Francesco Romano, Luca Tagliapietra
 * CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */

#include "XsensMVNRemote.h"

#include <thrift/XsensSegmentsFrame.h>
#include <thrift/XsensSensorsFrame.h>
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
            public yarp::os::TypedReaderCallback<xsens::XsensSegmentsFrame>, 
            public yarp::os::TypedReaderCallback<xsens::XsensSensorsFrame>
        {
        public:

            yarp::os::BufferedPort<xsens::XsensSegmentsFrame> m_inputSegmentsPort;
            yarp::os::BufferedPort<xsens::XsensSensorsFrame> m_inputSensorsPort;
            yarp::os::Port m_commandPort;

            xsens::XsensDriverService m_xsensService;
            yarp::os::ConstString m_remoteSegmentsStreamingPortName;
            yarp::os::ConstString m_remoteSensorsStreamingPortName;
            yarp::os::ConstString m_remoteCommandPortName;

            //Buffers for read & associated mutex
            yarp::os::Mutex m_mutex;
            yarp::os::Stamp m_segmentsTimestamp;
            yarp::os::Stamp m_sensorsTimestamp;

            // Buffers for segments data and status
            std::vector<yarp::sig::Vector> m_poses;
            std::vector<yarp::sig::Vector> m_velocities;
            std::vector<yarp::sig::Vector> m_accelerations;
            yarp::experimental::dev::IFrameProviderStatus m_segmentsStatus;
            unsigned m_segmentsCount;

            //  Buffers for sensors data and status
            std::vector<yarp::sig::Vector> m_imuOrientations;
            std::vector<yarp::sig::Vector> m_imuAngularVelocities;
            std::vector<yarp::sig::Vector> m_imuLinearAccelerations;
            std::vector<yarp::sig::Vector> m_imuMagneticFields;
            yarp::experimental::dev::IIMUFrameProviderStatus m_sensorsStatus;
            unsigned m_sensorsCount;


            XsensMVNRemotePrivate() : m_segmentsStatus(yarp::experimental::dev::IFrameProviderStatusNoData)
                                    , m_sensorsStatus(yarp::experimental::dev::IIMUFrameProviderStatusNoData)
                                    , m_segmentsCount(0)
                                    , m_sensorsCount(0) {}

            virtual ~XsensMVNRemotePrivate() {}

            // onRead callback for reading XSens Segments Frame data
            virtual void onRead(xsens::XsensSegmentsFrame& frame)
            {
                yarp::os::LockGuard guard(m_mutex);
                //get timestamp
                m_inputSegmentsPort.getEnvelope(m_segmentsTimestamp);

                m_segmentsStatus = static_cast<yarp::experimental::dev::IFrameProviderStatus>(frame.status);

                //if status is != OK we should not have any data
                if (m_segmentsStatus != yarp::experimental::dev::IFrameProviderStatusOK) return;

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

            // onRead callback for reading XSens Sensors Frame data
            virtual void onRead(xsens::XsensSensorsFrame& imuFrame)
            {
                yarp::os::LockGuard guard(m_mutex);
                //get timestamp
                m_inputSensorsPort.getEnvelope(m_sensorsTimestamp);

               m_sensorsStatus = static_cast<yarp::experimental::dev::IIMUFrameProviderStatus>(imuFrame.status);

                //if status is != OK we should not have any data
                if (m_sensorsStatus != yarp::experimental::dev::IIMUFrameProviderStatusOK) return;

                for (unsigned sens = 0; sens < m_sensorsCount; ++sens) {
                    xsens::Quaternion &imuOrientation = imuFrame.sensorsData[sens].orientation;
                    xsens::Vector3 &imuAngularVelocity = imuFrame.sensorsData[sens].angularVelocity;
                    xsens::Vector3 &imuLinearAcceleration = imuFrame.sensorsData[sens].acceleration;
                    xsens::Vector3 &imuMagneticField = imuFrame.sensorsData[sens].magnetometer;

                    yarp::sig::Vector &newImuOrientation = m_imuOrientations[sens];
                    yarp::sig::Vector &newImuAngularVelocity = m_imuAngularVelocities[sens];
                    yarp::sig::Vector &newImuLinearAcceleration = m_imuLinearAccelerations[sens];
                    yarp::sig::Vector &newMagneticField = m_imuMagneticFields[sens];

                    imuOrientation.w = newImuOrientation[0];
                    imuOrientation.imaginary.x = newImuOrientation[1];
                    imuOrientation.imaginary.y = newImuOrientation[2];
                    imuOrientation.imaginary.z = newImuOrientation[3];

                    imuAngularVelocity.x = newImuAngularVelocity[0];
                    imuAngularVelocity.y = newImuAngularVelocity[1];
                    imuAngularVelocity.z = newImuAngularVelocity[2];

                    imuLinearAcceleration.x = newImuLinearAcceleration[0];
                    imuLinearAcceleration.y = newImuLinearAcceleration[1];
                    imuLinearAcceleration.z = newImuLinearAcceleration[2];

                    imuMagneticField.x = newMagneticField[0];
                    imuMagneticField.y = newMagneticField[1];
                    imuMagneticField.z = newMagneticField[2];
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

            if (!m_pimpl->m_inputSegmentsPort.open(deviceName + "/frames:i")) {
                yError("Could not open streaming input port");
                return false;
            }

            if (!m_pimpl->m_inputSensorsPort.open(deviceName + "/imu_frames:i")) {
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
            m_pimpl->m_remoteSegmentsStreamingPortName = remote + "/frames:o";
            m_pimpl->m_remoteSensorsStreamingPortName = remote + "/imu_frames:o";

            yarp::os::ConstString carrier = config.check("carrier", yarp::os::Value("udp"), "Checking streaming connection carrier. Default udp").asString();

            bool result = yarp::os::Network::connect(m_pimpl->m_remoteSegmentsStreamingPortName.c_str(),
                                                     m_pimpl->m_inputSegmentsPort.getName().c_str(),
                                                     carrier.c_str());
            result = result && yarp::os::Network::connect(m_pimpl->m_remoteSensorsStreamingPortName.c_str(),
                                                          m_pimpl->m_inputSensorsPort.getName().c_str(),
                                                          carrier.c_str());

            result = result && yarp::os::Network::connect(m_pimpl->m_commandPort.getName(),
                                                          m_pimpl->m_remoteCommandPortName);

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

            unsigned sensorsCount = this->getIMUFrameCount();
            m_pimpl->m_imuOrientations.reserve(m_pimpl->m_sensorsCount);
            m_pimpl->m_imuAngularVelocities.reserve(m_pimpl->m_sensorsCount);
            m_pimpl->m_imuLinearAccelerations.reserve(m_pimpl->m_sensorsCount);
            m_pimpl->m_imuMagneticFields.reserve(m_pimpl->m_sensorsCount);
            m_pimpl->m_sensorsCount = sensorsCount;

            for (unsigned i = 0; i < sensorsCount; ++i) {
                m_pimpl->m_imuOrientations.push_back(yarp::sig::Vector(4, 0.0));
                m_pimpl->m_imuAngularVelocities.push_back(yarp::sig::Vector(3, 0.0));
                m_pimpl->m_imuLinearAccelerations.push_back(yarp::sig::Vector(3, 0.0));
                m_pimpl->m_imuMagneticFields.push_back(yarp::sig::Vector(3, 0.0));
            }

            //register for callbacks
            m_pimpl->m_inputSegmentsPort.useCallback(*m_pimpl);
            m_pimpl->m_inputSensorsPort.useCallback(*m_pimpl);

            return result;
        }
        bool XsensMVNRemote::close()
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);

            m_pimpl->m_inputSegmentsPort.disableCallback();
            m_pimpl->m_inputSensorsPort.disableCallback();

            bool result = yarp::os::Network::disconnect(m_pimpl->m_remoteSegmentsStreamingPortName,
                                                        m_pimpl->m_inputSegmentsPort.getName());
            result = result && yarp::os::Network::disconnect(m_pimpl->m_remoteSensorsStreamingPortName,
                                                             m_pimpl->m_inputSensorsPort.getName());
            result = result && yarp::os::Network::disconnect(m_pimpl->m_commandPort.getName(),
                                                             m_pimpl->m_remoteCommandPortName);

            m_pimpl->m_inputSegmentsPort.close();
            m_pimpl->m_inputSensorsPort.close();
            m_pimpl->m_commandPort.close();

            return true;
        }

        // IPreciselyTimed interface
        // TODO: check if ok returning just segments timestamp
        yarp::os::Stamp XsensMVNRemote::getLastInputStamp()
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            return m_pimpl->m_segmentsTimestamp;;
        }


        // IFrameProvider interface
        std::vector<yarp::experimental::dev::FrameReference> XsensMVNRemote::frames()
        {
            assert(m_pimpl);
            std::vector<xsens::FrameReferece> frames = m_pimpl->m_xsensService.segments_order();
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
            return m_pimpl->m_segmentsStatus;
        }

        yarp::experimental::dev::IFrameProviderStatus XsensMVNRemote::getFrameVelocities(std::vector<yarp::sig::Vector>& segmentVelocities)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentVelocities = m_pimpl->m_velocities;
            return m_pimpl->m_segmentsStatus;
        }

        yarp::experimental::dev::IFrameProviderStatus XsensMVNRemote::getFrameAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            segmentAccelerations = m_pimpl->m_accelerations;
            return m_pimpl->m_segmentsStatus;
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
            return m_pimpl->m_segmentsStatus;
        }

        // IIMUFrameProvider interface
        std::vector<yarp::experimental::dev::IMUFrameReference> XsensMVNRemote::IMUFrames()
        {
            assert(m_pimpl);
            std::vector<xsens::FrameReferece> imuFrames = m_pimpl->m_xsensService.imu_segments_order();
            std::vector<yarp::experimental::dev::IMUFrameReference> deserializedFrames;
            deserializedFrames.reserve(imuFrames.size());
            std::for_each(imuFrames.begin(), imuFrames.end(), [&](xsens::FrameReferece& frame) {
                yarp::experimental::dev::IMUFrameReference referenceFrame;
                referenceFrame.IMUframeReference = frame.frameReference;
                referenceFrame.IMUframeName = frame.frameName;
                deserializedFrames.push_back(referenceFrame);
            });

            return deserializedFrames;
        }

        // Get Data
        yarp::experimental::dev::IIMUFrameProviderStatus XsensMVNRemote::getIMUFrameOrientations(std::vector<yarp::sig::Vector>& imuOrientations)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            imuOrientations = m_pimpl->m_imuOrientations;
            return m_pimpl->m_sensorsStatus;
        }

        yarp::experimental::dev::IIMUFrameProviderStatus XsensMVNRemote::getIMUFrameAngularVelocities(std::vector<yarp::sig::Vector>& imuAngularVelocities)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            imuAngularVelocities = m_pimpl->m_imuAngularVelocities;
            return m_pimpl->m_sensorsStatus;
        }

        yarp::experimental::dev::IIMUFrameProviderStatus XsensMVNRemote::getIMUFrameLinearAccelerations(std::vector<yarp::sig::Vector>& imuLinearAccelerations)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            imuLinearAccelerations = m_pimpl->m_imuLinearAccelerations;
            return m_pimpl->m_sensorsStatus;
        }

        yarp::experimental::dev::IIMUFrameProviderStatus XsensMVNRemote::getIMUFrameMagneticFields(std::vector<yarp::sig::Vector>& imuMagneticFields)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            imuMagneticFields = m_pimpl->m_imuMagneticFields;
            return m_pimpl->m_sensorsStatus;
        }

        yarp::experimental::dev::IIMUFrameProviderStatus XsensMVNRemote::getIMUFrameInformation(std::vector<yarp::sig::Vector>& imuOrientations,
                                                                                                     std::vector<yarp::sig::Vector>& imuAngularVelocities,
                                                                                                     std::vector<yarp::sig::Vector>& imuLinearAccelerations,
                                                                                                     std::vector<yarp::sig::Vector>& imuMagneticFields)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            imuOrientations = m_pimpl->m_imuOrientations;
            imuAngularVelocities = m_pimpl->m_imuAngularVelocities;
            imuLinearAccelerations = m_pimpl->m_imuLinearAccelerations;
            imuMagneticFields = m_pimpl->m_imuMagneticFields;
            return m_pimpl->m_sensorsStatus;
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
