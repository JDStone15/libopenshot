#ifndef APPLYLUT_HPP
#define APPLYLUT_HPP

#include "../LutFile.hpp"
#include "../EffectBase.h"
#include <QJsonObject>

namespace openshot {
class ApplyLut : public EffectBase {
  public:
    ApplyLut() : ApplyLut(nullptr, Keyframe(1.0), QString()) {}

    shared_ptr<LutFile> lut;
    Keyframe intensity;
    QString asset_id;

    ApplyLut(shared_ptr<LutFile> lut, Keyframe intensity, QString const &&asset_id);
    string get_id() const { return id; }

    void init_file(shared_ptr<LutFile> lut);

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
};
} // namespace openshot

#endif // APPLYLUT_HPP
