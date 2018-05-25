/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
 */

#ifndef XSENSMVNPRIVATE_h
#define XSENSMVNPRIVATE_h

#include "XsensMVN.h"
#include "XsensMVNCalibrator.h"

#include <xsens/xmecallback.h>
#include <xsens/xmelicense.h>
#include <xsens/xmepose.h>
#include <xsens/xmesensorsamplearray.h>

#include <yarp/os/Stamp.h>
#include <yarp/sig/Vector.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct XmeControl;
class XsensCallbackHandler;
struct XmeSensorSampleArray;

namespace yarp {
    namespace os {
        class Searchable;
    }
} // namespace yarp

namespace xsens {
    class XsensMVNCalibrator;
    class XsensDriver;
} // namespace xsens

class yarp::dev::XsensMVN::XsensMVNPrivate
    : public XmeCallback
    , public xsens::XsensMVNCalibratorDelegate
{
    XmeLicense* m_license;
    XmeControl* m_connection;

    mutable std::recursive_mutex m_objectMutex;

    std::string m_referenceFrame;

    xsens::XsensMVNCalibrator* m_calibrator;
    std::string m_defaultCalibrationType;

    std::mutex m_initializationMutex;
    std::condition_variable m_initializationVariable;

    // process data
    struct FrameData
    {
        XmePose pose;
        XmeSensorSampleArray sensorsData;
    };

    mutable std::mutex m_dataMutex;
    // segment data
    std::vector<yarp::sig::Vector> m_lastSegmentPosesRead;
    std::vector<yarp::sig::Vector> m_lastSegmentVelocitiesRead;
    std::vector<yarp::sig::Vector> m_lastSegmentAccelerationRead;

    std::vector<yarp::sig::Vector> m_lastSensorOrientationsRead;
    std::vector<yarp::sig::Vector> m_lastSensorVelocitiesRead;
    std::vector<yarp::sig::Vector> m_lastSensorAccelerationsRead;
    std::vector<yarp::sig::Vector> m_lastSensorMagneticFieldsRead;

    yarp::os::Stamp m_lastTimestamp;

    bool m_acquiring;

    // std::vector<yarp::sig::Vector> m_lastIMUsRead;
    // yarp::os::Stamp m_lastIMUsTimestamp;

    // Hardware callback objects
    std::thread m_processor;
    bool m_stopProcessor;
    std::mutex m_processorGuard;
    std::condition_variable m_processorVariable;
    FrameData m_lastFrameData;
    bool m_lastFrameDataWritten;

    void processNewFrame();

    // hardware scan
    bool m_hardwareFound;

    yarp::experimental::dev::IFrameProviderStatus m_driverStatus;

    XsensMVNPrivate(const XsensMVNPrivate&) = delete;
    XsensMVNPrivate& operator=(const XsensMVNPrivate&) = delete;

    void resizeVectorToOuterAndInnerSize(std::vector<yarp::sig::Vector>& vector,
                                         unsigned outerSize,
                                         unsigned innerSize);

public:
    XsensMVNPrivate();
    virtual ~XsensMVNPrivate();

    bool init(yarp::os::Searchable&);
    bool fini();

    // Information on the available hardware/model/etc
    std::vector<yarp::experimental::dev::FrameReference> segmentNames() const;
    std::vector<yarp::experimental::dev::IMUFrameReference> sensorIDs() const; // to be implemented

    bool startAcquisition();
    bool stopAcquisition();

    // calibration methods
    bool setBodyDimensions(const std::map<std::string, double>& dimensions);
    std::map<std::string, double> bodyDimensions() const;
    bool calibrateWithType(const std::string& calibrationType);
    bool abortCalibration();

    // Get data
    yarp::experimental::dev::IFrameProviderStatus
    getLastSegmentReadTimestamp(yarp::os::Stamp& timestamp);
    yarp::experimental::dev::IFrameProviderStatus
    getLastSegmentInformation(yarp::os::Stamp& timestamp,
                              std::vector<yarp::sig::Vector>& lastPoses,
                              std::vector<yarp::sig::Vector>& lastVelocities,
                              std::vector<yarp::sig::Vector>& lastAccelerations);

    yarp::experimental::dev::IIMUFrameProviderStatus
    getLastSensorReadTimestamp(yarp::os::Stamp& timestamp);
    yarp::experimental::dev::IIMUFrameProviderStatus
    getLastSensorInformation(yarp::os::Stamp& timestamp,
                             std::vector<yarp::sig::Vector>& lastOrientations,
                             std::vector<yarp::sig::Vector>& lastVelocities,
                             std::vector<yarp::sig::Vector>& lastAccelerations,
                             std::vector<yarp::sig::Vector>& lastMagneticFields);

    // callbacks
    virtual void onHardwareReady(XmeControl* dev);
    virtual void onHardwareError(XmeControl* dev);
    virtual void onHardwareDisconnected(XmeControl*);
    virtual void onPoseReady(XmeControl* dev);

    // Calibrator callback
    virtual void
    calibratorHasReceivedNewCalibrationPose(const xsens::XsensMVNCalibrator* const sender,
                                            std::vector<yarp::sig::Vector> newPose);
};

#endif // XSENSMVNPRIVATE_h
