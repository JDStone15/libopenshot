#ifndef APPLYTEMPERATURE_HPP
#define APPLYTEMPERATURE_HPP

#include "../EffectBase.h"

#include "ValuedVideoEffect.h"

namespace openshot {
class ApplyTemperature : public ValuedVideoEffect {
  public:
    ApplyTemperature() : ValuedVideoEffect(Keyframe{}, "Temperature") {}
    ApplyTemperature(Keyframe value) : ValuedVideoEffect(value, "Temperature") {}

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override;
};
} // namespace openshot

#endif // APPLYTEMPERATURE_HPP
