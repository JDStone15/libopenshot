#include "../../include/effects/AdjustVfxColor.h"
#include "../../include/utilities/MathUtilities.h"
#include <QDebug>
#include <QImage>
#include <QUuid>
#include <cfenv>
#include <cmath>
#include <vector>

using namespace std;

namespace openshot {

std::unordered_set<string> AdjustVfxColor::adjustments = {"VisualFxHue",
        "VisualFxSaturation", "VisualFxLightness", "VisualFxOpacity"};

/// Change only this function to change the vfx color adjustment algorithm.
inline void adjustPixel(uchar *color_out, float *color_in, QColor &vfxColor) {
    // This transformation calculation is a linear map from (black, white) onto
    // (clear, vfxColor).

    qreal vfxRgba[4];
    vfxColor.getRgbF(vfxRgba + 0, vfxRgba + 1, vfxRgba + 2, vfxRgba + 3);
    float sum_in = 0.;

    for (auto i=0; i<3; i++) {
        sum_in += color_in[i];
        const float new_color = color_in[i] * vfxRgba[i];
        color_out[i] = static_cast<uchar>(
                clamp(new_color * 255.0f, 0.0f, 255.0f));
    }

    const float new_alpha = color_in[3] * vfxRgba[3];
    // To map black to clear, alpha is scaled by the average of incoming rgb.
    color_out[3] = static_cast<uchar>(
            clamp(new_alpha * (sum_in / 3.) * 255.0f, 0.0f, 255.0f));

}


/// Need not be changed to change the lighting adjustment algorithm.
void ChangeFrameWithVfxColor(QImage *image, QColor &vfxColor) {
    unsigned char * bits = image->bits();  //< Pointer to the current pixel of the image
    unsigned char * first_bit = bits;
    auto area = image->width() * image->height();
    float pix_color[4];  //< rgba

    bits = first_bit;
    for (int32_t pix = 0; pix < area; ++pix) {
        // Convert all pixel colors to fall within [0, 1].
        for (auto i=0; i<4; i++) { pix_color[i] = bits[i] / 255.0f; }
        adjustPixel(bits, pix_color, vfxColor);
        bits += 4;  //< rgba, hence add 4 for pointer arithmetic.
    }
}


AdjustVfxColor::AdjustVfxColor(Keyframe hue, Keyframe saturation,
                               Keyframe lightness, Keyframe opacity)
    : hue(hue), saturation(saturation), lightness(lightness), opacity(opacity) {
    InitCommon();
}


AdjustVfxColor::AdjustVfxColor()
    : hue(1.), saturation(1.), lightness(1.), opacity(1.) {
    InitCommon();
}


void AdjustVfxColor::InitCommon() {
    InitEffectInfo();

    auto uuid = QUuid::createUuid().toString();
    id = uuid.leftRef(uuid.length() - 1)
             .right(uuid.length() - 2)
             .toString()
             .toLocal8Bit()
             .constData();
    info.class_name = "AdjustVfxColor";
    info.name = "AdjustVfxColor";
    info.description = "AdjustVfxColor";
    info.has_audio = false;
    info.has_video = true;

    CalculateVfxColor();
}


void AdjustVfxColor::CalculateVfxColor() {
    vfxColor.setHslF(hue.GetValue(0), saturation.GetValue(0),
                     lightness.GetValue(0), opacity.GetValue(0));
}


float AdjustVfxColor::getHue() { return hue.GetValue(0); }
float AdjustVfxColor::getSaturation() { return saturation.GetValue(0); }
float AdjustVfxColor::getLightness() { return lightness.GetValue(0); }
float AdjustVfxColor::getOpacity() { return opacity.GetValue(0); }


void AdjustVfxColor::setHue(float aHue) {
    hue = Keyframe(aHue);
    CalculateVfxColor();
}
void AdjustVfxColor::setSaturation(float aSaturation) {
    saturation = Keyframe(aSaturation);
    CalculateVfxColor();
}
void AdjustVfxColor::setLightness(float aLightness) {
    lightness = Keyframe(aLightness);
    CalculateVfxColor();
}
void AdjustVfxColor::setOpacity(float anOpacity) {
    opacity = Keyframe(anOpacity);
    CalculateVfxColor();
}


string AdjustVfxColor::Json() { return JsonValue().toStyledString(); }
Json::Value AdjustVfxColor::JsonValue() {
    // Create root json object
    auto root = EffectBase::JsonValue(); // get parent properties
    root["type"] = info.class_name;
    root["vfxhue"] = this->hue.JsonValue();
    root["vfxsaturation"] = this->saturation.JsonValue();
    root["vfxlightness"] = this->lightness.JsonValue();
    root["vfxopacity"] = this->opacity.JsonValue();

    // return JsonValue
    return root;
}


void AdjustVfxColor::SetJson(string json_value) {
    Json::Value root;
    Json::Reader reader;
    const auto success = reader.parse(json_value, root);
    if (!success) {
        throw InvalidJSON{"Json could not be parsed.", ""};
    }

    try {
        SetJsonValue(root);
    } catch (exception e) {
        throw InvalidJSON{"Json is invalid (missing keys or invalid data types",
                          ""};
    }
}


void AdjustVfxColor::SetJsonValue(Json::Value root) {
    EffectBase::SetJsonValue(root);
    bool colorChanged = false;

    if (!root["vfxhue"].isNull()) {
        hue.SetJsonValue(root["vfxhue"]);
        colorChanged = true;
    }
    if (!root["vfxsaturation"].isNull()) {
        saturation.SetJsonValue(root["vfxsaturation"]);
        colorChanged = true;
    }
    if (!root["vfxlightness"].isNull()) {
        lightness.SetJsonValue(root["vfxlightness"]);
        colorChanged = true;
    }
    if (!root["vfxopacity"].isNull()) {
        opacity.SetJsonValue(root["vfxopacity"]);
        colorChanged = true;
    }

    if (colorChanged) {
        CalculateVfxColor();
    }
}


string AdjustVfxColor::PropertiesJSON(int64_t requested_frame) {
    Json::Value root;
    root["id"] = add_property_json("ID", 0.0, "string", Id(), nullptr, -1, -1,
                                   true, requested_frame);
    root["position"] =
        add_property_json("Position", Position(), "float", "", nullptr, 0,
                          1000 * 60 * 30, false, requested_frame);
    root["layer"] = add_property_json("Layer", Layer(), "int", "", nullptr, 0,
                                      1000, false, requested_frame);
    root["start"] = add_property_json("Start", Start(), "float", "", nullptr, 0,
                                      1000 * 60 * 30, false, requested_frame);
    root["end"] = add_property_json("End", End(), "float", "", nullptr, 0,
                                    1000 * 60 * 30, false, requested_frame);
    root["duration"] =
        add_property_json("Duration", Duration(), "float", "", nullptr, 0,
                          1000 * 60 * 30, true, requested_frame);

    // Keyframes
    root["vfxhue"] = add_property_json("VisualFxHue",
            static_cast<float>(hue.GetValue(requested_frame)),
            "float", "", &hue, 0.0, 1.0, false, requested_frame);
    root["vfxsaturation"] = add_property_json("VisualFxSaturation",
            static_cast<float>(saturation.GetValue(requested_frame)),
            "float", "", &saturation, 0.0, 1.0, false, requested_frame);
    root["vfxlightness"] = add_property_json("VisualFxLightness",
            static_cast<float>(lightness.GetValue(requested_frame)),
            "float", "", &lightness, 0.0, 1.0, false, requested_frame);
    root["vfxopacity"] = add_property_json("VisualFxOpacity",
            static_cast<float>(opacity.GetValue(requested_frame)),
            "float", "", &opacity, 0.0, 1.0, false, requested_frame);

    // Return formatted string
    return root.toStyledString();
}


shared_ptr<Frame> AdjustVfxColor::GetFrame(shared_ptr<Frame> frame,
                                           int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    ChangeFrameWithVfxColor(frame_image.get(), vfxColor);
    return frame;
}

} // namespace openshot

