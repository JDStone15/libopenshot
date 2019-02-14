#ifndef ADJUSTVFXCOLOR_HPP
#define ADJUSTVFXCOLOR_HPP

#include <unordered_set>
#include "../EffectBase.h"

namespace openshot {
class AdjustVfxColor : public openshot::EffectBase {
  public:
    static std::unordered_set<string> adjustments;

    AdjustVfxColor();
    AdjustVfxColor(Keyframe hue, Keyframe saturation,
                   Keyframe lightness, Keyframe opacity);

    std::shared_ptr<openshot::Frame>
    GetFrame(std::shared_ptr<openshot::Frame> frame,
             int64_t frame_number) override;

    /// Get and Set methods
    float getHue();
    float getSaturation();
    float getLightness();
    float getOpacity();
    void setHue(float aHue);
    void setSaturation(float aSaturation);
    void setLightness(float aLightness);
    void setOpacity(float anOpacity);

    /// Get and Set JSON methods
    string Json() override;
    void SetJson(string value) override;
    Json::Value JsonValue() override;
    void SetJsonValue(Json::Value root) override;

    /// Get all properties for a specific frame (perfect for a UI to display the
    /// current state of all properties at any time)
    string PropertiesJSON(int64_t requested_frame) override;
  private:
    QColor vfxColor;
    openshot::Keyframe hue, saturation, lightness, opacity;
    void InitCommon();
    void CalculateVfxColor();
};
} // namespace openshot

#endif // ADJUSTVFXCOLOR_HPP

