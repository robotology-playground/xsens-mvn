#include "XsensMVNWrapper.h"

#include <thrift/XsensFrame.h>
#include "thrift/XsensDriverService.h"

#include <yarp/os/Searchable.h>
#include <yarp/os/LockGuard.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/Port.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/Value.h>
#include <yarp/dev/IHumanSkeleton.h>
#include <yarp/sig/Vector.h>

#include <vector>
#include <cassert>

namespace yarp {
    namespace dev {

        class XsensMVNWrapper::XsensMVNWrapperPrivate : 
            public yarp::os::RateThread,
            public xsens::XsensDriverService
        {
        public:

            XsensMVNWrapperPrivate() : RateThread(100) {}

            virtual ~XsensMVNWrapperPrivate() {}
            yarp::os::Mutex m_mutex;
            yarp::dev::IHumanSkeleton* m_human;
            yarp::dev::IPreciselyTimed* m_timedDriver;

            yarp::os::BufferedPort<xsens::XsensFrame> m_outputPort;
            yarp::os::Port m_commandPort;

            std::vector<yarp::sig::Vector> m_poses;
            std::vector<yarp::sig::Vector> m_velocities;
            std::vector<yarp::sig::Vector> m_accelerations;

            virtual void run()
            {
                if (!m_human) return;
                //read from device
                yarp::os::LockGuard guard(m_mutex);
                yInfo() << "[RUN]" << __FILE__ << ":" << __LINE__;

                yarp::os::Stamp timestamp = m_timedDriver->getLastInputStamp();
                if (!m_human->getSegmentInformation(m_poses, m_velocities, m_accelerations)) {
                    yError("Reading from Xsens device returned error");
                }

                xsens::XsensFrame &frame = m_outputPort.prepare();
                
                m_outputPort.write();
            }

            virtual bool calibrate()
            {
                yInfo() << __FILE__ << ":" << __LINE__;
                return calibrateWithType("");
            }

            virtual bool calibrateWithType(const std::string& calibrationType)
            {
                yInfo() << __FILE__ << ":" << __LINE__;
                if (!m_human) return false;
                yInfo() << __FILE__ << ":" << __LINE__;
                resume();
                bool result = m_human->calibrate(calibrationType);
                suspend();
                return result;
            }

            virtual void startAcquisition()
            {
                if (!m_human) return;
                bool result = m_human->startAcquisition();
                if (result) resume();
            }

            virtual void stopAcquisition()
            {
                if (!m_human) return;
                bool result = m_human->stopAcquisition();
                if (result) suspend();
            }

            virtual std::vector<std::string> segments()
            {
                if (!m_human) return std::vector<std::string>();
                return m_human->segmentNames();
            }

            virtual std::map<std::string, double> bodyDimensions()
            {
                if (!m_human) return std::map<std::string, double>();
                return m_human->bodyDimensions();
            }

            virtual bool setBodyDimension(const std::string& dimensionKey, const double dimensionValue)
            {
                if (!m_human) return false;
                return m_human->setBodyDimension(dimensionKey, dimensionValue);
            }

            virtual bool setBodyDimensions(const std::map<std::string, double> & dimensions)
            {
                if (!m_human) return false;
                return m_human->setBodyDimensions(dimensions);
            }
        };

        XsensMVNWrapper::XsensMVNWrapper()
            : m_pimpl(new XsensMVNWrapperPrivate()) {}

        XsensMVNWrapper::~XsensMVNWrapper()
        {
            assert(m_pimpl);
            detachAll();
            m_pimpl->stop();
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

            if (!m_pimpl->m_outputPort.open(wrapperName + "/frames:o")) {
                yError("Could not open streaming output port");
                return false;
            }

            if (!m_pimpl->m_commandPort.open(wrapperName + "/cmd:i")) {
                yError("Could not open command input port");
                return false;
            }
            this->m_pimpl->yarp().attachAsServer(m_pimpl->m_commandPort);
            bool result = m_pimpl->start();
            //start in suspend mode
            //We resume only for acquisition
            m_pimpl->suspend();
            return result;
        }
        bool XsensMVNWrapper::close()
        {
            assert(m_pimpl);
            yInfo() << __FILE__ << ":" << __LINE__;
            m_pimpl->m_outputPort.close();
            m_pimpl->m_commandPort.close();
            yInfo() << __FILE__ << ":" << __LINE__;
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

            yInfo() << __FILE__ << ":" << __LINE__;
            if (!poly || m_pimpl->m_human) return false;

            yInfo() << __FILE__ << ":" << __LINE__;
            if (!poly->view(m_pimpl->m_human) || !m_pimpl->m_human) return false;

            yInfo() << __FILE__ << ":" << __LINE__;
            if (!poly->view(m_pimpl->m_timedDriver) || !m_pimpl->m_timedDriver) return false;

            yInfo() << __FILE__ << ":" << __LINE__;
            //resize the vectors
            unsigned segmentCount = m_pimpl->m_human->getSegmentCount();
            m_pimpl->m_poses.resize(segmentCount);
            m_pimpl->m_velocities.resize(segmentCount);
            m_pimpl->m_accelerations.resize(segmentCount);
            for (unsigned i = 0; i < segmentCount; ++i) {
                m_pimpl->m_poses[i].resize(7, 0.0);
                m_pimpl->m_velocities[i].resize(6, 0.0);
                m_pimpl->m_accelerations[i].resize(6, 0.0);
            }


            return true;
        }

        bool XsensMVNWrapper::detach()
        {
            assert(m_pimpl);
            yarp::os::LockGuard guard(m_pimpl->m_mutex);
            m_pimpl->m_human = 0;
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
