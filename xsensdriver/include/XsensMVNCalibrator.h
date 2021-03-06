/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
 */

#ifndef XSENSMVNCALIBRATOR_H
#define XSENSMVNCALIBRATOR_H

#include <xsens/xmecallback.h>

#include <yarp/sig/Vector.h>

#include <condition_variable>
#include <map>
#include <mutex>
#include <vector>

namespace xsens {
    class XsensMVNCalibrator;
    class XsensMVNCalibratorDelegate;
} // namespace xsens

namespace yarp {
    namespace os {
        class Value;
    }
    namespace sig {
        class Vector;
    }
} // namespace yarp

class XmeContol;

class xsens::XsensMVNCalibratorDelegate
{
public:
    virtual ~XsensMVNCalibratorDelegate() {}
    virtual void
    calibratorHasReceivedNewCalibrationPose(const xsens::XsensMVNCalibrator* const sender,
                                            std::vector<yarp::sig::Vector> newPose) = 0;
};

class xsens::XsensMVNCalibrator : public XmeCallback
{

    // simple (and hardcoded) state machine for the calibration
    enum
    {
        NONE,
        DIMENSIONS_LOADED,
        INPROGRESS,
        FINISHED,
        FAILED,

    } m_calibratorState;

    XmeControl& m_suitsConnector;

    std::mutex m_calibrationMutex;
    std::mutex m_syncMutex;
    std::condition_variable m_calibrationSynchronizer;

    std::vector<yarp::sig::Vector> m_calibrationPose;

    double m_calibrationTime;

    bool m_calibrationAborted;
    bool m_calibrationProcessed;
    bool m_calibrationCompleted;

    std::vector<xsens::XsensMVNCalibratorDelegate*> m_delegates;

public:
    XsensMVNCalibrator(XmeControl& connector);
    virtual ~XsensMVNCalibrator();

    bool isCalibrationInProgress();

    void addDelegate(xsens::XsensMVNCalibratorDelegate&);
    void removeDelegate(xsens::XsensMVNCalibratorDelegate&);

    void setCalibrationTime(double seconds);
    double calibrationTime() const;

    bool setBodyDimensions(const std::map<std::string, double>& bodyDimensions);
    std::map<std::string, double> bodyDimensions() const;
    bool calibrateWithType(std::string calibrationType, int maxTrials = 5);

    void abortCalibration();

    // Calibration callbacks
    virtual void onCalibrationAborted(XmeControl* dev);
    virtual void onCalibrationComplete(XmeControl* dev);
    virtual void onCalibrationProcessed(XmeControl* dev);
    virtual void onCalibrationStopped(XmeControl* dev);
};

#endif // XSENSMVNCALIBRATOR_H
