#include "../../include/effects/ApplyWhiteBalance.hpp"
#include "../../include/utilities/MathUtilities.hpp"
#include <QDebug>
#include <QImage>
#include <algorithm>
#include <cfenv>
#include <vector>

using namespace std;

namespace openshot {

/// Possible to change just this to change the white balance calculation
inline void balancePixel(uchar *color_out, float *color_in, float *white_in) {
    // This white balance calculation rescales all colors according to "white".
    for (auto i=0; i<3; i++) {
        color_out[i] = static_cast<uchar>(
                clamp(color_in[i] / white_in[i] * 255.0f, 0.0f, 255.0f));
    }
}


void ChangeWhiteBalance(QImage *image, Keyframe &white) {
    unsigned char * bits = image->bits();  //< Pointer to the current pixel of the image
    auto area = image->width() * image->height();
    float pix_color[3];
    float white_in[3];
    for (auto i=0; i<3; i++) { white_in[i] = white.GetPoint(i).co.Y / 255.0f; }

    for (int32_t pix = 0; pix < area; ++pix) {
        // Convert all pixel colors to fall within [0, 1].
        for (auto i=0; i<3; i++) { pix_color[i] = bits[i] / 255.0f; }
        balancePixel(bits, pix_color, white_in);
        bits += 4;  //< rgba, hence add 4 for pointer arithmetic.
    }
}

shared_ptr<Frame> ApplyWhiteBalance::GetFrame(shared_ptr<Frame> frame,
                                     int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    ChangeWhiteBalance(frame_image.get(), value);
    return frame;
}

} // namespace openshot
