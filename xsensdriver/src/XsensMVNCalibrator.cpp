/*
* Copyright(C) 2016 iCub Facility
* Authors: Francesco Romano
* CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/


#include "XsensMVNCalibrator.h"
#include <xsens/xmecontrol.h>
#include <xsens/xmecalibrationresult.h>

#include <yarp/os/Value.h>
#include <yarp/os/LogStream.h>

#include <algorithm>
#include <thread>
#include <chrono>
#include <cassert>
#include <map>

namespace xsens {

    XsensMVNCalibrator::XsensMVNCalibrator(XmeControl& connector)
        : m_calibratorState(NONE)
        , m_suitsConnector(connector)
        , m_calibrationTime(3)
    {
        m_calibrationPose.reserve(m_suitsConnector.segmentCount());
        for (unsigned i = 0; i < m_suitsConnector.segmentCount(); ++i) {
            m_calibrationPose.push_back(yarp::sig::Vector(7, 0.0));
        }
        m_suitsConnector.addCallbackHandler(this);
    }

    XsensMVNCalibrator::~XsensMVNCalibrator()
    {
        m_suitsConnector.removeCallbackHandler(this);
    }

    void XsensMVNCalibrator::setCalibrationTime(double seconds) 
    {
        assert(seconds > 0);
        m_calibrationTime = seconds;
    }

    double XsensMVNCalibrator::calibrationTime() const
    {
        return m_calibrationTime;
    }

    void XsensMVNCalibrator::addDelegate(xsens::XsensMVNCalibratorDelegate& delegate)
    { 
        std::vector<xsens::XsensMVNCalibratorDelegate*>::iterator found = std::find(m_delegates.begin(), m_delegates.end(), &delegate);
        if (found == m_delegates.end())
            m_delegates.push_back(&delegate);
    }

    void XsensMVNCalibrator::removeDelegate(xsens::XsensMVNCalibratorDelegate& delegate)
    {
        std::vector<xsens::XsensMVNCalibratorDelegate*>::iterator found = std::find(m_delegates.begin(), m_delegates.end(), &delegate);
        if (found != m_delegates.end())
            m_delegates.erase(found);
    }

    bool XsensMVNCalibrator::setBodyDimensions(const std::map<std::string, double> &bodyDimensions)
    {
        if (m_calibratorState != NONE &&
            m_calibratorState != FINISHED) { //we can reset the calibration
            return false;
        }

        //We do not cache the dimensions as we obtain always the most updated values from Xsens
        for (std::map<std::string, double>::const_iterator it = bodyDimensions.begin();
            it != bodyDimensions.end(); ++it) {
            m_suitsConnector.setBodyDimension(it->first, it->second);
        }

        //wait for completion
        std::unique_lock<std::mutex> lock(m_syncMutex);
        m_calibrationAborted = m_calibrationCompleted = m_calibrationProcessed = false;
        m_calibrationSynchronizer.wait(lock, [&]() { return m_calibrationCompleted; });

        yInfo("Body dimensions saved");

        return m_calibrationCompleted;

    }

    std::map<std::string, double> XsensMVNCalibrator::bodyDimensions() const
    {
        std::map<std::string, double> dimensions;
        XsStringArray dimensionList = m_suitsConnector.bodyDimensionLabelList();
        for (auto dim : dimensionList) {
            double value = m_suitsConnector.bodyDimensionValueEstimate(dim);
            if (value != -1) {
                dimensions.insert(std::map<std::string, double>::value_type(dim.c_str(), value));
            }
        }

        return dimensions;
    }

    bool XsensMVNCalibrator::calibrateWithType(std::string calibrationType, int maxTrials)
    {
        std::lock_guard<std::mutex> duplicateCalibrationGuard(m_calibrationMutex);
        //Check if body dimensions are set
        std::map<std::string, double> dimensions = bodyDimensions();
        if (dimensions.empty()) {
            yError("Calibrator: Set body dimensions first");
        }

        m_suitsConnector.initializeCalibration(calibrationType);
        XsIntArray phases = m_suitsConnector.calibrationPhaseList();

        unsigned trial = 0;
        while(true) {

            //From Xsens support:
            //Between the startCalibration and the stopCalibration, 
            //the system will record the calibration data (60Hz). 
            //Youll then need to get the poses by calling XmePose calibPose = myXme->calibrationPose(frameStart++);

            m_suitsConnector.startCalibration();
            for (int phase = 0; phase < phases.size() - 1; phase++) {
                int frameStart = phases[phase];
                int frameEnd = phases[phase + 1];
                XsString phaseText = m_suitsConnector.calibrationPhaseText(phase);

                yInfo("%s", phaseText.c_str());

                while (frameStart < frameEnd) {
                    XmePose calibPose = m_suitsConnector.calibrationPose(frameStart++);
                    //
                    // Display the pose and the phaseText
                    //
                    if (m_delegates.size() > 0) {
                        XmeSegmentStateArray poseSegments = calibPose.m_segmentStates;
                        for (unsigned index = 0; index < m_calibrationPose.size(); ++index) {
                            yarp::sig::Vector &currentSegment = m_calibrationPose[index];
                            const XmeSegmentState &currentPoseSegment = poseSegments[index];

                            for (unsigned i = 0; i < 3; ++i) {
                                currentSegment(i) = currentPoseSegment.m_position[i];
                                currentSegment(3 + i) = currentPoseSegment.m_orientation[i];
                            }
                            currentSegment(6) = currentPoseSegment.m_orientation[3];
                        }
                        
                        for (auto delegate : m_delegates) {
                            delegate->calibratorHasReceivedNewCalibrationPose(this, m_calibrationPose);
                        }
                    }

                    // sleep, yield, allow other threads to run...
                    //Sleep for 1/60s to perform calibration at 60Hz
                    long timeout = static_cast<long>(std::ceil(1000.0 / 60.0));
                    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long>(std::ceil(1000.0 / 60.0))));
                    
                }
            }
            m_suitsConnector.stopCalibration();

            {
                std::unique_lock<std::mutex> lock(m_syncMutex);
                m_calibrationAborted = m_calibrationCompleted = m_calibrationProcessed = false;
                // wait for the onCalibrationProcessed callback here
                m_calibrationSynchronizer.wait(lock, [&]()
                { return !m_calibrationProcessed && !m_calibrationAborted; });
            }

            if (m_calibrationAborted) {
                yInfo("Calibration aborted");
                return true;
            }

            XmeCalibrationResult result = m_suitsConnector.calibrationResult(calibrationType);

            std::string quality = "Unknown";
            if (result.m_quality == XCalQ_Good) quality = "Good";
            else if (result.m_quality == XCalQ_Acceptable) quality = "Acceptable";
            else if (result.m_quality == XCalQ_Poor) quality = "Poor";
            else if (result.m_quality == XCalQ_Failed) quality = "Failed";

            yInfo("Calibration finished. Quality of data is \"%s\"", quality.c_str());

            if (result.m_quality == XCalQ_Good) {
                m_suitsConnector.finalizeCalibration();
                // wait for the onCalibrationComplete callback here
                {
                    std::unique_lock<std::mutex> lock(m_syncMutex);
                    m_calibrationAborted = m_calibrationCompleted = m_calibrationProcessed = false;
                    // wait for the onCalibrationProcessed callback here
                    m_calibrationSynchronizer.wait(lock, [&]()
                    { return !m_calibrationCompleted && !m_calibrationAborted; });
                }

                if (m_calibrationAborted) {
                    yInfo("Calibration aborted");
                    return true;
                }
                m_calibratorState = FINISHED;
                break;

            }
            
            m_calibratorState = FAILED;
            if (maxTrials > 0 && ++trial > maxTrials)
                break;

            // repeat calibration
            yInfo("Repeating calibration");
        }

        //What if we do not accept calibration? Should we clear it?
        if (m_calibratorState != FINISHED) {
            m_suitsConnector.abortCalibration();
            std::unique_lock<std::mutex> lock(m_syncMutex);
            m_calibrationAborted = m_calibrationCompleted = m_calibrationProcessed = false;
            // wait for the onCalibrationProcessed callback here
            m_calibrationSynchronizer.wait(lock, [&]()
            { return !m_calibrationAborted; });
        }

        return m_calibratorState == FINISHED;
    }

    void XsensMVNCalibrator::abortCalibration()
    {
        m_suitsConnector.abortCalibration();
    }

    void XsensMVNCalibrator::onCalibrationAborted(XmeControl *dev)
    {
        std::unique_lock<std::mutex> lock(m_syncMutex);
        m_calibrationAborted = true;
        m_calibrationSynchronizer.notify_one();
    }

    void XsensMVNCalibrator::onCalibrationComplete(XmeControl *dev)
    {
        std::unique_lock<std::mutex> lock(m_syncMutex);
        m_calibrationCompleted = true;
        m_calibrationSynchronizer.notify_one();
    }

    void XsensMVNCalibrator::onCalibrationProcessed(XmeControl *dev)
    {
        std::unique_lock<std::mutex> lock(m_syncMutex);
        m_calibrationProcessed = true;
        m_calibrationSynchronizer.notify_one();
    }

    void XsensMVNCalibrator::onCalibrationStopped(XmeControl *dev)
    {

    }

}
