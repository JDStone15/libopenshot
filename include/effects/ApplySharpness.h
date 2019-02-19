#ifndef APPLYSHARPNESS_HPP
#define APPLYSHARPNESS_HPP

#include "../EffectBase.h"

#include "ValuedVideoEffect.h"

namespace openshot {
class ApplySharpness : public ValuedVideoEffect {
  public:
    ApplySharpness() : ValuedVideoEffect(Keyframe{}, "Sharpness") {}
    ApplySharpness(Keyframe value) : ValuedVideoEffect(value, "Sharpness") {}

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override;
};
} // namespace openshot

#endif // APPLYSHARPNESS_HPP
