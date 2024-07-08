#ifndef NMUTIL_MATH_H
#define NMUTIL_MATH_H

#include "vector.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>

namespace nm {

NM_VAR_SPECIFIER constexpr float e         = 2.7182818284590452354; /* e */
NM_VAR_SPECIFIER constexpr float log2e     = 1.4426950408889634074; /* log_2 e */
NM_VAR_SPECIFIER constexpr float log10e    = 0.43429448190325182765; /* log_10 e */
NM_VAR_SPECIFIER constexpr float ln2       = 0.69314718055994530942; /* log_e 2 */
NM_VAR_SPECIFIER constexpr float ln10      = 2.30258509299404568402; /* log_e 10 */
NM_VAR_SPECIFIER constexpr float pi        = 3.14159265358979323846; /* pi */
NM_VAR_SPECIFIER constexpr float pi2       = 1.57079632679489661923; /* pi/2 */
NM_VAR_SPECIFIER constexpr float pi4       = 0.78539816339744830962; /* pi/4 */
NM_VAR_SPECIFIER constexpr float inv_pi    = 0.31830988618379067154; /* 1/pi */
NM_VAR_SPECIFIER constexpr float inv_pi2   = 0.63661977236758134308; /* 2/pi */
NM_VAR_SPECIFIER constexpr float sqrt2     = 1.41421356237309504880; /* sqrt(2) */
NM_VAR_SPECIFIER constexpr float inv_sqrt2 = 0.70710678118654752440; /* 1/sqrt(2) */

NM_SPECIFIERS inline i32 min(i32 a, i32 b) { return a < b ? a : b; }

NM_SPECIFIERS inline u32 min(u32 a, u32 b) { return a < b ? a : b; }

NM_SPECIFIERS inline float min(float a, float b) { return fminf(a, b); }

NM_SPECIFIERS inline double min(double a, double b) { return fmin(a, b); }

/// CUDA has no long double support (so no __device__ and not in RTC).
#ifndef NM_RTC
inline long double min(long double a, long double b) { return fminl(a, b); }
#endif

NM_SPECIFIERS inline i32 max(i32 a, i32 b) { return a < b ? b : a; }

NM_SPECIFIERS inline u32 max(u32 a, u32 b) { return a < b ? b : a; }

NM_SPECIFIERS inline float max(float a, float b) { return fmaxf(a, b); }

NM_SPECIFIERS inline double max(double a, double b) { return fmax(a, b); }

/// CUDA has no long double support (so no __device__ and not in RTC).
#ifndef NM_RTC
inline long double max(long double a, long double b) { return fmaxl(a, b); }
#endif

/// Illustrative example: -1.3 -> .3
NM_SPECIFIERS inline float fractf(float a) { return a - ::floorf(a); }

NM_SPECIFIERS inline float clampf(float val, float min, float max)
{
    return fmaxf(min, fminf(val, max));
}

/// Cubic smoothstep.
NM_SPECIFIERS inline float smoothstep(float a, float b, float x)
{
    x = clampf((x - a) / (b - a), 0.f, 1.f);
    return x * x * (3 - 2 * x);
}

/// Divides, but always rounds down.
/// Needed since integer division for negative numbers rounds towards zero.
NM_SPECIFIERS inline i32 idiv(i32 x, i32 m)
{
    if (x >= 0) {
        return x / m; // normal division
    } else {
        return -((-x + m - 1) / m); // negate the result of rounding up
    }
}

NM_SPECIFIERS inline ivec3 idiv(const ivec3& x, const ivec3& m)
{
    return {idiv(x.x, m.x), idiv(x.y, m.y), idiv(x.z, m.z)};
}

NM_SPECIFIERS inline float to_rad(float deg) { return deg * float(pi / 180.f); }

NM_SPECIFIERS inline float to_deg(float rad) { return rad * float(180.f * inv_pi); }

/// GLSL mod (https://docs.gl/sl4/mod)
NM_SPECIFIERS inline float mod(float x, float y) { return x - y * floorf(x / y); }

/* vec2 */

template <typename T>
NM_SPECIFIERS vec2<T> normalize(vec2<T> a)
{
    T len = sqrtf(a.x * a.x + a.y * a.y);
    return vec2<T>(a.x / len, a.y / len);
}

template <typename T>
NM_SPECIFIERS vec2<T> floorf(vec2<T> v)
{
    return vec2<T>(::floorf(v.x), ::floorf(v.y));
}

template <typename T>
NM_SPECIFIERS vec2<T> fractf(vec2<T> v)
{
    return v - floorf(v);
}

template <typename T>
NM_SPECIFIERS T dot(vec2<T> a, vec2<T> b)
{
    return a.x * b.x + a.y * b.y;
}

template <typename T>
NM_SPECIFIERS vec2<T> max(vec2<T> a, vec2<T> b)
{
    return vec2<T>(std::max(a.x, b.x), std::max(a.y, b.y));
}

/* vec3 */

template <typename T>
NM_SPECIFIERS vec3<T> floorf(vec3<T> v)
{
    return vec3<T>(floorf(v.x), floorf(v.y), floorf(v.z));
}

template <typename T>
NM_SPECIFIERS vec3<T> fractf(vec3<T> v)
{
    return v - floorf(v);
}

template <typename T>
NM_SPECIFIERS constexpr vec3<T> cross(const vec3<T>& a, const vec3<T>& b)
{
    return vec3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template <typename T>
NM_SPECIFIERS T dot(vec3<T> a, vec3<T> b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename T>
NM_SPECIFIERS constexpr T length(vec3<T> a)
{
    // todo technically sqrtf is not constexpr
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

template <typename T>
NM_SPECIFIERS constexpr vec3<T> normalize(const vec3<T>& a)
{
    T len = length(a);
    return vec3<T>(a.x / len, a.y / len, a.z / len);
}

template <typename T>
NM_SPECIFIERS T min(vec3<T> a)
{
    return nm::min(a.x, nm::min(a.y, a.z));
}

template <typename T>
NM_SPECIFIERS vec3<T> min(vec3<T> a, vec3<T> b)
{
    return vec3<T>(nm::min(a.x, b.x), nm::min(a.y, b.y), nm::min(a.z, b.z));
}

template <typename T>
NM_SPECIFIERS T max(vec3<T> a)
{
    return nm::max(a.x, nm::max(a.y, a.z));
}

template <typename T>
NM_SPECIFIERS vec3<T> max(vec3<T> a, vec3<T> b)
{
    return vec3<T>(nm::max(a.x, b.x), nm::max(a.y, b.y), nm::max(a.z, b.z));
}

NM_SPECIFIERS inline fvec3 abs(fvec3 a) { return {fabsf(a.x), fabsf(a.y), fabsf(a.z)}; }

NM_SPECIFIERS inline fvec3 mod(const fvec3& v, float a)
{
    return {mod(v.x, a), mod(v.y, a), mod(v.z, a)};
}

/* vec4 */

template <typename T>
NM_SPECIFIERS inline T dot(vec4<T> a, vec4<T> b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

} // namespace nm

#endif // NMUTIL_MATH_H