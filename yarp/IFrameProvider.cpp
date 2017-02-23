/*
* Copyright(C) 2016 iCub Facility
* Authors: Francesco Romano
* CopyPolicy : Released under the terms of the LGPLv2.1 or later, see LGPL.TXT
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
            
            unsigned IFrameProvider::getFrameCount() const { return frames().size(); }
            FrameReference IFrameProvider::frameAtIndex(unsigned frameIndex) const
            {
                if (frameIndex >= getFrameCount()) {
                    FrameReference dummy = { "", "" };
                    return dummy;
                }
                return frames()[frameIndex];
            }
            
            int IFrameProvider::frameIndexForFrame(const FrameReference& frame) const
            {
                std::vector<FrameReference> frames = this->frames();
                std::vector<FrameReference>::iterator found = std::find(frames.begin(), frames.end(), frame);
                if (found == frames.end()) return -1;
                return std::distance(frames.begin(), found);
            }

            bool IFrameProvider::getFrameInformation(std::vector<yarp::sig::Vector>& segmentPoses,
                std::vector<yarp::sig::Vector>& segmentVelocities,
                std::vector<yarp::sig::Vector>& segmentAccelerations)
            {
                return getFramePoses(segmentPoses)
                    && getFrameVelocities(segmentVelocities)
                    && getFrameAccelerations(segmentAccelerations);
            }

        }
    }
}
