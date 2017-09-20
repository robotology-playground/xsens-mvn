/*
* Copyright(C) 2017 iCub Facility
* Authors: Luca Tagliapietra
* CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
*/

#ifndef YARP_DEV_IIMUFRAMEPROVIDER_H
#define YARP_DEV_IIMUFRAMEPROVIDER_H

#include <yarp/dev/api.h>

#include <string>
#include <vector>

namespace yarp {
    namespace experimental {
        namespace dev {
            class IIMUFrameProvider;
            struct IMUFrameReference;

            enum IIMUFrameProviderStatus {
                IIMUFrameProviderStatusOK = 0,
                IIMUFrameProviderStatusError = 1,
                IIMUFrameProviderStatusNoData = 1 << 1,
                IIMUFrameProviderStatusTimeout = 1 << 2,
                };
        }
    }

    namespace sig {
        class Vector;
    }
}

/**
 * \since 2.3.69
 */
struct yarp::experimental::dev::IMUFrameReference {
    std::string IMUframeReference;
    std::string IMUframeName;
    bool operator==(const IMUFrameReference&) const;
};

/**
 * Interface representing a provider of frames (and possibly their velocity and acceleration)
 * \since 2.3.69
 */
class yarp::experimental::dev::IIMUFrameProvider {
public:

    virtual ~IIMUFrameProvider();

    virtual unsigned getIMUFrameCount();
    virtual IMUFrameReference IMUFrameAtIndex(unsigned IMUFrameIndex);
    virtual int IMUFrameIndexForIMUFrame(const IMUFrameReference& IMUFrame);

    virtual std::vector<IMUFrameReference> IMUFrames() = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus getIMUFrameOrientation(std::vector<yarp::sig::Vector>& imuOrientations) = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus getIMUFrameAngularVelocities(std::vector<yarp::sig::Vector>& imuAngularVelocities) = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus getIMUFrameLinearAccelerations(std::vector<yarp::sig::Vector>& imuLinearAccelerations) = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus getIMUFrameMagneticFields(std::vector<yarp::sig::Vector>& imuMagneticFields) = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus getIMUFrameInformation(std::vector<yarp::sig::Vector>& imuOrientations,
                                                                                    std::vector<yarp::sig::Vector>& imuAngularVelocities,
                                                                                    std::vector<yarp::sig::Vector>& imuLinearAccelerations,
                                                                                    std::vector<yarp::sig::Vector>& imuMagneticFields);
};

#endif /* End of YARP_DEV_IIMUFRAMEPROVIDER_H */
