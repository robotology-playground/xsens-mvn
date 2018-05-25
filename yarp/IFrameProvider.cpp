/*
 * Copyright (C) 2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * GNU Lesser General Public License v2.1 or any later version.
 */

#include "IFrameProvider.h"

#include <algorithm>

namespace yarp {
    namespace experimental {
        namespace dev {

            bool FrameReference::operator==(const FrameReference& frame) const
            {
                return frame.frameName == this->frameName
                       && frame.frameReference == this->frameReference;
            }

            IFrameProvider::~IFrameProvider() {}

            unsigned IFrameProvider::getFrameCount() { return frames().size(); }
            FrameReference IFrameProvider::frameAtIndex(unsigned frameIndex)
            {
                if (frameIndex >= getFrameCount()) {
                    FrameReference dummy = {"", ""};
                    return dummy;
                }
                return frames()[frameIndex];
            }

            int IFrameProvider::frameIndexForFrame(const FrameReference& frame)
            {
                std::vector<FrameReference> frames = this->frames();
                std::vector<FrameReference>::iterator found =
                    std::find(frames.begin(), frames.end(), frame);
                if (found == frames.end())
                    return -1;
                return std::distance(frames.begin(), found);
            }

            IFrameProviderStatus IFrameProvider::getFrameInformation(
                std::vector<yarp::sig::Vector>& segmentPoses,
                std::vector<yarp::sig::Vector>& segmentVelocities,
                std::vector<yarp::sig::Vector>& segmentAccelerations)
            {
                IFrameProviderStatus status = getFramePoses(segmentPoses);
                if (status != IFrameProviderStatusOK) {
                    return status;
                }
                status = getFrameVelocities(segmentVelocities);
                if (status != IFrameProviderStatusOK) {
                    return status;
                }
                return getFrameAccelerations(segmentAccelerations);
            }

        } // namespace dev
    } // namespace experimental
} // namespace yarp
