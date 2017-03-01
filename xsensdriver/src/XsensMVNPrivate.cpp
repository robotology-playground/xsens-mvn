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
#include <yarp/os/Time.h>
#include <string>
#include <chrono>

void yarp::dev::XsensMVN::XsensMVNPrivate::resizeVectorToOuterAndInnerSize(std::vector<yarp::sig::Vector>& vector, unsigned outerSize, unsigned innerSize)
{
    size_t oldSize = vector.size();
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
    , m_driverStatus(yarp::experimental::dev::IFrameProviderStatusNoData)
{}

yarp::dev::XsensMVN::XsensMVNPrivate::~XsensMVNPrivate() {}

bool yarp::dev::XsensMVN::XsensMVNPrivate::init(yarp::os::Searchable &config)
{
    std::lock_guard<std::recursive_mutex> globalGuard(m_objectMutex);
    if (m_connection) return false;
    
    //First thing: retrieve paths. As from Xsens support:
    //The location of the other files (xmedef, *.mvnc) can be configured at runtime by calling xmeSetPaths. 
    //This expects in this order: the path to the xmedef.xsb file, also used for the .mvnc files,
    // the path where props.xsb can be found (probably not useful for you),
    // the path where you want the xme.log to be put and a Boolean value if you want to remove the old xme.log 
    // (if you specified a new path). Specifying empty strings will use the default path for those options.

    //getting TEMP folder
    TCHAR tempFolder[MAX_PATH + 1];
    if (GetTempPath(MAX_PATH, tempFolder) == 0) {
        yWarning("Failed to retrieve temporary folder");
    }

    std::wstring wideCharString(tempFolder);

    std::string tempFolderConverted(wideCharString.begin(), wideCharString.end());
    yInfo() << "Temporary directory is " << tempFolderConverted;

    yarp::os::ConstString licenseDir = config.check("license-dir", yarp::os::Value(""), "Checking directory containing license files").asString();
    xmeSetPaths(licenseDir.c_str(), "", tempFolder, true);
    yInfo("Reading license files from %s", licenseDir.c_str());

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

    yarp::os::ConstString referenceFrame = config.check("reference-frame", yarp::os::Value("xsens_world"), "checking absolute reference frame name").asString();

    //I have some problems with custom durations. Use the default milliseconds
    int scanTimeout = static_cast<int>(config.check("scanTimeout", yarp::os::Value(10.0), "scan timeout before failing (in seconds). -1 for disabling timeout").asDouble() * 1000.0);
    m_connection->setConfiguration(configuration.c_str());

    //This call is asynchronous.
    //Manually synchronize it
    //std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now(); //why is this wrong?
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

    std::unique_lock<std::mutex> lock(m_initializationMutex);
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
    m_stopProcessor = false;
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
    std::vector<yarp::experimental::dev::FrameReference> segments = segmentNames();
    //Segment ID is 1-based, segment Index = segmentID - 1 (i.e. 0-based)
    for (unsigned index = 0; index < segments.size(); ++index) {
        yInfo("[%d]%s_H_%s", index + 1, segments[index].frameReference.c_str(), segments[index].frameName.c_str());
    }
    yInfo("--------------------------------");

    return m_hardwareFound;
}

bool yarp::dev::XsensMVN::XsensMVNPrivate::fini()
{
    std::lock_guard<std::recursive_mutex> globalGuard(m_objectMutex);
    if (m_calibrator) {
        m_calibrator->removeDelegate(*this);
        delete m_calibrator;
        m_calibrator = 0;
    }

    if (m_connection) {
        stopAcquisition();
        m_connection->setScanMode(false);
        m_connection->disconnectHardware();

        std::unique_lock<std::mutex> lock(m_initializationMutex);
        m_initializationVariable.wait(lock, [&]() { return !m_connection->status().isConnected(); });
        
        //Now remove callbacks
        m_connection->clearCallbackHandlers();

        m_connection->destruct();
        m_connection = 0;
    }
    //Xsens now should not provide anymore callbacks
    //close the thread
    {
        std::unique_lock<std::mutex> lock(m_processorGuard);
        m_stopProcessor = true;
        //notify the thread
        m_processorVariable.notify_one();
        //I still have the lock. Let's join the thread and wait fo its termination
    }
    m_processor.join();
    return true;
}

std::vector<yarp::experimental::dev::FrameReference> yarp::dev::XsensMVN::XsensMVNPrivate::segmentNames() const
{
    std::lock_guard<std::recursive_mutex> globalGuard(m_objectMutex);
    if (!m_connection || m_connection->segmentCount() < 0) return std::vector<yarp::experimental::dev::FrameReference>();

    std::vector<yarp::experimental::dev::FrameReference> segments;
    segments.reserve(m_connection->segmentCount());
    //Segment ID is 1-based, segment Index = segmentID - 1 (i.e. 0-based)
    for (unsigned index = 0; index < static_cast<unsigned>(m_connection->segmentCount()); ++index) {
        yarp::experimental::dev::FrameReference frameInfo = { m_referenceFrame, m_connection->segmentName(index + 1).c_str() };
        segments.push_back(frameInfo);
    }
    return segments;
}

bool yarp::dev::XsensMVN::XsensMVNPrivate::startAcquisition()
{
    std::lock_guard<std::recursive_mutex> globalGuard(m_objectMutex);
    if (!m_connection || !m_connection->status().isConnected()) return false;
    //TODO: do some checks also on the status of the device
    m_acquiring = true;
    m_connection->setRealTimePoseMode(true);
    yInfo("Starting acquiring data");
    m_driverStatus = yarp::experimental::dev::IFrameProviderStatusOK;
    return true;

}

bool yarp::dev::XsensMVN::XsensMVNPrivate::stopAcquisition()
{
    std::lock_guard<std::recursive_mutex> globalGuard(m_objectMutex);
    if (!m_connection) return false;
    m_acquiring = false;
    m_connection->setRealTimePoseMode(false);
    yInfo("Stopping acquiring data");
    m_driverStatus = yarp::experimental::dev::IFrameProviderStatusNoData;
    return true;
}


bool yarp::dev::XsensMVN::XsensMVNPrivate::setBodyDimensions(const std::map<std::string, double>& dimensions)
{
    std::lock_guard<std::recursive_mutex> globalGuard(m_objectMutex);
    if (!m_connection || !m_calibrator) return false;
    return m_calibrator->setBodyDimensions(dimensions);
}

std::map<std::string, double> yarp::dev::XsensMVN::XsensMVNPrivate::bodyDimensions() const 
{
    std::lock_guard<std::recursive_mutex> globalGuard(m_objectMutex);
    return m_calibrator->bodyDimensions();
}

bool yarp::dev::XsensMVN::XsensMVNPrivate::calibrateWithType(const std::string &calibrationType)
{
    std::string calibration = calibrationType;
    {
        std::lock_guard<std::recursive_mutex> globalGuard(m_objectMutex);
        if (!m_connection || !m_calibrator) return false;
        if (m_acquiring) {
            yError("Cannot calibrate while acquiring data. Please stop acquisition first");
            return false;
        }

        if (calibration.empty()) {
            calibration = m_defaultCalibrationType;
        }
        if (calibration.empty()) {
            yInfo() << __FILE__ << ":" << __LINE__;
            return false;
        }

    }
    m_driverStatus = yarp::experimental::dev::IFrameProviderStatusOK;
    return m_calibrator->calibrateWithType(calibration);
}



bool yarp::dev::XsensMVN::XsensMVNPrivate::abortCalibration()
{
    // I don't know if this makes sense, but how can I abort the calibration
    // if I have to take the same lock of the calibrate function?
    {
        std::lock_guard<std::recursive_mutex> globalGuard(m_objectMutex);
        if (!m_connection || !m_calibrator) return false;
    }
    m_driverStatus = yarp::experimental::dev::IFrameProviderStatusNoData;
    m_calibrator->abortCalibration();
    return true;
}

void yarp::dev::XsensMVN::XsensMVNPrivate::processNewFrame()
{
    while (true) {
        std::unique_lock<std::mutex> lock(m_processorGuard);
        //while no data
        m_processorVariable.wait(lock, [&](){ return m_stopProcessor || !m_frameData.empty(); });
        //last chance to check if we have to return
        //before starting processing stuff
        if (m_stopProcessor) {
            //we should return
            break;
        }
        //get the copied data
        FrameData lastFrame = m_frameData.front();
        m_frameData.pop();
        //release lock
        lock.unlock();

        //process incoming pose to obtain information

        {
            //Unique lock for all data?
            std::lock_guard<std::mutex> readLock(m_dataMutex);

            if (m_lastSegmentPosesRead.size() != lastFrame.pose.m_segmentStates.size()) {
                m_driverStatus = yarp::experimental::dev::IFrameProviderStatusError;
                continue; //error
            }

            //HP: absoluteTime = ms from epoch (as Unix Epoch)
            int64_t unixTime = lastFrame.pose.m_absoluteTime;
            double time = unixTime / 1000.0;
            m_lastSegmentTimestamp = yarp::os::Stamp(lastFrame.pose.m_frameNumber, time);
            
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
                    segmentVelocity(3 + i) = segmentData.m_angularVelocity[i];
                    segmentAcceleration(i) = segmentData.m_angularAcceleration[i];
                }
                // Do the quaternion explicitly to avoid issues in format
                segmentPosition(3) = segmentData.m_orientation.w();
                segmentPosition(4) = segmentData.m_orientation.x();
                segmentPosition(5) = segmentData.m_orientation.y();
                segmentPosition(6) = segmentData.m_orientation.z();
            }
            m_driverStatus = yarp::experimental::dev::IFrameProviderStatusOK;
        }
    }
}

yarp::experimental::dev::IFrameProviderStatus yarp::dev::XsensMVN::XsensMVNPrivate::getLastSegmentReadTimestamp(yarp::os::Stamp& timestamp)
{
    std::lock_guard<std::mutex> readLock(m_dataMutex);
    timestamp = m_lastSegmentTimestamp;
    return m_driverStatus;
}

yarp::experimental::dev::IFrameProviderStatus yarp::dev::XsensMVN::XsensMVNPrivate::getLastSegmentInformation(yarp::os::Stamp& timestamp,
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

    {
        std::lock_guard<std::mutex> readLock(m_dataMutex);

        if (m_acquiring) {
            //we should receive data
            double now = yarp::os::Time::now();
            if ((now - m_lastSegmentTimestamp.getTime()) > 1.0) {
                m_driverStatus = yarp::experimental::dev::IFrameProviderStatusTimeout;
            }
            /*else {
                m_driverStatus = yarp::experimental::dev::IFrameProviderStatusOK;
            }*/
        }
        //get anyway data out
        timestamp = m_lastSegmentTimestamp;
        for (unsigned i = 0; i < m_lastSegmentPosesRead.size(); ++i) {
            lastPoses[i] = m_lastSegmentPosesRead[i];
            lastVelocities[i] = m_lastSegmentVelocitiesRead[i];
            lastAccelerations[i] = m_lastSegmentAccelerationRead[i];
        }
    }

   return m_driverStatus;
}

// Callback functions
void yarp::dev::XsensMVN::XsensMVNPrivate::onHardwareReady(XmeControl *dev)
{
    yInfo("Ready");
    std::unique_lock<std::mutex> lock(m_initializationMutex);
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

void  yarp::dev::XsensMVN::XsensMVNPrivate::onHardwareDisconnected(XmeControl*)
{
    yInfo("Suit disconnected");
    std::unique_lock<std::mutex> lock(m_initializationMutex);
    m_hardwareFound = false;
    m_initializationVariable.notify_one();
}

void yarp::dev::XsensMVN::XsensMVNPrivate::calibratorHasReceivedNewCalibrationPose(const xsens::XsensMVNCalibrator* const sender, 
    std::vector<yarp::sig::Vector> newPose)
{
    //only manage my own calibrator
    if (sender != m_calibrator) return;
    std::lock_guard<std::mutex> readLock(m_dataMutex);

    for (unsigned index = 0; index < newPose.size(); ++index) { 
        m_lastSegmentPosesRead[index] = newPose[index];
        m_lastSegmentVelocitiesRead[index].zero();
        m_lastSegmentAccelerationRead[index].zero();
    }
}
