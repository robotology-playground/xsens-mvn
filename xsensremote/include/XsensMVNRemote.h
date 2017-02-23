/*
* Copyright(C) 2016 iCub Facility
* Authors: Francesco Romano
* CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/


#ifndef YARP_XSENSMVNWRAPPER_H
#define YARP_XSENSMVNWRAPPER_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/PreciselyTimed.h>
#include <yarp/dev/IFrameProvider.h>
#include <yarp/dev/IXsensMVNInterface.h>

namespace yarp {
    namespace dev {
        class XsensMVNRemote;
    }
}

class yarp::dev::XsensMVNRemote
: public yarp::dev::DeviceDriver
, public yarp::dev::IPreciselyTimed
, public yarp::experimental::dev::IFrameProvider
, public yarp::experimental::dev::IXsensMVNInterface

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

    // IFrameProvider interface
    virtual std::vector<yarp::experimental::dev::FrameReference> frames() const;

    virtual bool getFramePoses(std::vector<yarp::sig::Vector>& segmentPoses);
    virtual bool getFrameVelocities(std::vector<yarp::sig::Vector>& segmentVelocities);
    virtual bool getFrameAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations);
    virtual bool getFrameInformation(std::vector<yarp::sig::Vector>& segmentPoses,
                                     std::vector<yarp::sig::Vector>& segmentVelocities,
                                     std::vector<yarp::sig::Vector>& segmentAccelerations);


    // IXsensMVNInterface interface
    virtual bool setBodyDimensions(const std::map<std::string, double>& dimensions);
    virtual bool setBodyDimension(const std::string& bodyPart, const double dimension);
    virtual std::map<std::string, double> bodyDimensions() const;

    virtual bool calibrate(const std::string &calibrationType = "");
    virtual bool abortCalibration();

    virtual bool startAcquisition();
    virtual bool stopAcquisition();


};


#endif // end of YARP_XSENSMVNWRAPPER_H
