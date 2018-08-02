/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
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

            enum IIMUFrameProviderStatus
            {
                IIMUFrameProviderStatusOK = 0,
                IIMUFrameProviderStatusError = 1,
                IIMUFrameProviderStatusNoData = 1 << 1,
                IIMUFrameProviderStatusTimeout = 1 << 2,
            };
        } // namespace dev
    } // namespace experimental

    namespace sig {
        template <class T>
        class VectorOf;
        typedef VectorOf<double> Vector;
    } // namespace sig
} // namespace yarp

/**
 * \since 2.3.69
 */
struct yarp::experimental::dev::IMUFrameReference
{
    std::string IMUframeReference;
    std::string IMUframeName;
    bool operator==(const IMUFrameReference&) const;
};

/**
 * Interface representing a provider of frames (and possibly their velocity and acceleration)
 * \since 2.3.69
 */
class yarp::experimental::dev::IIMUFrameProvider
{
public:
    virtual ~IIMUFrameProvider();

    virtual unsigned getIMUFrameCount();
    virtual IMUFrameReference IMUFrameAtIndex(unsigned IMUFrameIndex);
    virtual int IMUFrameIndexForIMUFrame(const IMUFrameReference& IMUFrame);

    virtual std::vector<IMUFrameReference> IMUFrames() = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus
    getIMUFrameOrientations(std::vector<yarp::sig::Vector>& imuOrientations) = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus
    getIMUFrameAngularVelocities(std::vector<yarp::sig::Vector>& imuAngularVelocities) = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus
    getIMUFrameLinearAccelerations(std::vector<yarp::sig::Vector>& imuLinearAccelerations) = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus
    getIMUFrameMagneticFields(std::vector<yarp::sig::Vector>& imuMagneticFields) = 0;
    virtual yarp::experimental::dev::IIMUFrameProviderStatus
    getIMUFrameInformation(std::vector<yarp::sig::Vector>& imuOrientations,
                           std::vector<yarp::sig::Vector>& imuAngularVelocities,
                           std::vector<yarp::sig::Vector>& imuLinearAccelerations,
                           std::vector<yarp::sig::Vector>& imuMagneticFields);
};

#endif /* End of YARP_DEV_IIMUFRAMEPROVIDER_H */
