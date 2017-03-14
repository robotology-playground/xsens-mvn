/*
* Copyright(C) 2016 iCub Facility
* Authors: Francesco Romano
* CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/

#include "XsensMVNWrapper.h"

#include <thrift/XsensSegmentsFrame.h>
#include <thrift/XsensDriverService.h>

#include <yarp/os/Searchable.h>
#include <yarp/os/LockGuard.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Port.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/Value.h>
#include <yarp/dev/IFrameProvider.h>
#include <yarp/dev/IXsensMVNInterface.h>
#include <yarp/sig/Vector.h>

#include <vector>
#include <cassert>
#include <algorithm>

namespace yarp {
    namespace dev {

        class XsensMVNWrapper::XsensMVNWrapperPrivate : 
            public yarp::os::RateThread,
            public xsens::XsensDriverService
        {

            XsensMVNWrapper &m_wrapper;
        public:

            XsensMVNWrapperPrivate(XsensMVNWrapper& wrapper)
            : RateThread(100)
            , m_wrapper(wrapper)
            , m_frameProvider(0)
            , m_xsensInterface(0)
            , m_timedDriver(0)
            , m_frameCount(0) {}

            virtual ~XsensMVNWrapperPrivate() {}
            yarp::os::Mutex m_mutex;
            yarp::experimental::dev::IFrameProvider* m_frameProvider;
            yarp::experimental::dev::IXsensMVNInterface* m_xsensInterface;
            yarp::dev::IPreciselyTimed* m_timedDriver;

            std::vector<yarp::sig::Vector> m_poses;
            std::vector<yarp::sig::Vector> m_velocities;
            std::vector<yarp::sig::Vector> m_accelerations;

            unsigned m_frameCount;

            virtual void run()
            {
                assert(m_wrapper.m_outputPort);
                if (!m_frameProvider) return;
                //read from device
                yarp::os::LockGuard guard(m_mutex);

                yarp::experimental::dev::IFrameProviderStatus frameStatus = m_frameProvider->getFrameInformation(m_poses, m_velocities, m_accelerations);
                
                xsens::XsensSegmentsFrame &frame = m_wrapper.m_outputPort->prepare();
                yarp::os::Stamp timestamp = m_timedDriver->getLastInputStamp();

                m_wrapper.m_outputPort->setEnvelope(timestamp);
                frame.status = static_cast<xsens::XsensStatus>(frameStatus);

                if (frameStatus != yarp::experimental::dev::IFrameProviderStatusOK) {
                    // write without data. Only status
                    // I would like to clear the data so as to transmit only the status.
                    frame.segmentsData.resize(0);
                    m_wrapper.m_outputPort->write();
                    return;
                }

                
                frame.segmentsData.resize(m_frameCount);
                for (unsigned seg = 0; seg < m_frameCount; ++seg) {
                    xsens::Vector3 &position = frame.segmentsData[seg].position;
                    xsens::Quaternion &orientation = frame.segmentsData[seg].orientation;
                    xsens::Vector3 &linVelocity = frame.segmentsData[seg].velocity;
                    xsens::Vector3 &angVelocity = frame.segmentsData[seg].angularVelocity;
                    xsens::Vector3 &linAcceleration = frame.segmentsData[seg].acceleration;
                    xsens::Vector3 &angAcceleration = frame.segmentsData[seg].angularAcceleration;
                   
                    yarp::sig::Vector &newPose = m_poses[seg];
                    yarp::sig::Vector &newVelocity = m_velocities[seg];
                    yarp::sig::Vector &newAcceleration = m_accelerations[seg];

                    position.x = newPose[0];
                    position.y = newPose[1];
                    position.z = newPose[2];
                    orientation.w = newPose[3];
                    orientation.imaginary.x = newPose[4];
                    orientation.imaginary.y = newPose[5];
                    orientation.imaginary.z = newPose[6];

                    linVelocity.x = newVelocity[0];
                    linVelocity.y = newVelocity[1];
                    linVelocity.z = newVelocity[2];
                    angVelocity.x = newVelocity[3];
                    angVelocity.y = newVelocity[4];
                    angVelocity.z = newVelocity[5];

                    linAcceleration.x = newAcceleration[0];
                    linAcceleration.y = newAcceleration[1];
                    linAcceleration.z = newAcceleration[2];
                    angAcceleration.x = newAcceleration[3];
                    angAcceleration.y = newAcceleration[4];
                    angAcceleration.z = newAcceleration[5];

                }
                m_wrapper.m_outputPort->write();
            }

            virtual void calibrateAsync() { calibrateWithType(""); }
           
            virtual void calibrateAsyncWithType(const std::string& calibrationType) { calibrateWithType(calibrationType);  }

            virtual bool calibrate()
            {
                return calibrateWithType("");
            }

            virtual bool calibrateWithType(const std::string& calibrationType)
            {
                if (!m_xsensInterface) return false;
                //resume();
                bool result = m_xsensInterface->calibrate(calibrationType);
                //suspend();
                return result;
            }

            virtual void startAcquisition()
            {
                if (!m_xsensInterface) return;
                bool result = m_xsensInterface->startAcquisition();
                //if (result) resume();
            }

            virtual void stopAcquisition()
            {
                if (!m_xsensInterface) return;
                bool result = m_xsensInterface->stopAcquisition();
                //if (result) suspend();
            }

            virtual std::vector<xsens::FrameReferece> segments()
            {
                if (!m_frameProvider) return std::vector<xsens::FrameReferece>();
                //convert from underlining yarp::dev object into thrift object
                std::vector<yarp::experimental::dev::FrameReference> frames = m_frameProvider->frames();
                std::vector<xsens::FrameReferece> serializableObject;
                serializableObject.reserve(frames.size());
                std::for_each(frames.begin(), frames.end(), [&](const yarp::experimental::dev::FrameReference& frame){
                    xsens::FrameReferece reference;
                    reference.frameName = frame.frameName;
                    reference.frameReference = frame.frameReference;
                    serializableObject.push_back(reference);
                });
                return serializableObject;
            }

            virtual std::map<std::string, double> bodyDimensions()
            {
                if (!m_xsensInterface) return std::map<std::string, double>();
                return m_xsensInterface->bodyDimensions();
            }

            virtual bool setBodyDimension(const std::string& dimensionKey, const double dimensionValue)
            {
                if (!m_xsensInterface) return false;
                return m_xsensInterface->setBodyDimension(dimensionKey, dimensionValue);
            }

            virtual bool setBodyDimensions(const std::map<std::string, double> & dimensions)
            {
                if (!m_xsensInterface) return false;
                return m_xsensInterface->setBodyDimensions(dimensions);
            }
        };

        XsensMVNWrapper::XsensMVNWrapper()
            : m_pimpl(new XsensMVNWrapperPrivate(*this))
            , m_outputPort(0)
            , m_commandPort(0) {}

        XsensMVNWrapper::~XsensMVNWrapper()
        {
            assert(m_pimpl);
            detachAll();
            delete m_pimpl;
            m_pimpl = 0;
        }


        bool XsensMVNWrapper::open(yarp::os::Searchable &config)
        {
            assert(m_pimpl);
            int period = config.check("period", yarp::os::Value(100), "Checking wrapper period [ms]").asInt();
            m_pimpl->setRate(period);
            yarp::os::ConstString wrapperName = config.check("name", yarp::os::Value("/xsens"), "Checking wrapper name").asString();
            if (wrapperName.empty() || wrapperName.at(0) != '/') {
                yError("Invalid wrapper name '%s'", wrapperName.c_str());
                return false;
            }

            m_outputPort = new yarp::os::BufferedPort<xsens::XsensSegmentsFrame>();

            if (!m_outputPort || !m_outputPort->open(wrapperName + "/frames:o")) {
                yError("Could not open streaming output port");
                close();
                return false;
            }

            m_commandPort = new yarp::os::Port();
            if (!m_commandPort || !m_commandPort->open(wrapperName + "/cmd:i")) {
                yError("Could not open command input port");
                close();
                return false;
            }
            this->m_pimpl->yarp().attachAsServer(*m_commandPort);
            bool result = m_pimpl->start();
            //start in suspend mode
            //We resume only for acquisition
            //m_pimpl->suspend();
            return result;
        }

        bool XsensMVNWrapper::close()
        {
            assert(m_pimpl);
            m_pimpl->stop();
            if (m_outputPort) {
                m_outputPort->close();
                delete m_outputPort;
                m_outputPort = 0;
            }
            if (m_commandPort) {
                m_commandPort->close();
                delete m_commandPort;
                m_commandPort = 0;
            }

            detachAll();
            return true;
        }

        // IPreciselyTimed interface
        yarp::os::Stamp XsensMVNWrapper::getLastInputStamp()
        {
            assert(m_pimpl);
            if (!m_pimpl->m_timedDriver) return yarp::os::Stamp();
            return m_pimpl->m_timedDriver->getLastInputStamp();
        }

        // IWrapper interface
        bool XsensMVNWrapper::attach(yarp::dev::PolyDriver *poly)
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);

            if (!poly 
                || m_pimpl->m_frameProvider 
                || m_pimpl->m_xsensInterface) return false;

            if (!poly->view(m_pimpl->m_frameProvider) || !m_pimpl->m_frameProvider) return false;
            if (!poly->view(m_pimpl->m_xsensInterface) || !m_pimpl->m_xsensInterface) return false;

            if (!poly->view(m_pimpl->m_timedDriver) || !m_pimpl->m_timedDriver) return false;

            //resize the vectors
            m_pimpl->m_frameCount = m_pimpl->m_frameProvider->getFrameCount();
            m_pimpl->m_poses.resize(m_pimpl->m_frameCount);
            m_pimpl->m_velocities.resize(m_pimpl->m_frameCount);
            m_pimpl->m_accelerations.resize(m_pimpl->m_frameCount);
            for (unsigned i = 0; i < m_pimpl->m_frameCount; ++i) {
                m_pimpl->m_poses[i].resize(7, 0.0);
                m_pimpl->m_velocities[i].resize(6, 0.0);
                m_pimpl->m_accelerations[i].resize(6, 0.0);
            }
            xsens::XsensSegmentsFrame &frame = m_outputPort->prepare();
            frame.segmentsData.reserve(m_pimpl->m_frameCount);
            m_outputPort->unprepare();

            return true;
        }

        bool XsensMVNWrapper::detach()
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            m_pimpl->m_frameProvider = 0;
            m_pimpl->m_xsensInterface = 0;
            m_pimpl->m_timedDriver = 0;
            return true;
        }

        bool XsensMVNWrapper::attachAll(const yarp::dev::PolyDriverList &driverList)
        {
            assert(m_pimpl);
            if (driverList.size() > 1) {
                yError("Only one device to be attached is supported");
                return false;
            }
            const yarp::dev::PolyDriverDescriptor *firstDriver = driverList[0];
            if (!firstDriver) {
                yError("Failed to get the driver descriptor");
                return false;
            }
            return attach(firstDriver->poly);
        }

        bool XsensMVNWrapper::detachAll()
        {
            assert(m_pimpl);
            return detach();
        }

    }
}
