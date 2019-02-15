#ifndef APPLYSATURATION_HPP
#define APPLYSATURATION_HPP

#include "../EffectBase.h"

#include "ValuedVideoEffect.h"

namespace openshot {
class ApplySaturation : public ValuedVideoEffect {
  public:
    ApplySaturation() : ValuedVideoEffect(Keyframe{}, "Saturation") {}
    ApplySaturation(Keyframe value) : ValuedVideoEffect(value, "Saturation") {}

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override;
};
} // namespace openshot

#endif // APPLYSATURATION_HPP
