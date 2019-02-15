#include "../../include/effects/ValuedVideoEffect.h"

#include <QUuid>
#include <algorithm>

using namespace std;

namespace openshot {

ValuedVideoEffect::ValuedVideoEffect(Keyframe value, string name)
    : value(value) {
    InitEffectInfo();

    auto uuid = QUuid::createUuid().toString();
    id = uuid.leftRef(uuid.length() - 1)
             .right(uuid.length() - 2)
             .toString()
             .toLocal8Bit()
             .constData();
    info.class_name = name;
    info.name = name;
    info.description = name;
    info.has_audio = false;
    info.has_video = true;
}

string ValuedVideoEffect::Json() { return JsonValue().toStyledString(); }
Json::Value ValuedVideoEffect::JsonValue() {
    // Create root json object
    auto root = EffectBase::JsonValue(); // get parent properties
    root["value"] = this->value.JsonValue();

    // return JsonValue
    return root;
}

void ValuedVideoEffect::SetJson(string json_value) {
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

void ValuedVideoEffect::SetJsonValue(Json::Value root) {
    EffectBase::SetJsonValue(root);

    if (!root["value"].isNull()) {
        value.SetJsonValue(root["value"]);
    }
}

string ValuedVideoEffect::PropertiesJSON(int64_t requested_frame) {
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
    root["value"] = add_property_json(
        "Value", static_cast<float>(value.GetValue(requested_frame)), "float",
        "", &value, 0.0, 1.0, false, requested_frame);

    // Return formatted string
    return root.toStyledString();
}

} // namespace openshot
