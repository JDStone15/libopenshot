#ifndef ADJUSTLIGHTING_HPP
#define ADJUSTLIGHTING_HPP

#include <unordered_set>
#include "../EffectBase.h"


namespace openshot {
class AdjustLighting : public openshot::EffectBase {
  public:
    static std::unordered_set<string> adjustments;
    openshot::Keyframe black, white, shadow, highlight;

    AdjustLighting();
    AdjustLighting(Keyframe black, Keyframe white,
                   Keyframe shadow, Keyframe highlight);

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override;

    /// Get and Set JSON methods
    string Json() override;
    void SetJson(string value) override;
    Json::Value JsonValue() override;
    void SetJsonValue(Json::Value root) override;

    /// Get all properties for a specific frame (perfect for a UI to display the
    /// current state of all properties at any time)
    string PropertiesJSON(int64_t requested_frame) override;
  private:
    void InitCommon();
};
} // namespace openshot

#endif // ADJUSTLIGHTING_HPP
