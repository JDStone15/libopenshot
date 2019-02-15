#ifndef APPLYEXPOSURE_HPP
#define APPLYEXPOSURE_HPP

#include "../EffectBase.h"
#include "ValuedVideoEffect.h"

namespace openshot {
class ApplyExposure : public ValuedVideoEffect {
  public:
    ApplyExposure() : ValuedVideoEffect(Keyframe{}, "Exposure") {}
    ApplyExposure(Keyframe value) : ValuedVideoEffect(value, "Exposure") {}

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override;
};
} // namespace openshot

#endif // APPLYEXPOSURE_HPP
