#include "../../include/effects/ApplyVibrance.hpp"
#include "../../include/utilities/MathUtilities.hpp"
#include <QDebug>
#include <QImage>
#include <algorithm>
#include <cfenv>
#include <vector>

using namespace std;

namespace openshot {

/// Possible to change just this to change the vibrance calculation
inline void vibrantPixel(uchar *color_out, float *color_in, float vibrance) {
    // This vibrance calculation brightens dull colors more than saturated colors.
    const float c_max = *max_element(color_in, color_in+3);  //< max of rgb
    const float c_min = *min_element(color_in, color_in+3);  //< min of rgb
    if (c_max - c_min < 0.001 || c_min < 0.001) {
        for (auto i=0; i<3; i++) {
            color_out[i] = static_cast<uchar>(color_in[i] * 255.0f);
        }
        return;
    }

    const float sigma = c_min / c_max;  //< equals one minus saturation
    const float c_bar = (color_in[0] + color_in[1] + color_in[2]) / 3.0f;
    const float sat_pt = c_max / (c_bar - c_min);

    auto colorshift = [=] (float color) -> float {
        const float slope = (color - c_bar) * sigma;  //< sigma in the slope punishes saturation
        return color + slope * (vibrance >= sat_pt ? sat_pt : vibrance);
    };

    for (auto i=0; i<3; i++) {
        color_out[i] = static_cast<uchar>(
                clamp(colorshift(color_in[i]) * 255.0f, 0.0f, 255.0f));
    }
}


void ChangeVibrance(QImage *image, Keyframe &vibrance) {
    unsigned char * bits = image->bits();  //< Pointer to the current pixel of the image
    auto area = image->width() * image->height();
    float pix_color[3];

    for (int32_t pix = 0; pix < area; ++pix) {
        // Convert all pixel colors to fall within [0, 1].
        for (auto i=0; i<3; i++) { pix_color[i] = bits[i] / 255.0f; }
        vibrantPixel(bits, pix_color, vibrance.GetValue(0));
        bits += 4;  //< rgba, hence add 4 for pointer arithmetic.
    }
}

shared_ptr<Frame> ApplyVibrance::GetFrame(shared_ptr<Frame> frame,
                                     int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    ChangeVibrance(frame_image.get(), value);
    return frame;
}

} // namespace openshot
