#ifndef MATHUTILITIES_HPP
#define MATHUTILITIES_HPP

inline float clamp(float d, float min, float max) {
    const float t = d < min ? min : d;
    return t > max ? max : t;
}

inline int clamp(int d, int min, int max) {
    const int t = d < min ? min : d;
    return t > max ? max : t;
}

#endif // MATHUTILITIES_HPP
