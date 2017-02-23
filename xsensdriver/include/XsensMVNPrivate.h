/*
* Copyright (C) 2016 iCub Facility
* Authors: Francesco Romano
* CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/

#ifndef XSENSMVNPRIVATE_h
#define XSENSMVNPRIVATE_h

#include "XsensMVN.h"
#include "XsensMVNCalibrator.h"

#include <xsens/xmelicense.h>
#include <xsens/xmepose.h>
#include <xsens/xmecallback.h>
#include <xsens/xmesensorsamplearray.h>

#include <yarp/os/Stamp.h>
#include <yarp/sig/Vector.h>

#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>
#include <vector>

struct XmeControl;
class XsensCallbackHandler;
struct XmeSensorSampleArray;

namespace yarp {
    namespace os {
        class Searchable;
    }
}

namespace xsens {
    class XsensMVNCalibrator;
    class XsensDriver;
}
 
class yarp::dev::XsensMVN::XsensMVNPrivate : public XmeCallback,
    public xsens::XsensMVNCalibratorDelegate
{
    XmeLicense m_license;
    XmeControl *m_connection;

    std::string m_referenceFrame;

    xsens::XsensMVNCalibrator *m_calibrator;
    std::string m_defaultCalibrationType;

    std::mutex m_mutex;
    std::condition_variable m_initializationVariable;

    //process data
    struct FrameData {
        XmePose pose;
        XmeSensorSampleArray sensorsData;
    };

    std::mutex m_dataMutex;
    std::vector<yarp::sig::Vector> m_lastSegmentPosesRead;
    std::vector<yarp::sig::Vector> m_lastSegmentVelocitiesRead;
    std::vector<yarp::sig::Vector> m_lastSegmentAccelerationRead;
    yarp::os::Stamp m_lastSegmentTimestamp;
    bool m_acquiring;

    std::vector<yarp::sig::Vector> m_lastIMUsRead;
    yarp::os::Stamp m_lastIMUsTimestamp;

    // Hardware callback objects
    std::thread m_processor;
    bool m_stopProcessor;
    std::mutex m_processorGuard;
    std::condition_variable m_processorVariable;
    std::queue<FrameData> m_frameData;

    void processNewFrame();

    //hardware scan
    bool m_hardwareFound;


    XsensMVNPrivate(const XsensMVNPrivate&) = delete;
    XsensMVNPrivate& operator=(const XsensMVNPrivate&) = delete;

    void resizeVectorToOuterAndInnerSize(std::vector<yarp::sig::Vector>& vector, unsigned outerSize, unsigned innerSize);

public:
    XsensMVNPrivate();
    virtual ~XsensMVNPrivate();

    bool init(yarp::os::Searchable&);
    bool fini();

    //Information on the available hardware/model/etc
    std::vector<yarp::experimental::dev::FrameReference> segmentNames() const;

    bool startAcquisition();
    bool stopAcquisition();

    //calibration methods
    bool setBodyDimensions(const std::map<std::string, double>& dimensions);
    std::map<std::string, double> bodyDimensions() const;
    bool calibrateWithType(const std::string &calibrationType);
    bool abortCalibration();

    //Get data
    bool getLastSegmentReadTimestamp(yarp::os::Stamp& timestamp);
    bool getLastSegmentInformation(yarp::os::Stamp& timestamp,
        std::vector<yarp::sig::Vector>& lastPoses,
        std::vector<yarp::sig::Vector>& lastVelocities,
        std::vector<yarp::sig::Vector>& lastAccelerations);


    // callbacks
    virtual void onHardwareReady(XmeControl* dev);
    virtual void onHardwareError(XmeControl* dev);
    virtual void onPoseReady(XmeControl* dev);

    //Calibrator callback
    virtual void calibratorHasReceivedNewCalibrationPose(const xsens::XsensMVNCalibrator* const sender, std::vector<yarp::sig::Vector> newPose);

};



#endif //XSENSMVNPRIVATE_h
