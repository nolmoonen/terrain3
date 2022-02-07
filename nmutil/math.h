#ifndef NMUTIL_MATH_H
#define NMUTIL_MATH_H

#include <stdint.h>

#define _USE_MATH_DEFINES

#include <math.h>

/// Illustrative example: -1.3 -> .3
inline float fractf(float a)
{ return a - floorf(a); }

inline float clampf(float val, float min, float max)
{ return fmaxf(min, fminf(val, max)); }

/// Cubic smoothstep.
inline float smoothstep(float a, float b, float x)
{
    x = clampf((x - a) / (b - a), 0.f, 1.f);
    return x * x * (3 - 2 * x);
}

inline uint32_t maxu(uint32_t a, uint32_t b)
{ return a > b ? a : b; }

/// Divides, but always rounds down.
/// Needed since integer division for negative numbers rounds towards zero.
inline int32_t idiv(int32_t x, int32_t m)
{
    if (x >= 0) {
        return x / m; // normal division
    } else {
        return -((-x + m - 1) / m); // negate the result of rounding up
    }
}

inline float to_rad(float deg)
{ return deg * float(M_PI / 180.f); }

inline float to_deg(float rad)
{ return rad * float(180.f * M_1_PI); }

#endif // NMUTIL_MATH_H
