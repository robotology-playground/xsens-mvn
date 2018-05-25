/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
 */

#ifndef YARP_DEV_IFRAMEPROVIDER_H
#define YARP_DEV_IFRAMEPROVIDER_H

#include <yarp/dev/api.h>

#include <string>
#include <vector>

namespace yarp {
    namespace experimental {
        namespace dev {
            class IFrameProvider;
            struct FrameReference;

            enum IFrameProviderStatus
            {
                IFrameProviderStatusOK = 0,
                IFrameProviderStatusError = 1,
                IFrameProviderStatusNoData = 1 << 1,
                IFrameProviderStatusTimeout = 1 << 2,
            };

        } // namespace dev
    } // namespace experimental

    namespace sig {
        class Vector;
    }
} // namespace yarp

/**
 * \since 2.3.69
 */
struct yarp::experimental::dev::FrameReference
{
    std::string frameReference;
    std::string frameName;
    bool operator==(const FrameReference&) const;
};

/**
 * Interface representing a provider of frames (and possibly their velocity and acceleration)
 * \since 2.3.69
 */
class yarp::experimental::dev::IFrameProvider
{
public:
    virtual ~IFrameProvider();

    virtual unsigned getFrameCount();
    virtual FrameReference frameAtIndex(unsigned frameIndex);
    virtual std::vector<FrameReference> frames() = 0;
    virtual int frameIndexForFrame(const FrameReference& frame);

    /**
     * Retrieve the last read poses of all frames
     *
     * Each frame is represented with a 7D vector.
     * The first 3 elements represent the position (x,y,z) of the origin w.r.t. its reference frame
     * The last 4 elements are the quaterion representing the orientation of the frame w.r.t. its
     * reference frame. \note the quaternion is serialized as (w, x, y, z) where
     * - w is the real part
     * - (x,y,z) are the components of the vector representing the imaginary part
     *
     * \see getFrameInformation
     * \see frames
     */
    virtual yarp::experimental::dev::IFrameProviderStatus
    getFramePoses(std::vector<yarp::sig::Vector>& segmentPoses) = 0;
    virtual yarp::experimental::dev::IFrameProviderStatus
    getFrameVelocities(std::vector<yarp::sig::Vector>& segmentVelocities) = 0;
    virtual yarp::experimental::dev::IFrameProviderStatus
    getFrameAccelerations(std::vector<yarp::sig::Vector>& segmentAccelerations) = 0;
    virtual yarp::experimental::dev::IFrameProviderStatus
    getFrameInformation(std::vector<yarp::sig::Vector>& segmentPoses,
                        std::vector<yarp::sig::Vector>& segmentVelocities,
                        std::vector<yarp::sig::Vector>& segmentAccelerations);
};

#endif /* End of YARP_DEV_IFRAMEPROVIDER_H */
