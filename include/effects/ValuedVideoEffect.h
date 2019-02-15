#ifndef VALUEDVIDEOEFFECT_HPP
#define VALUEDVIDEOEFFECT_HPP

#include "../EffectBase.h"

namespace openshot {
class ValuedVideoEffect : public openshot::EffectBase {
  public:
    openshot::Keyframe value;

    ValuedVideoEffect(openshot::Keyframe value, string name);

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override = 0; // pure virtual overrider

    /// Get and Set JSON methods
    string Json() override;
    void SetJson(string value) override;
    Json::Value JsonValue() override;
    void SetJsonValue(Json::Value root) override;

    /// Get all properties for a specific frame (perfect for a UI to display the
    /// current state of all properties at any time)
    string PropertiesJSON(int64_t requested_frame) override;
};
} // namespace openshot

#endif // VALUEDVIDEOEFFECT_HPP
