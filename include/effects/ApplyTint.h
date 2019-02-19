#ifndef APPLYTINT_HPP
#define APPLYTINT_HPP

#include "../EffectBase.h"

#include "ValuedVideoEffect.h"

namespace openshot {
class ApplyTint : public ValuedVideoEffect {
  public:
    ApplyTint() : ValuedVideoEffect(Keyframe{}, "Tint") {}
    ApplyTint(Keyframe value) : ValuedVideoEffect(value, "Tint") {}

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override;
};
} // namespace openshot

#endif // APPLYTINT_HPP
