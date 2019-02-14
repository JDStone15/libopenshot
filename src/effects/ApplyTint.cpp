#include "../../include/effects/ApplyTint.hpp"
#include "../../include/utilities/MathUtilities.hpp"
#include <QDebug>
#include <QImage>
#include <cfenv>
#include <vector>

using namespace std;

namespace openshot {

/// Possible to change just this to change the tint calculation
inline void tintPixel(uchar *color_out, float *color_in, float tint) {
    // This heat calculation scales up the red and down the blue.
    const float shift = tint * 255.0f;

    color_out[0] = static_cast<uchar>(
            clamp(color_in[0] * 255.0f, 0.0f, 255.0f));
    color_out[1] = static_cast<uchar>(
            clamp(color_in[1] * 255.0f + shift, 0.0f, 255.0f));
    color_out[2] = static_cast<uchar>(
            clamp(color_in[2] * 255.0f, 0.0f, 255.0f));
}


void ChangeTint(QImage *image, Keyframe &tint) {
    unsigned char * bits = image->bits();  //< Pointer to the current pixel of the image
    auto area = image->width() * image->height();
    float pix_color[3];

    for (int32_t pix = 0; pix < area; ++pix) {
        // Convert all pixel colors to fall within [0, 1].
        for (auto i=0; i<3; i++) { pix_color[i] = bits[i] / 255.0f; }
        tintPixel(bits, pix_color, tint.GetValue(0));
        bits += 4;  //< rgba, hence add 4 for pointer arithmetic.
    }
}

shared_ptr<Frame> ApplyTint::GetFrame(shared_ptr<Frame> frame,
                                     int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    ChangeTint(frame_image.get(), value);
    return frame;
}

} // namespace openshot
