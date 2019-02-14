#include "../../include/effects/ApplyExposure.hpp"
#include "../../include/utilities/MathUtilities.hpp"
#include <QDebug>
#include <QImage>
#include <cfenv>
#include <cmath>
#include <vector>

using namespace std;

namespace openshot {

/// Possible to change just this to change the exposure calculation
inline void exposePixel(uchar *color_out, float *color_in, float exposure) {
    // This exposure calculation just uniformly scales up the color.
    for (auto i=0; i<3; i++) {
        color_out[i] = static_cast<uchar>(
                clamp(color_in[i] * pow(2.0f, exposure) * 255.0f, 0.0f, 255.0f));
    }
}


void ChangeExposure(QImage *image, Keyframe &exposure) {
    unsigned char * bits = image->bits();  //< Pointer to the current pixel of the image
    auto area = image->width() * image->height();
    float pix_color[3];

    for (int32_t pix = 0; pix < area; ++pix) {
        // Convert all pixel colors to fall within [0, 1].
        for (auto i=0; i<3; i++) { pix_color[i] = bits[i] / 255.0f; }
        exposePixel(bits, pix_color, exposure.GetValue(0));
        bits += 4;  //< rgba, hence add 4 for pointer arithmetic.
    }
}

shared_ptr<Frame> ApplyExposure::GetFrame(shared_ptr<Frame> frame,
                                     int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    ChangeExposure(frame_image.get(), value);
    return frame;
}

} // namespace openshot
