#include "../../include/effects/ApplySharpness.h"
#include "../../include/utilities/MathUtilities.h"
#include <QDebug>
#include <QImage>
#include <cfenv>
#include <vector>

using namespace std;

namespace openshot {

/// Possible to change just this and blur_const to change the Sharpness calculation.
inline void blurify(float *out, float *c_00, float *c_01, float *c_02,
           float *c_10, float *c_11, float *c_12,
           float *c_20, float *c_21, float *c_22) {
    // The following blur calculation, when used to make the mask for unsharp masking,
    // is equivalent to a weighted averaging of surrounding colors:
    //  -1  -1  -1
    //  -1  +8  -1
    //  -1  -1  -1
    // Thus, the center pixel becomes more like itself, less like its surroundings.

    for (auto i=0; i<3; i++) {
        out[i] = (c_00[i] + c_01[i] + c_02[i]
                + c_10[i] + c_11[i] + c_12[i]
                + c_20[i] + c_21[i] + c_22[i]) / 9.0f;
    }
}

/// The blur function might change the meaning of the sharpness Keyframe, so:
inline float blur_const(Keyframe &sharpness) {
    return sharpness.GetValue(0) * 9.0f;
}

/// Convert a color from [0, 1] space to [0, 255] uchar space.
inline uchar color_to_uchar(float color_in) {
    return static_cast<uchar>(clamp(color_in * 255.0f, 0.0f, 255.0f));
}


void UnsharpMask(QImage *image, Keyframe &sharpness) {
    unsigned char * zero_pixel = image->bits();  //< Pointer to the zeroth pixel of the image
    auto width = image->width();
    auto height = image->height();
    // To remember an entire row of colors
    vector<vector<float> > mem_row_colors(image->width(), vector<float>(3));
    // To remember one color
    vector<float> mem_color(3);

    // A 3x3 box of pixels
    unsigned char * pixel_00;
    unsigned char * pixel_01;
    unsigned char * pixel_02;
    unsigned char * pixel_10;
    unsigned char * pixel_11;
    unsigned char * pixel_12;
    unsigned char * pixel_20;
    unsigned char * pixel_21;
    unsigned char * pixel_22;
    float pixelrgb_00[3];
    float pixelrgb_01[3];
    float pixelrgb_02[3];
    float pixelrgb_10[3];
    float pixelrgb_11[3];
    float pixelrgb_12[3];
    float pixelrgb_20[3];
    float pixelrgb_21[3];
    float pixelrgb_22[3];
    // Unsharp mask applies a blur to calculate a mask with which to sharpen.
    // TODO: Document equations here?
    float blur_color[3];

    // Pixel loop: calculate and apply the sharpened colors.
    for (int32_t y = 0; y <= height; ++y) {   //< Using <= is intentional.
        int pixel_y0 = clamp(y - 1, 0, height - 1) * width * 4;
        int pixel_y1 = clamp(y, 0, height - 1) * width * 4;
        int pixel_y2 = clamp(y + 1, 0, height - 1) * width * 4;
        for (int32_t x = 0; x < width; ++x) {
            int pixel_x0 = clamp(x - 1, 0, width - 1) * 4;
            int pixel_x1 = clamp(x, 0, width - 1) * 4;
            int pixel_x2 = clamp(x + 1, 0, width - 1) * 4;
            // Pointer arithmetic to get this and surrounding pixels.
            // NOTE: colors are rgba, hence the factors of 4 below.
            pixel_00 = zero_pixel + pixel_x0 + pixel_y0;
            pixel_01 = zero_pixel + pixel_x0 + pixel_y1;
            pixel_02 = zero_pixel + pixel_x0 + pixel_y2;
            pixel_10 = zero_pixel + pixel_x1 + pixel_y0;
            pixel_11 = zero_pixel + pixel_x1 + pixel_y1;
            pixel_12 = zero_pixel + pixel_x1 + pixel_y2;
            pixel_20 = zero_pixel + pixel_x2 + pixel_y0;
            pixel_21 = zero_pixel + pixel_x2 + pixel_y1;
            pixel_22 = zero_pixel + pixel_x2 + pixel_y2;

            // Convert all pixel colors to fall within [0, 1].
            for (auto i=0; i<3; i++) {
                pixelrgb_00[i] = pixel_00[i] / 255.0f;
                pixelrgb_01[i] = pixel_01[i] / 255.0f;
                pixelrgb_02[i] = pixel_02[i] / 255.0f;
                pixelrgb_10[i] = pixel_10[i] / 255.0f;
                pixelrgb_11[i] = pixel_11[i] / 255.0f;
                pixelrgb_12[i] = pixel_12[i] / 255.0f;
                pixelrgb_20[i] = pixel_20[i] / 255.0f;
                pixelrgb_21[i] = pixel_21[i] / 255.0f;
                pixelrgb_22[i] = pixel_22[i] / 255.0f;
            }

            blurify(blur_color, pixelrgb_00, pixelrgb_01, pixelrgb_02,
                                pixelrgb_10, pixelrgb_11, pixelrgb_12,
                                pixelrgb_20, pixelrgb_21, pixelrgb_22);

            for (auto i=0; i<3; i++) {
                if (y == height) {
                    // Apply result color to final row.
                    pixel_11[i] = color_to_uchar(mem_row_colors[x][i]);
                    continue;
                } else if(y > 0 && x > 0) {
                    // Apply result color remembered from last x and y.
                    pixel_00[i] = color_to_uchar(mem_color[i]);
                }
                // Remember one color from the last row.
                mem_color[i] = mem_row_colors[x][i];
                if(x == width - 1) {
                    // At end of row, apply result color to last pixel of prev row.
                    pixel_10[i] = color_to_uchar(mem_color[i]);
                }
                // Calculate unsharp mask and remember this row of colors.
                mem_row_colors[x][i] = (blur_const(sharpness) + 1.0f) * pixelrgb_11[i]
                                  - blur_const(sharpness) * blur_color[i];
            }
        }
    }
}

shared_ptr<Frame> ApplySharpness::GetFrame(shared_ptr<Frame> frame,
                                     int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    UnsharpMask(frame_image.get(), value);
    return frame;
}

} // namespace openshot
