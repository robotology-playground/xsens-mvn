/*
* Copyright(C) 2016 iCub Facility
* Authors: Francesco Romano
* CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/


#ifndef YARP_XSENSMVNWRAPPER_H
#define YARP_XSENSMVNWRAPPER_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/PreciselyTimed.h>
#include <yarp/dev/IHumanSkeleton.h>

namespace yarp {
    namespace dev {
        class XsensMVNRemote;
    }
}

class yarp::dev::XsensMVNRemote : public yarp::dev::DeviceDriver,
    public yarp::dev::IPreciselyTimed,
    public yarp::dev::IHumanSkeleton

{

    class XsensMVNRemotePrivate;
    XsensMVNRemotePrivate* m_pimpl;

public:
    XsensMVNRemote();
    virtual ~XsensMVNRemote();

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

    //Configuration
    virtual bool setBodyDimensions(const std::map<std::string, double>& dimensions);
    virtual bool setBodyDimension(const std::string& bodyPart, const double dimension);
    virtual std::map<std::string, double> bodyDimensions() const;

    // Calibration methods
    virtual bool calibrate(const std::string &calibrationType = "");
    virtual bool abortCalibration();

    //Acquisition methods
    virtual bool startAcquisition();
    virtual bool stopAcquisition();

    // Get Data
    virtual bool getSegmentPoses(std::vector<yarp::sig::Vector>& segmentPoses);
    virtual bool getSegmentVelocities(std::vector<yarp::sig::Vector>& segmentVelocities);
    virtual bool getSegmentAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations);
    virtual bool getSegmentInformation(std::vector<yarp::sig::Vector>& segmentPoses,
                                       std::vector<yarp::sig::Vector>& segmentVelocities,
                                       std::vector<yarp::sig::Vector>& segmentAccelerations);

};


#endif // end of YARP_XSENSMVNWRAPPER_H
