/*
 * Copyright (C) 2016 iCub Facility
 * Authors: Francesco Romano
 * CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
 */


#ifndef YARP_XSENSMVN_H
#define YARP_XSENSMVN_H

#include <yarp/os/Mutex.h>

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/PreciselyTimed.h>
#include <yarp/dev/IHumanSkeleton.h>

#include <yarp/sig/Vector.h>

#include <map>

namespace yarp {
namespace dev {

class XsensMVN;

}
}

class yarp::dev::XsensMVN : public yarp::dev::DeviceDriver,
                            public yarp::dev::IPreciselyTimed,
                            public yarp::dev::IHumanSkeleton
{
private:
    // Prevent copy 
    XsensMVN(const XsensMVN & other);
    XsensMVN & operator=(const XsensMVN & other);
    
    // Use a mutex to avoid race conditions
    yarp::os::Mutex m_mutex;
    
    class XsensMVNPrivate;
    XsensMVNPrivate *m_pimpl;
    
public:
    XsensMVN();
    virtual ~XsensMVN();

    // DeviceDriver interface 
    bool open(yarp::os::Searchable &config);
    bool close();
  
    // IPreciselyTimed interface
    virtual yarp::os::Stamp getLastInputStamp();

    // IHumanSkeleton interface
    virtual unsigned getSegmentCount() const;
    virtual std::string segmentNameAtIndex(unsigned segmentIndex) const;
    virtual std::vector<std::string> segmentNames() const;
    virtual int segmentIndexForName(const std::string& name) const;

    virtual bool setBodyDimensions(const std::map<std::string, double>& dimensions);
    virtual bool setBodyDimension(const std::string& bodyPart, const double dimension);
    virtual std::map<std::string, double> bodyDimensions() const;

    virtual bool calibrate(const std::string &calibrationType = "");
    virtual bool abortCalibration();

    virtual bool startAcquisition();
    virtual bool stopAcquisition();

    virtual bool getSegmentPoses(std::vector<yarp::sig::Vector>& segmentPoses);
    virtual bool getSegmentVelocities(std::vector<yarp::sig::Vector>& segmentVelocities);
    virtual bool getSegmentAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations);
    virtual bool getSegmentInformation(std::vector<yarp::sig::Vector>& segmentPoses,
        std::vector<yarp::sig::Vector>& segmentVelocities,
        std::vector<yarp::sig::Vector>& segmentAccelerations);
};



#endif //YARP_XSENSMVN_H

