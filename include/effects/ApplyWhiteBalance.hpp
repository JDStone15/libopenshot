#ifndef APPLYWHITEBALANCE_HPP
#define APPLYWHITEBALANCE_HPP

#include "../EffectBase.h"

#include "ValuedVideoEffect.hpp"

namespace openshot {
class ApplyWhiteBalance : public ValuedVideoEffect {
  public:
    ApplyWhiteBalance() : ValuedVideoEffect(Keyframe{}, "WhiteBalance") {}
    ApplyWhiteBalance(Keyframe value) : ValuedVideoEffect(value, "WhiteBalance") {}

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override;
};
} // namespace openshot

#endif // APPLYWHITEBALANCE_HPP
