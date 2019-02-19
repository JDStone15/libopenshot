#include "../../include/effects/AdjustLighting.h"
#include "../../include/utilities/MathUtilities.h"
#include <QDebug>
#include <QImage>
#include <QUuid>
#include <cfenv>
#include <cmath>
#include <vector>

using namespace std;

namespace openshot {

std::unordered_set<string> AdjustLighting::adjustments =
        {"Black", "White", "Shadow", "Highlight"};

/// Change these two functions to change the lighting adjustment algorithm.
inline void adjustPixel(uchar *color_out, float *color_in,
                           float *black_in, float *white_in,
                           float *from_black, float *from_white,
                           float *to_black, float *to_white) {
    // This transformation calculation is a linear map from one whiteness domain
    // to another.

    // The blackest and whitest parts of the image are black_in and white_in.

    // The initial domain for the transform, (from_black, from_white), is based
    // upon the colors of the image, and the shadow and highlight values.

    // The target domain (to_black, to_white) is based upon the colors of the
    // image, and the black and white values.

    for (auto i=0; i<3; i++) {
        const float slope = (to_white[i] - to_black[i]) /
                            (from_white[i] - from_black[i]);
        const float new_color = to_black[i] +
                                slope * (color_in[i] - from_black[i]);
        color_out[i] = static_cast<uchar>(
                clamp(new_color * 255.0f, 0.0f, 255.0f));
    }
}


inline void chooseDomains(float *from_black, float *from_white,
                          float *to_black, float *to_white,
                          float *black_in, float *white_in,
                          float black, float white,
                          float shadow, float highlight) {
    // Create initial and target domains for the transform.
    // NOTE: No clamping here... adjustPixel will clamp the final colors.
    for (auto i=0; i<3; i++) {
        const float color_diff = white_in[i] - black_in[i];
        from_black[i] = black_in[i] + color_diff * shadow;
        // highlight and black are negative for a natural feeling UI. 
        from_white[i] = white_in[i] - color_diff * highlight;
        to_black[i] = black_in[i] - color_diff * black;
        to_white[i] = white_in[i] + color_diff * white;
    }
}


/// Need not be changed to change the lighting adjustment algorithm.
void ChangeLighting(QImage *image, Keyframe &black, Keyframe &white,
                              Keyframe &shadow, Keyframe &highlight) {
    unsigned char * bits = image->bits();  //< Pointer to the current pixel of the image
    unsigned char * first_bit = bits;
    auto area = image->width() * image->height();
    float pix_color[3];
    float black_in[3] = { 2.,  2.,  2.};
    float white_in[3] = {-1., -1., -1.};
    float from_black[3], from_white[3], to_black[3], to_white[3];
    
    // Loop over the image once to find "initial black" and "initial white".
    for (int32_t pix = 0; pix < area; ++pix) {
        for (auto i=0; i<3; i++) {
            const float basic_color = bits[i] / 255.0f;
            if (basic_color < black_in[i])
                black_in[i] = basic_color;
            if (basic_color > white_in[i])
                white_in[i] = basic_color;
        }
        bits += 4;  //< rgba, hence add 4 for pointer arithmetic.
    }

    chooseDomains(from_black, from_white, to_black, to_white,
                  black_in, white_in, black.GetValue(0), white.GetValue(0),
                  shadow.GetValue(0), highlight.GetValue(0));

    bits = first_bit;
    for (int32_t pix = 0; pix < area; ++pix) {
        // Convert all pixel colors to fall within [0, 1].
        for (auto i=0; i<3; i++) { pix_color[i] = bits[i] / 255.0f; }
        adjustPixel(bits, pix_color, black_in, white_in,
                       from_black, from_white, to_black, to_white);
        bits += 4;  //< rgba, hence add 4 for pointer arithmetic.
    }
}


AdjustLighting::AdjustLighting(Keyframe black, Keyframe white,
                                         Keyframe shadow, Keyframe highlight)
    : black(black), white(white), shadow(shadow), highlight(highlight) {
    InitCommon();
}


AdjustLighting::AdjustLighting()
    : black(0.), white(0.), shadow(0.), highlight(0.) {
    InitCommon();
}


void AdjustLighting::InitCommon() {
    InitEffectInfo();

    auto uuid = QUuid::createUuid().toString();
    id = uuid.leftRef(uuid.length() - 1)
             .right(uuid.length() - 2)
             .toString()
             .toLocal8Bit()
             .constData();
    info.class_name = "AdjustLighting";
    info.name = "AdjustLighting";
    info.description = "AdjustLighting";
    info.has_audio = false;
    info.has_video = true;
}


string AdjustLighting::Json() { return JsonValue().toStyledString(); }
Json::Value AdjustLighting::JsonValue() {
    // Create root json object
    auto root = EffectBase::JsonValue(); // get parent properties
    root["type"] = info.class_name;
    root["black"] = this->black.JsonValue();
    root["white"] = this->white.JsonValue();
    root["shadow"] = this->shadow.JsonValue();
    root["highlight"] = this->highlight.JsonValue();

    // return JsonValue
    return root;
}

void AdjustLighting::SetJson(string json_value) {
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

void AdjustLighting::SetJsonValue(Json::Value root) {
    EffectBase::SetJsonValue(root);

    if (!root["black"].isNull())
        black.SetJsonValue(root["black"]);
    if (!root["white"].isNull())
        white.SetJsonValue(root["white"]);
    if (!root["shadow"].isNull())
        shadow.SetJsonValue(root["shadow"]);
    if (!root["highlight"].isNull())
        highlight.SetJsonValue(root["highlight"]);
}

string AdjustLighting::PropertiesJSON(int64_t requested_frame) {
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
    root["black"] = add_property_json(
        "Black", static_cast<float>(black.GetValue(requested_frame)),
        "float", "", &black, 0.0, 1.0, false, requested_frame);
    root["white"] = add_property_json(
        "White", static_cast<float>(white.GetValue(requested_frame)),
        "float", "", &white, 0.0, 1.0, false, requested_frame);
    root["shadow"] = add_property_json(
        "Shadow", static_cast<float>(shadow.GetValue(requested_frame)),
        "float", "", &shadow, 0.0, 1.0, false, requested_frame);
    root["highlight"] = add_property_json(
        "Highlight", static_cast<float>(highlight.GetValue(requested_frame)),
        "float", "", &highlight, 0.0, 1.0, false, requested_frame);

    // Return formatted string
    return root.toStyledString();
}

shared_ptr<Frame> AdjustLighting::GetFrame(shared_ptr<Frame> frame,
                                           int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    ChangeLighting(frame_image.get(), black, white, shadow, highlight);
    return frame;
}

} // namespace openshot
