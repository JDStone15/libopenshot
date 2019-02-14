#ifndef APPLYVIBRANCE_HPP
#define APPLYVIBRANCE_HPP

#include "../EffectBase.h"

#include "ValuedVideoEffect.hpp"

namespace openshot {
class ApplyVibrance : public ValuedVideoEffect {
  public:
    ApplyVibrance() : ValuedVideoEffect(Keyframe{}, "Vibrance") {}
    ApplyVibrance(Keyframe value) : ValuedVideoEffect(value, "Vibrance") {}

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override;
};
} // namespace openshot

#endif // APPLYVIBRANCE_HPP
