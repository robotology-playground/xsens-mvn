/*
* Copyright(C) 2016 iCub Facility
* Authors: Francesco Romano
* CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/

#ifndef YARP_XSENSMVNREMOTELIGHT_H
#define YARP_XSENSMVNREMOTELIGHT_H

#include <yarp/dev/DeviceDriver.h>
#include <yarp/dev/PreciselyTimed.h>
#include <yarp/dev/IFrameProvider.h>

namespace yarp {
    namespace dev {
        class XsensMVNRemoteLight;
    }
}

class yarp::dev::XsensMVNRemoteLight
: public yarp::dev::DeviceDriver
, public yarp::dev::IPreciselyTimed
, public yarp::experimental::dev::IFrameProvider
{
    class XsensMVNRemoteLightPrivate;
    XsensMVNRemoteLightPrivate* m_pimpl;

public:
    XsensMVNRemoteLight();
    virtual ~XsensMVNRemoteLight();

    // DeviceDriver interface 
    bool open(yarp::os::Searchable &config);
    bool close();

    // IPreciselyTimed interface
    virtual yarp::os::Stamp getLastInputStamp();

    // IFrameProvider interface
    virtual std::vector<yarp::experimental::dev::FrameReference> frames();

    virtual yarp::experimental::dev::IFrameProviderStatus getFramePoses(std::vector<yarp::sig::Vector>& segmentPoses);
    virtual yarp::experimental::dev::IFrameProviderStatus getFrameVelocities(std::vector<yarp::sig::Vector>& segmentVelocities);
    virtual yarp::experimental::dev::IFrameProviderStatus getFrameAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations);
    virtual yarp::experimental::dev::IFrameProviderStatus getFrameInformation(std::vector<yarp::sig::Vector>& segmentPoses,
                                                                              std::vector<yarp::sig::Vector>& segmentVelocities,
                                                                              std::vector<yarp::sig::Vector>& segmentAccelerations);

};

#endif /* end of include guard: YARP_XSENSMVNREMOTELIGHT_H */
