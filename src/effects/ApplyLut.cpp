#include "../../include/effects/ApplyLut.hpp"
#include "../../include/utilities/MathUtilities.hpp"
#include <QDebug>
#include <QImage>
#include <QUuid>
#include <cfenv>

using namespace std;
using namespace openshot;

namespace openshot {

ApplyLut::ApplyLut(shared_ptr<LutFile> lut, Keyframe intensity, QString const &&asset_id)
    : lut(std::move(lut)), intensity(intensity), asset_id(asset_id) {
    InitEffectInfo();

    auto uuid = QUuid::createUuid().toString();
    id = uuid.leftRef(uuid.length() - 1)
             .right(uuid.length() - 2)
             .toString()
             .toLocal8Bit()
             .constData();
    info.class_name = "Lut";
    info.name = "Lut";
    info.description = "Lut"; // Sensing a pattern?
    info.has_audio = false;
    info.has_video = true;
}

/// Take a float color in [0, 1] space to a color in uchar [0, 255] space
inline void to_uchar_rgb(unsigned char *out, float *in) {
    for (auto i = 0; i < 3; i++) {
        out[i] = static_cast<uchar>(clamp(in[i] * 255.0f, 0.0f, 255.0f));
    }
}

void ApplyLut::init_file(shared_ptr<LutFile> lut) { this->lut = lut; }

/// Find a point on a line which is a fraction z of the way between points a and
/// b
inline void linearInterpolate(float *out, float *a, float *b, float *z) {
    for (auto i = 0; i < 3; i++) {
        out[i] = (b[i] - a[i]) * z[i] + a[i];
    }
}

/// Linearly interpolate, scaling by the domain of the LUT
inline void linearInterpolate_within_domain(float *out, float domainSize,
                                            float *a, float *b, float *z) {
    float intermediate[3];
    linearInterpolate(intermediate, a, b, z);

    for (auto i = 0; i < 3; i++) {
        out[i] = clamp(intermediate[i] / domainSize, 0.0f, 1.0f);
    }
}

/// 2D interpolation, using linearInterpolate
inline void bilinearInterpolate(float *out, float *a, float *b, float *c,
                                float *d, float *y, float *z) {
    float v1[3];
    float v2[3];

    linearInterpolate(v1, a, b, z);
    linearInterpolate(v2, c, d, z);
    linearInterpolate(out, v1, v2, y);
}

/// 3D interpolation, using bilinearInterpolate and linearInterpolate_within_domain.
inline void trilinearInterpolate_within_domain(float *out, float domainSize,
                                               float *a, float *b, float *c, float *d,
                                               float *e, float *f, float *g, float *h,
                                               float *x, float *y, float *z) {
    float v1[3];
    float v2[3];

    bilinearInterpolate(v1, a, b, c, d, y, z);
    bilinearInterpolate(v2, e, f, g, h, y, z);
    linearInterpolate_within_domain(out, domainSize, v1, v2, x);
}

inline int GetLut3DIndex_R(int indexR, int indexG, int indexB, int /*sizeR*/,
                           int sizeG, int sizeB) {
    return 3 * (indexB + sizeB * (indexG + sizeG * indexR));
}

inline int GetLut3DIndex_B(int indexR, int indexG, int indexB, int size)

{
    return 3 * (indexR + size * (indexG + size * indexB));
}

inline void lookupNearest_3D_rgb(float *rgb, int rIndex, int gIndex, int bIndex,
                                 int size, const float *simple_rgb_lut) {
    int offset = GetLut3DIndex_B(rIndex, gIndex, bIndex, size);
    for (auto i = 0; i < 3; i++) {
        rgb[i] = simple_rgb_lut[offset + i];
    }
}

///////////////////////////////////////////////////////////////////////
// Linear Forward

void Lut3D_Linear(QImage *image, Keyframe intensity, const LutFile &lut) {
    float maxIndex;
    float mInv;
    float domainMin;
    float mInv_x_maxIndex;
    int lutSize;
    const float *startPos = &(lut.lut[0]);

    maxIndex = lut.size - 1;
    mInv = 1.0f / (lut.domain_max - lut.domain_min);
    domainMin = lut.domain_min;
    mInv_x_maxIndex = static_cast<float>(mInv * maxIndex);

    lutSize = lut.size;

    auto total = image->height() * image->width();
    auto bits = image->bits();

    for (int32_t Y = 0; Y < total; ++Y) {
        float localIndex[3];
        float fLow[3];
        int indexLow[3];
        int indexHigh[3];
        float delta[3];
        float c000[3];
        float c001[3];
        float c010[3];
        float c011[3];
        float c100[3];
        float c101[3];
        float c110[3];
        float c111[3];
        float x[3];
        float y[3];
        float z[3];
        float result[3];

        for (auto i = 0; i < 3; i++) {
            localIndex[i] = clamp(mInv_x_maxIndex * (bits[i] / 255.0f - domainMin),
                                  0.0f, maxIndex);
            fLow[i] = std::floor(localIndex[i]);
            indexLow[i] = static_cast<int>(fLow[i]);
            indexHigh[i] = static_cast<int>(std::ceil(localIndex[i]));
            delta[i] = localIndex[i] - fLow[i];
        }

        // Lookup 8 corners of cube
        lookupNearest_3D_rgb(c000, indexLow[0], indexLow[1], indexLow[2],
                             lutSize, startPos);
        lookupNearest_3D_rgb(c001, indexLow[0], indexLow[1], indexHigh[2],
                             lutSize, startPos);
        lookupNearest_3D_rgb(c010, indexLow[0], indexHigh[1], indexLow[2],
                             lutSize, startPos);
        lookupNearest_3D_rgb(c011, indexLow[0], indexHigh[1], indexHigh[2],
                             lutSize, startPos);
        lookupNearest_3D_rgb(c100, indexHigh[0], indexLow[1], indexLow[2],
                             lutSize, startPos);
        lookupNearest_3D_rgb(c101, indexHigh[0], indexLow[1], indexHigh[2],
                             lutSize, startPos);
        lookupNearest_3D_rgb(c110, indexHigh[0], indexHigh[1], indexLow[2],
                             lutSize, startPos);
        lookupNearest_3D_rgb(c111, indexHigh[0], indexHigh[1], indexHigh[2],
                             lutSize, startPos);

        // Also store the 3d interpolation coordinates
        x[0] = delta[0];
        x[1] = delta[0];
        x[2] = delta[0];
        y[0] = delta[1];
        y[1] = delta[1];
        y[2] = delta[1];
        z[0] = delta[2];
        z[1] = delta[2];
        z[2] = delta[2];

        // Do a trilinear interpolation of the 8 corners
        // 4726.8 scanlines/sec

        trilinearInterpolate_within_domain(result, mInv, c000, c001, c010, c011, c100,
                                           c101, c110, c111, x, y, z);
        // Adjust the result based upon the intensity.
        for (auto i = 0; i < 3; i++) {
            result[i] = (result[i] - bits[i] / 255.0f) * intensity.GetValue(0)
                      + bits[i] / 255.0f;
        }
        to_uchar_rgb(bits, result);

        bits += 4;  //< 4 because rgba.
    }
}

shared_ptr<Frame> ApplyLut::GetFrame(shared_ptr<Frame> frame,
                                     int64_t frame_number) {
    Q_UNUSED(frame_number)

    auto frame_image = frame->GetImage();
    Lut3D_Linear(frame_image.get(), intensity, *lut);
    return frame;
}

string ApplyLut::Json() { return EffectBase::JsonValue().toStyledString(); }
Json::Value ApplyLut::JsonValue() { return EffectBase::JsonValue(); }
void ApplyLut::SetJson(string value) {}
void ApplyLut::SetJsonValue(Json::Value root) {
    EffectBase::SetJsonValue(root);
    asset_id = root["assetId"].asCString();
}

string ApplyLut::PropertiesJSON(int64_t requested_frame) { return ""; }

} // namespace openshot
