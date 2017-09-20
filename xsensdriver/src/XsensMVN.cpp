/*
* Copyright (C) 2016-2017 iCub Facility
* Authors: Francesco Romano, Luca Tagliapietra
* CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/

#include "XsensMVN.h"

#include "XsensMVNPrivate.h"

#include <yarp/os/LockGuard.h>
#include <yarp/os/LogStream.h>
#include <cassert>

yarp::dev::XsensMVN::XsensMVN() 
    : m_pimpl(new XsensMVNPrivate())
{  
}

yarp::dev::XsensMVN::~XsensMVN()
{
    assert(m_pimpl);
    delete m_pimpl;
    m_pimpl = 0;
}

bool yarp::dev::XsensMVN::open(yarp::os::Searchable &config)
{
    assert(m_pimpl);
    yarp::os::LockGuard guard(m_mutex);
    bool val=  m_pimpl->init(config);
    return val;
}

bool yarp::dev::XsensMVN::close()
{
    assert(m_pimpl);
    yarp::os::LockGuard guard(m_mutex);
    m_pimpl->fini();
    return true;
}

yarp::dev::XsensMVN::XsensMVN(const yarp::dev::XsensMVN& /*other*/)
{
    // Copy is disabled 
    assert(false);
}

yarp::dev::XsensMVN& yarp::dev::XsensMVN::operator=(const yarp::dev::XsensMVN& /*other*/)
{
    // Copy is disabled 
    assert(false);
    return *this;
}

yarp::os::Stamp yarp::dev::XsensMVN::getLastInputStamp()
{
    assert(m_pimpl);
    yarp::os::Stamp timestamp;
    m_pimpl->getLastSegmentReadTimestamp(timestamp);
    return timestamp;
}

std::vector<yarp::experimental::dev::FrameReference> yarp::dev::XsensMVN::frames()
{
    assert(m_pimpl);
    return m_pimpl->segmentNames();
}

std::vector<yarp::experimental::dev::IMUFrameReference> yarp::dev::XsensMVN::IMUFrames()
{
    assert(m_pimpl);
    return m_pimpl->sensorIDs();
}

bool yarp::dev::XsensMVN::setBodyDimensions(const std::map<std::string, double>& dimensions)
{
    assert(m_pimpl);
    return m_pimpl->setBodyDimensions(dimensions);
}

bool yarp::dev::XsensMVN::setBodyDimension(const std::string& bodyPart, const double dimension)
{
    assert(m_pimpl);
    std::map<std::string, double> dimensions;
    dimensions.insert(std::map<std::string, double>::value_type(bodyPart, dimension));
    return m_pimpl->setBodyDimensions(dimensions);
}

std::map<std::string, double> yarp::dev::XsensMVN::bodyDimensions()
{
    assert(m_pimpl);
    return m_pimpl->bodyDimensions();
}
bool yarp::dev::XsensMVN::calibrate(const std::string &calibrationType)
{
    assert(m_pimpl);
    return m_pimpl->calibrateWithType(calibrationType);
}

bool yarp::dev::XsensMVN::abortCalibration()
{
    assert(m_pimpl);
    return m_pimpl->abortCalibration();
}

bool yarp::dev::XsensMVN::startAcquisition()
{
    assert(m_pimpl);
    return m_pimpl->startAcquisition();
}

bool yarp::dev::XsensMVN::stopAcquisition()
{
    assert(m_pimpl);
    return m_pimpl->stopAcquisition();
}

yarp::experimental::dev::IFrameProviderStatus yarp::dev::XsensMVN::getFramePoses(std::vector<yarp::sig::Vector>& segmentPoses)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyVelocities, dummyAccelerations;
    return getFrameInformation(segmentPoses, dummyVelocities, dummyAccelerations);
}

yarp::experimental::dev::IFrameProviderStatus yarp::dev::XsensMVN::getFrameVelocities(std::vector<yarp::sig::Vector>& segmentVelocities)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyPoses, dummyAccelerations;
    return getFrameInformation(dummyPoses, segmentVelocities, dummyAccelerations);
}

yarp::experimental::dev::IFrameProviderStatus yarp::dev::XsensMVN::getFrameAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyPoses, dummyVelocities;
    return getFrameInformation(dummyPoses, dummyVelocities, segmentAccelerations);
}

yarp::experimental::dev::IFrameProviderStatus yarp::dev::XsensMVN::getFrameInformation(std::vector<yarp::sig::Vector>& segmentPoses,
                                                                                       std::vector<yarp::sig::Vector>& segmentVelocities,
                                                                                       std::vector<yarp::sig::Vector>& segmentAccelerations)
{
    assert(m_pimpl);
    yarp::os::Stamp dummy;
    return m_pimpl->getLastSegmentInformation(dummy, segmentPoses, segmentVelocities, segmentAccelerations);
}

yarp::experimental::dev::IIMUFrameProviderStatus yarp::dev::XsensMVN::getIMUFrameOrientation(std::vector<yarp::sig::Vector>& imuOrientations)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyVelocities, dummyAccelerations, dummyMagneticFields;
    return getIMUFrameInformation(imuOrientations, dummyVelocities, dummyAccelerations, dummyMagneticFields);
}

yarp::experimental::dev::IIMUFrameProviderStatus yarp::dev::XsensMVN::getIMUFrameAngularVelocities(std::vector<yarp::sig::Vector>& imuAngularVelocities)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyOrientations, dummyAccelerations, dummyMagneticFields;
    return getIMUFrameInformation(dummyOrientations, imuAngularVelocities, dummyAccelerations, dummyMagneticFields);
}

yarp::experimental::dev::IIMUFrameProviderStatus yarp::dev::XsensMVN::getIMUFrameLinearAccelerations(std::vector<yarp::sig::Vector>& imuLinearAccelerations)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyOrientations, dummyVelocities, dummyMagneticFields;
    return getIMUFrameInformation(dummyOrientations, dummyVelocities, imuLinearAccelerations, dummyMagneticFields);
}

yarp::experimental::dev::IIMUFrameProviderStatus yarp::dev::XsensMVN::getIMUFrameMagneticFields(std::vector<yarp::sig::Vector>& imuMagneticFields)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyOrientations, dummyVelocities, dummyAccelerations;
    return getIMUFrameInformation(dummyOrientations, dummyVelocities, dummyAccelerations, imuMagneticFields);
}

yarp::experimental::dev::IIMUFrameProviderStatus yarp::dev::XsensMVN::getIMUFrameInformation(std::vector<yarp::sig::Vector>& imuOrientations,
                                                                                             std::vector<yarp::sig::Vector>& imuAngularVelocities,
                                                                                             std::vector<yarp::sig::Vector>& imuLinearAccelerations,
                                                                                             std::vector<yarp::sig::Vector>& imuMagneticFields)
{
    assert(m_pimpl);
    yarp::os::Stamp dummy;
    return m_pimpl->getLastSensorInformation(dummy, imuOrientations, imuAngularVelocities, imuLinearAccelerations, imuMagneticFields);
}