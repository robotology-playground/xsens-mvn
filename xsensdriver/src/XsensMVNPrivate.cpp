/*
* Copyright (C) 2016 iCub Facility
* Authors: Francesco Romano
* CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/

#include "XsensMVNPrivate.h"

#include "XsensMVNCalibrator.h"

#include <xme.h>
#include <yarp/os/Searchable.h>
#include <yarp/os/LogStream.h>
#include <string>
#include <chrono>

void yarp::dev::XsensMVN::XsensMVNPrivate::resizeVectorToOuterAndInnerSize(std::vector<yarp::sig::Vector>& vector, unsigned outerSize, unsigned innerSize)
{
    unsigned oldSize = vector.size();
    vector.resize(outerSize);
    for (unsigned i = oldSize; i < outerSize; ++i) {
        vector[i].resize(innerSize, 0.0);
    }
}

yarp::dev::XsensMVN::XsensMVNPrivate::XsensMVNPrivate()
    : m_connection(0)
    , m_calibrator(0)
    , m_acquiring(false)
    , m_stopProcessor(false)
    , m_hardwareFound(false)
{}

yarp::dev::XsensMVN::XsensMVNPrivate::~XsensMVNPrivate() {}

bool yarp::dev::XsensMVN::XsensMVNPrivate::init(yarp::os::Searchable &config)
{
    if (m_connection) return false;

    TCHAR pwd[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, pwd);
    std::wcout << "[INFO]PWD: " << pwd << "\n";

    m_connection = XmeControl::construct();
    if (!m_connection) {
        yError("Could not open Xsens DLL. Check to use the correct DLL.");
        return false;
    }

    m_connection->addCallbackHandler(this);
    yInfo("--- Available configurations ---");
    XsStringArray configurations = m_connection->configurationList();
    for (XsStringArray::const_iterator it = configurations.begin();
        it != configurations.end(); ++it) {
        yInfo(" - %s",it->c_str());
    }
    yInfo("--------------------------------");

    bool configFound = false;
    yarp::os::ConstString configuration = config.check("suit-config", yarp::os::Value(""), "checking MVN configuration").asString();
    for (const XsString &conf : configurations) {
        if (conf == configuration.c_str()) {
            configFound = true;
            break;
        }
    }

    if (!configFound) {
        yError("Configuration %s not found", configuration.c_str());
        fini();
        return false;
    }
    yInfo("Using MVN configuration %s", configuration.c_str());

    //I have some problems with custom durations. Use the default milliseconds
    int scanTimeout = static_cast<int>(config.check("scanTimeout", yarp::os::Value(10.0), "scan timeout before failing (in seconds). -1 for disabling timeout").asDouble() * 1000.0);
    m_connection->setConfiguration(configuration.c_str());

    //This call is asynchronous.
    //Manually synchronize it
    //std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now(); //why is this wrong?
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    std::unique_lock<std::mutex> lock(m_mutex);
    m_hardwareFound = false;
    m_connection->setScanMode(true);
    if (scanTimeout > 0) {
        m_initializationVariable.wait_until(lock, now + std::chrono::milliseconds(scanTimeout), [&]() { return m_hardwareFound; });
    }
    else {
        m_initializationVariable.wait(lock, [&]() { return m_hardwareFound; });
    }

    //I should check if everything is ok..
    XmeStatus status = m_connection->status();
    if (!status.isConnected()) {
        yError("Could not connect to suit.");
        m_connection->setScanMode(false);
        fini();
        return false;
    }

    //create processor thread to free computation from MVN callbacks
    m_processor = std::thread(&yarp::dev::XsensMVN::XsensMVNPrivate::processNewFrame, this);

    yInfo("Xsens suit ready to be calibrated");
    yInfo("--- Available body dimensions ---");
    XsStringArray bodyDimensions = m_connection->bodyDimensionLabelList();
    for (XsStringArray::const_iterator it = bodyDimensions.begin();
        it != bodyDimensions.end(); ++it) {
        yInfo("%s", it->c_str());
    }
    yInfo("--------------------------------");

    yInfo("--- Available calibration methods ---");
    XsStringArray calibrationMethods = m_connection->calibrationLabelList();
    for (XsStringArray::const_iterator it = calibrationMethods.begin();
        it != calibrationMethods.end(); ++it) {
        yInfo("%s", it->c_str());
    }
    yInfo("--------------------------------");
    yarp::os::ConstString defaultConfiguration = config.check("default-calibration-type", yarp::os::Value("Npose"), "checking default calibration type").asString();
    //This should not be necessary
    m_defaultCalibrationType.assign(defaultConfiguration.c_str(), defaultConfiguration.length());
    yInfo("Default calibration type: %s", m_defaultCalibrationType.c_str());

    //create a calibrator
    m_calibrator = new xsens::XsensMVNCalibrator(*m_connection);
    if (!m_calibrator) {
        yError("Could not create a calibrator object");
        fini();
        return false;
    }
    m_calibrator->addDelegate(*this);

    //Initialize size of vectors (model is set before calibration)
    resizeVectorToOuterAndInnerSize(m_lastSegmentPosesRead, m_connection->segmentCount(), 7);
    resizeVectorToOuterAndInnerSize(m_lastSegmentVelocitiesRead, m_connection->segmentCount(), 6);
    resizeVectorToOuterAndInnerSize(m_lastSegmentAccelerationRead, m_connection->segmentCount(), 6);

    yInfo("--- Body model Segments ---");
    std::vector<std::string> segments = segmentNames();
    //Segment ID is 1-based, segment Index = segmentID - 1 (i.e. 0-based)
    for (unsigned index = 0; index < segments.size(); ++index) {
        yInfo("[%d]%s", index + 1, segments[index].c_str());
    }
    yInfo("--------------------------------");

    return m_hardwareFound;
}

bool yarp::dev::XsensMVN::XsensMVNPrivate::fini()
{
    if (m_calibrator) {
        m_calibrator->removeDelegate(*this);
        delete m_calibrator;
        m_calibrator = 0;
    }

    if (m_connection) {
        stopAcquisition();
        m_connection->clearCallbackHandlers();
        m_connection->setScanMode(false);
        m_connection->disconnectHardware();

        m_connection->destruct();
        m_connection = 0;
    }
    return true;
}

std::vector<std::string> yarp::dev::XsensMVN::XsensMVNPrivate::segmentNames() const
{
    if (!m_connection) return std::vector<std::string>();

    std::vector<std::string> segments;
    segments.reserve(m_connection->segmentCount());
    //Segment ID is 1-based, segment Index = segmentID - 1 (i.e. 0-based)
    for (unsigned index = 0; index < m_connection->segmentCount(); ++index) {
        segments.push_back(m_connection->segmentName(index + 1).c_str());
    }
    return segments;
}

bool yarp::dev::XsensMVN::XsensMVNPrivate::startAcquisition()
{
    if (!m_connection) return false;
    //TODO: do some checks also on the status of the device
    m_acquiring = true;
    m_connection->setRealTimePoseMode(true);
    return true;

}

bool yarp::dev::XsensMVN::XsensMVNPrivate::stopAcquisition()
{
    if (!m_connection) return false;
    m_acquiring = false;
    m_connection->setRealTimePoseMode(false);
    return true;
}


bool yarp::dev::XsensMVN::XsensMVNPrivate::setBodyDimensions(const std::map<std::string, double>& dimensions)
{
    if (!m_connection || !m_calibrator) return false;
    return m_calibrator->setBodyDimensions(dimensions);
}

std::map<std::string, double> yarp::dev::XsensMVN::XsensMVNPrivate::bodyDimensions() const 
{
    return m_calibrator->bodyDimensions();
}

bool yarp::dev::XsensMVN::XsensMVNPrivate::calibrateWithType(const std::string &calibrationType)
{
    if (!m_connection || !m_calibrator) return false;
    if (m_acquiring) {
        yError("Cannot calibrate while acquiring data. Please stop acquisition first");
        return false;
    }

    yInfo() << __FILE__ << ":" << __LINE__;
    std::string calibration = calibrationType;
    if (calibration.empty()) {
        calibration = m_defaultCalibrationType;
    }
    if (calibration.empty()) {
        yInfo() << __FILE__ << ":" << __LINE__;
        return false;
    }
    
    yInfo() << __FILE__ << ":" << __LINE__;
    return m_calibrator->calibrateWithType(calibration);
}



bool yarp::dev::XsensMVN::XsensMVNPrivate::abortCalibration()
{
    if (!m_connection || !m_calibrator) return false;
    m_calibrator->abortCalibration();
    return true;
}

void yarp::dev::XsensMVN::XsensMVNPrivate::processNewFrame()
{
    std::unique_lock<std::mutex> lock(m_processorGuard);
    //while no data
    m_processorVariable.wait(lock, [&](){ return !m_frameData.empty(); });
    //get the copied data
    FrameData lastFrame = m_frameData.front();
    m_frameData.pop();
    //release lock
    lock.unlock();

    //process incoming pose to obtain information

    {
        //Unique lock for all data?
        std::lock_guard<std::mutex> readLock(m_dataMutex);

        if (m_lastSegmentPosesRead.size() != lastFrame.pose.m_segmentStates.size()) return; //error
        //HP: absoluteTime = ms from epoch (as Unix Epoch)
        long unixTime = lastFrame.pose.m_absoluteTime;
        double time = unixTime / 1000.0;
        m_lastIMUsTimestamp = yarp::os::Stamp(lastFrame.pose.m_frameNumber, time);

        for (unsigned index = 0; index < lastFrame.pose.m_segmentStates.size(); ++index) {
            const XmeSegmentState &segmentData = lastFrame.pose.m_segmentStates[index];
            yarp::sig::Vector &segmentPosition = m_lastSegmentPosesRead[index];
            yarp::sig::Vector &segmentVelocity = m_lastSegmentVelocitiesRead[index];
            yarp::sig::Vector &segmentAcceleration = m_lastSegmentAccelerationRead[index];


            for (unsigned i = 0; i < 3; ++i) {
                //linear part
                segmentPosition(i) = segmentData.m_position[i];
                segmentVelocity(i) = segmentData.m_velocity[i];
                segmentAcceleration(i) = segmentData.m_acceleration[i];
                //angular part for velocity and acceleration
                //TODO: check quaternion format of Xsens
                segmentPosition(3 + i) = segmentData.m_orientation[i];
                segmentVelocity(3 + i) = segmentData.m_angularVelocity[i];
                segmentAcceleration(i) = segmentData.m_angularAcceleration[i];
            }
            //last element of orientation
            segmentPosition(6) = segmentData.m_orientation[3];
        }
    }

    //newFrame.sensorsData.reserve(lastFrame.sensorsData.size());
    //for (auto segmentData : lastFrame.sensorsData) {
    //    xsens::XsensSensorData sensor;
    //    
    //    segment.position.c1 = segmentData.m_position[0];
    //    segment.position.c2 = segmentData.m_position[1];
    //    segment.position.c3 = segmentData.m_position[2];

    //    segment.velocity.c1 = segmentData.m_velocity[0];
    //    segment.velocity.c2 = segmentData.m_velocity[1];
    //    segment.velocity.c3 = segmentData.m_velocity[2];

    //    segment.acceleration.c1 = segmentData.m_acceleration[0];
    //    segment.acceleration.c2 = segmentData.m_acceleration[1];
    //    segment.acceleration.c3 = segmentData.m_acceleration[2];

    //    //angular information
    //    segment.orientation.c1 = segmentData.m_orientation[0];
    //    segment.orientation.c2 = segmentData.m_orientation[1];
    //    segment.orientation.c3 = segmentData.m_orientation[2];
    //    segment.orientation.c4 = segmentData.m_orientation[3];

    //    segment.angularVelocity.c1 = segmentData.m_angularVelocity[0];
    //    segment.angularVelocity.c2 = segmentData.m_angularVelocity[1];
    //    segment.angularVelocity.c3 = segmentData.m_angularVelocity[2];

    //    segment.angularAcceleration.c1 = segmentData.m_angularAcceleration[0];
    //    segment.angularAcceleration.c2 = segmentData.m_angularAcceleration[1];
    //    segment.angularAcceleration.c3 = segmentData.m_angularAcceleration[2];

    //    newFrame.segments.push_back(segment);

    //}


}

bool yarp::dev::XsensMVN::XsensMVNPrivate::getLastSegmentReadTimestamp(yarp::os::Stamp& timestamp)
{
    std::lock_guard<std::mutex> readLock(m_dataMutex);
    timestamp = m_lastSegmentTimestamp;
    return true;
}

bool yarp::dev::XsensMVN::XsensMVNPrivate::getLastSegmentInformation(yarp::os::Stamp& timestamp, 
    std::vector<yarp::sig::Vector>& lastPoses,
    std::vector<yarp::sig::Vector>& lastVelocities,
    std::vector<yarp::sig::Vector>& lastAccelerations)
{
    //These also ensure all the sizes are the same
    if (lastPoses.size() < m_lastSegmentPosesRead.size()) {
        //This will cause an allocation
        resizeVectorToOuterAndInnerSize(lastPoses, m_lastSegmentPosesRead.size(), 7);
    }
    if (lastVelocities.size() < m_lastSegmentPosesRead.size()) {
        //This will cause an allocation
        resizeVectorToOuterAndInnerSize(lastVelocities, m_lastSegmentPosesRead.size(), 6);
    }
    if (lastAccelerations.size() < m_lastSegmentPosesRead.size()) {
        //This will cause an allocation
        resizeVectorToOuterAndInnerSize(lastAccelerations, m_lastSegmentPosesRead.size(), 6);
    }

    std::lock_guard<std::mutex> readLock(m_dataMutex);
    timestamp = m_lastSegmentTimestamp;
    for (unsigned i = 0; i < m_lastSegmentPosesRead.size(); ++i) {
        lastPoses[i] = m_lastSegmentPosesRead[i];
        lastVelocities[i] = m_lastSegmentVelocitiesRead[i];
        lastAccelerations[i] = m_lastSegmentAccelerationRead[i];
    }
    return true;
}

// Callback functions
void yarp::dev::XsensMVN::XsensMVNPrivate::onHardwareReady(XmeControl *dev)
{
    yInfo("Ready");
    std::unique_lock<std::mutex> lock(m_mutex);
    m_hardwareFound = true;
    m_initializationVariable.notify_one();
}

void yarp::dev::XsensMVN::XsensMVNPrivate::onHardwareError(XmeControl* dev)
{
    XmeStatus status = dev->status();
    yWarning("Hw error received.\nSuit connected? %s\nSuit scanning? %s",
        (status.isConnected() ? "yes" : "no"),
        (status.isScanning() ? "yes" : "no"));
}

void yarp::dev::XsensMVN::XsensMVNPrivate::onPoseReady(XmeControl* dev)
{
    FrameData newFrame;
    newFrame.pose = dev->pose(XME_LAST_AVAILABLE_FRAME);
    newFrame.sensorsData = dev->sampleData(XME_LAST_AVAILABLE_FRAME);
    // or suitSample (int frameNumber)??
 

    std::unique_lock<std::mutex> lock(m_processorGuard);
    //copy data into processor queue
    m_frameData.push(newFrame);
    m_processorVariable.notify_one();

}

void yarp::dev::XsensMVN::XsensMVNPrivate::calibratorHasReceivedNewCalibrationPose(const xsens::XsensMVNCalibrator* const sender, 
    std::vector<yarp::sig::Vector> newPose)
{
    //only manage my own calibrator
    if (sender != m_calibrator) return;
    std::lock_guard<std::mutex> readLock(m_dataMutex);

    for (unsigned index = 0; index < newPose.size(); ++index) { 
        m_lastSegmentPosesRead[index] = newPose[index];
    }
}
