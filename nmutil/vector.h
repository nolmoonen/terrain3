#ifndef NMUTIL_VECTOR_H
#define NMUTIL_VECTOR_H

#include "config.h"
#include "defs.h"

#include <cassert>
#include <cmath>
#include <cstdint>

namespace nm {

template <typename T>
struct vec2 {
    T x, y;

    vec2() = default;

    NM_SPECIFIERS explicit vec2(T s) : x(s), y(s) {}

    NM_SPECIFIERS constexpr vec2(T x, T y) : x(x), y(y) {}

    template <typename S>
    NM_SPECIFIERS constexpr explicit vec2(const vec2<S>& v)
        : x(static_cast<T>(v.x)), y(static_cast<T>(v.y))
    {
    }

    NM_SPECIFIERS vec2& operator+=(const vec2& v)
    {
        x += v.x;
        y += v.y;
        return *this;
    }

    NM_SPECIFIERS vec2& operator+=(T s) { return *this += vec2(s); }

    NM_SPECIFIERS vec2& operator-=(const vec2& v)
    {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    NM_SPECIFIERS vec2& operator-=(T s) { return *this -= vec2(s); }

    NM_SPECIFIERS vec2& operator*=(const vec2& v)
    {
        x *= v.x;
        y *= v.y;
        return *this;
    }

    NM_SPECIFIERS vec2& operator*=(T s) { return *this *= vec2(s); }

    NM_SPECIFIERS vec2& operator/=(const vec2& v)
    {
        x /= v.x;
        y /= v.y;
        return *this;
    }

    NM_SPECIFIERS vec2& operator/=(T s) { return *this /= vec2(s); }

    NM_SPECIFIERS vec2 operator-() { return vec2(-x, -y); }

    NM_SPECIFIERS friend vec2 operator+(vec2 lhs, const vec2& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec2 operator+(vec2 lhs, const T& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec2 operator+(const T& lhs, vec2 rhs)
    {
        vec2 ret(lhs);
        ret += rhs;
        return ret;
    }

    NM_SPECIFIERS friend vec2 operator-(vec2 lhs, const vec2& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec2 operator-(vec2 lhs, const T& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec2 operator-(const T& lhs, vec2 rhs)
    {
        vec2 ret(lhs);
        ret -= rhs;
        return ret;
    }

    NM_SPECIFIERS friend vec2 operator*(vec2 lhs, const vec2& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec2 operator*(vec2 lhs, const T& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec2 operator*(const T& lhs, vec2 rhs)
    {
        vec2 ret(lhs);
        ret *= rhs;
        return ret;
    }

    NM_SPECIFIERS friend vec2 operator/(vec2 lhs, const vec2& rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec2 operator/(vec2 lhs, const T& rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec2 operator/(const T& lhs, vec2 rhs)
    {
        vec2 ret(lhs);
        ret /= rhs;
        return ret;
    }

    NM_SPECIFIERS friend bool operator==(const vec2& lhs, const vec2& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    NM_SPECIFIERS friend bool operator!=(const vec2& lhs, const vec2& rhs) { return !(lhs == rhs); }
};

typedef vec2<f32> fvec2;
typedef vec2<f64> dvec2;
typedef vec2<i32> ivec2;
typedef vec2<u32> uvec2;

NM_SPECIFIERS inline ivec2 toivec2(const uvec2& v) { return {i32(v.x), i32(v.y)}; }
NM_SPECIFIERS inline ivec2 toivec2(const fvec2& v) { return {i32(v.x), i32(v.y)}; }

NM_SPECIFIERS inline uvec2 touvec2(const ivec2& v) { return {u32(v.x), u32(v.y)}; }

NM_SPECIFIERS inline fvec2 tofvec2(const ivec2& v) { return {f32(v.x), f32(v.y)}; }

NM_SPECIFIERS inline ivec2 operator&(ivec2 lhs, const i32& rhs)
{
    return {lhs.x & rhs, lhs.y & rhs};
}

template <typename T>
struct vec3 {
    T x, y, z;

    vec3() = default;

    template <typename S>
    NM_SPECIFIERS constexpr explicit vec3(vec3<S> const& v)
        : x(static_cast<T>(v.x)), y(static_cast<T>(v.y)), z(static_cast<T>(v.z))
    {
    }

    NM_SPECIFIERS constexpr explicit vec3(T s) : x(s), y(s), z(s) {}

    NM_SPECIFIERS constexpr vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    NM_SPECIFIERS static constexpr vec3 X() { return {1, 0, 0}; };

    NM_SPECIFIERS static constexpr vec3 Y() { return {0, 1, 0}; };

    NM_SPECIFIERS static constexpr vec3 Z() { return {0, 0, 1}; };

    NM_SPECIFIERS static constexpr vec3 zero() { return {0, 0, 0}; };

    NM_SPECIFIERS constexpr vec3& operator+=(const vec3& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    NM_SPECIFIERS vec3& operator+=(T s) { return *this += vec3(s); }

    NM_SPECIFIERS constexpr vec3& operator-=(const vec3& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    NM_SPECIFIERS vec3& operator-=(T s) { return *this -= vec3(s); }

    NM_SPECIFIERS constexpr vec3& operator*=(const vec3& v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        return *this;
    }

    NM_SPECIFIERS constexpr vec3& operator*=(T s) { return *this *= vec3(s); }

    NM_SPECIFIERS constexpr vec3& operator/=(const vec3& v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        return *this;
    }

    NM_SPECIFIERS constexpr vec3& operator/=(T s) { return *this /= vec3(s); }

    NM_SPECIFIERS vec3 operator+() const { return vec3(x, y, z); }

    NM_SPECIFIERS vec3 operator-() const { return vec3(-x, -y, -z); }

    NM_SPECIFIERS constexpr friend vec3 operator+(vec3 lhs, const vec3& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec3 operator+(vec3 lhs, const T& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    NM_SPECIFIERS constexpr friend vec3 operator-(vec3 lhs, const vec3& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec3 operator-(vec3 lhs, const T& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    // TODO: ambiguous
    NM_SPECIFIERS friend vec3 operator*(vec3 lhs, const vec3& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    NM_SPECIFIERS constexpr friend vec3 operator*(vec3 lhs, const T& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    NM_SPECIFIERS constexpr friend vec3 operator*(T lhs, const vec3& rhs)
    {
        return vec3(lhs) *= rhs;
    }

    NM_SPECIFIERS constexpr friend vec3 operator/(vec3 lhs, const vec3& rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec3 operator/(vec3 lhs, const T& rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend bool operator==(const vec3& lhs, const vec3& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
    }

    NM_SPECIFIERS friend bool operator!=(const vec3& lhs, const vec3& rhs) { return !(lhs == rhs); }
};

typedef vec3<f32> fvec3;
typedef vec3<f64> dvec3;
typedef vec3<i32> ivec3;
typedef vec3<u32> uvec3;

NM_SPECIFIERS inline ivec3 operator%(ivec3 lhs, const i32& rhs)
{
    return {lhs.x % rhs, lhs.y % rhs, lhs.z % rhs};
}

NM_SPECIFIERS inline uvec3 operator%(uvec3 lhs, const u32& rhs)
{
    return {lhs.x % rhs, lhs.y % rhs, lhs.z % rhs};
}

NM_SPECIFIERS inline uvec3 operator&(uvec3 lhs, const u32& rhs)
{
    return {lhs.x & rhs, lhs.y & rhs, lhs.z & rhs};
}

template <typename T>
struct vec4 {
    T x, y, z, w;

    vec4() = default;

    NM_SPECIFIERS vec4(vec4 const& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

    NM_SPECIFIERS vec4(vec3<T> const& v, T w) : x(v.x), y(v.y), z(v.z), w(w) {}

    NM_SPECIFIERS explicit vec4(T s) : x(s), y(s), z(s), w(s) {}

    NM_SPECIFIERS vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    NM_SPECIFIERS vec4& operator+=(const vec4& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }

    NM_SPECIFIERS vec4& operator+=(T s) { return *this += vec4(s); }

    NM_SPECIFIERS vec4& operator-=(const vec4& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }

    NM_SPECIFIERS vec4& operator-=(T s) { return *this -= vec4(s); }

    NM_SPECIFIERS vec4& operator*=(const vec4& v)
    {
        x *= v.x;
        y *= v.y;
        z *= v.z;
        w *= v.w;
        return *this;
    }

    NM_SPECIFIERS vec4& operator*=(T s) { return *this *= vec4(s); }

    NM_SPECIFIERS vec4& operator/=(const vec4& v)
    {
        x /= v.x;
        y /= v.y;
        z /= v.z;
        w /= v.w;
        return *this;
    }

    NM_SPECIFIERS vec4& operator/=(T s) { return *this /= vec4(s); }

    NM_SPECIFIERS vec4 operator-() { return vec4(-x, -y, -z, -w); }

    NM_SPECIFIERS friend vec4 operator+(vec4 lhs, const vec4& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec4 operator+(vec4 lhs, const T& rhs)
    {
        lhs += rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec4 operator-(vec4 lhs, const vec4& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec4 operator-(vec4 lhs, const T& rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec4 operator*(vec4 lhs, const vec4& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec4 operator*(vec4 lhs, const T& rhs)
    {
        lhs *= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec4 operator/(vec4 lhs, const vec4& rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend vec4 operator/(vec4 lhs, const T& rhs)
    {
        lhs /= rhs;
        return lhs;
    }

    NM_SPECIFIERS friend bool operator==(const vec4& lhs, const vec4& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
    }

    NM_SPECIFIERS friend bool operator!=(const vec4& lhs, const vec4& rhs) { return !(lhs == rhs); }
};

typedef vec4<f32> fvec4;
typedef vec4<f64> dvec4;
typedef vec4<i32> ivec4;
typedef vec4<u32> uvec4;

NM_SPECIFIERS inline ivec4 toivec4(const uvec4& v)
{
    return {i32(v.x), i32(v.y), i32(v.z), i32(v.w)};
}
NM_SPECIFIERS inline ivec4 toivec4(const fvec4& v)
{
    return {i32(v.x), i32(v.y), i32(v.z), i32(v.w)};
}

NM_SPECIFIERS inline uvec4 touvec4(const ivec4& v)
{
    return {u32(v.x), u32(v.y), u32(v.z), u32(v.w)};
}

NM_SPECIFIERS inline fvec4 tofvec4(const ivec4& v)
{
    return {f32(v.x), f32(v.y), f32(v.z), f32(v.w)};
}

} // namespace nm

#endif // NMUTIL_VECTOR_H