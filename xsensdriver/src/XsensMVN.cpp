/*
* Copyright (C) 2016 iCub Facility
* Authors: Francesco Romano
* CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/

#include "XsensMVN.h"

#include "XsensMVNPrivate.h"

#include <yarp/os/LockGuard.h>
#include <yarp/os/LogStream.h>
#include <xme.h>
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
    yInfo() << __FILE__ << ":" << __LINE__ << val;
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

unsigned yarp::dev::XsensMVN::getSegmentCount() const
{
    assert(m_pimpl);
    return m_pimpl->segmentNames().size();
}

std::string yarp::dev::XsensMVN::segmentNameAtIndex(unsigned segmentIndex) const
{
    assert(m_pimpl);
    std::vector<std::string> segments = m_pimpl->segmentNames();
    if (segmentIndex >= segments.size()) return "";
    return segments[segmentIndex];
}

std::vector<std::string> yarp::dev::XsensMVN::segmentNames() const
{
    assert(m_pimpl);
    return m_pimpl->segmentNames();
}

int yarp::dev::XsensMVN::segmentIndexForName(const std::string& name) const
{
    assert(m_pimpl);
    std::vector<std::string> segments = m_pimpl->segmentNames();
    int index = 0;
    for (auto segment : segments) {
        if (segment == name) {
            return index;
        }
        index++;
    }
    return -1;
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

std::map<std::string, double> yarp::dev::XsensMVN::bodyDimensions() const
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

bool yarp::dev::XsensMVN::getSegmentPoses(std::vector<yarp::sig::Vector>& segmentPoses)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyVelocities, dummyAccelerations;
    return getSegmentInformation(segmentPoses, dummyVelocities, dummyAccelerations);
}

bool yarp::dev::XsensMVN::getSegmentVelocities(std::vector<yarp::sig::Vector>& segmentVelocities)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyPoses, dummyAccelerations;
    return getSegmentInformation(dummyPoses, segmentVelocities, dummyAccelerations);
}

bool yarp::dev::XsensMVN::getSegmentAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations)
{
    //create dummy vectors
    std::vector<yarp::sig::Vector> dummyPoses, dummyVelocities;
    return getSegmentInformation(dummyPoses, dummyVelocities, segmentAccelerations);
}

bool yarp::dev::XsensMVN::getSegmentInformation(std::vector<yarp::sig::Vector>& segmentPoses,
    std::vector<yarp::sig::Vector>& segmentVelocities,
    std::vector<yarp::sig::Vector>& segmentAccelerations)
{
    assert(m_pimpl);
    yarp::os::Stamp dummy;
    return m_pimpl->getLastSegmentInformation(dummy, segmentPoses, segmentVelocities, segmentAccelerations);
}
