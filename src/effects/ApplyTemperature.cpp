#include "../../include/effects/ApplyTemperature.hpp"
#include "../../include/utilities/MathUtilities.hpp"
#include <QDebug>
#include <QImage>
#include <cfenv>
#include <vector>

using namespace std;

namespace openshot {

/// Possible to change just this to change the temperature calculation
inline void heatPixel(uchar *color_out, float *color_in, float temperature) {
    // This heat calculation scales up the red and down the blue.
    const float shift = temperature * 255.0f;

    color_out[0] = static_cast<uchar>(
            clamp(color_in[0] * 255.0f + shift, 0.0f, 255.0f));
    color_out[1] = static_cast<uchar>(
            clamp(color_in[1] * 255.0f, 0.0f, 255.0f));
    color_out[2] = static_cast<uchar>(
            clamp(color_in[2] * 255.0f - shift, 0.0f, 255.0f));
}


void ChangeTemperature(QImage *image, Keyframe &temperature) {
    unsigned char * bits = image->bits();  //< Pointer to the current pixel of the image
    auto area = image->width() * image->height();
    float pix_color[3];

    for (int32_t pix = 0; pix < area; ++pix) {
        // Convert all pixel colors to fall within [0, 1].
        for (auto i=0; i<3; i++) { pix_color[i] = bits[i] / 255.0f; }
        heatPixel(bits, pix_color, temperature.GetValue(0));
        bits += 4;  //< rgba, hence add 4 for pointer arithmetic.
    }
}

shared_ptr<Frame> ApplyTemperature::GetFrame(shared_ptr<Frame> frame,
                                     int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    ChangeTemperature(frame_image.get(), value);
    return frame;
}

} // namespace openshot
