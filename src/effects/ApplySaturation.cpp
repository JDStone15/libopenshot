#include "../../include/effects/ApplySaturation.hpp"
#include "../../include/utilities/MathUtilities.hpp"
#include <QDebug>
#include <QImage>
#include <cfenv>
#include <cmath>
#include <vector>

using namespace std;

namespace openshot {

/// Possible to change just this to change the saturation calculation
inline void saturatePixel(uchar *color_out, float *color_in, float saturation) {
    // This saturation calculation brightens colors rather linearly.
    const float c_max = *max_element(color_in, color_in+3);  //< max of rgb
    const float c_min = *min_element(color_in, color_in+3);  //< min of rgb
    if (c_max - c_min < 0.001 || c_min < 0.001) {
        for (auto i=0; i<3; i++) {
            color_out[i] = static_cast<uchar>(color_in[i] * 255.0f);
        }
        return;
    }

    const float c_bar = (color_in[0] + color_in[1] + color_in[2]) / 3.0f;
    const float sat_pt = c_min / (c_bar - c_min);

    auto colorshift = [=] (float color) -> float {
        const float slope = (color - c_bar);
        return color + slope * (saturation >= sat_pt ? sat_pt : saturation);
    };

    for (auto i=0; i<3; i++) {
        color_out[i] = static_cast<uchar>(
                clamp(colorshift(color_in[i]) * 255.0f, 0.0f, 255.0f));
    }
}


void ChangeSaturation(QImage *image, Keyframe &saturation) {
    unsigned char * bits = image->bits();  //< Pointer to the current pixel of the image
    auto area = image->width() * image->height();
    float pix_color[3];

    for (int32_t pix = 0; pix < area; ++pix) {
        // Convert all pixel colors to fall within [0, 1].
        for (auto i=0; i<3; i++) { pix_color[i] = bits[i] / 255.0f; }
        saturatePixel(bits, pix_color, saturation.GetValue(0));
        bits += 4;  //< rgba, hence add 4 for pointer arithmetic.
    }
}

shared_ptr<Frame> ApplySaturation::GetFrame(shared_ptr<Frame> frame,
                                     int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    ChangeSaturation(frame_image.get(), value);
    return frame;
}

} // namespace openshot

