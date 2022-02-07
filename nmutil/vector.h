#ifndef NMUTIL_VECTOR_H
#define NMUTIL_VECTOR_H

#include <cassert>
#include <cmath>
#include <cstdint>
#include "math.h"

struct uvec2;

struct ivec2;

struct vec2 {
    float x, y;

    vec2() = default;

    explicit vec2(float a);

    vec2(float x, float y);

    explicit vec2(uvec2 v);

    explicit vec2(ivec2 v);

    vec2 operator+(float a) const;

    vec2 operator+(vec2 v) const;

    void operator+=(vec2 v);

    friend vec2 operator+(float a, const vec2 &v);

    vec2 operator-(float a) const;

    vec2 operator-(vec2 v) const;

    friend vec2 operator-(float a, const vec2 &v);

    vec2 operator*(float a) const;

    vec2 operator*(vec2 v) const;

    void operator*=(float a);

    void operator*=(vec2 v);

    friend vec2 operator*(float a, const vec2 &v);

    vec2 operator/(float a) const;

    vec2 operator/(vec2 v) const;
};

inline vec2::vec2(float a) : x(a), y(a)
{}

inline vec2::vec2(float x, float y) : x(x), y(y)
{}

inline vec2 vec2::operator+(float a) const
{ return vec2(x + a, y + a); }

inline vec2 vec2::operator+(vec2 v) const
{ return vec2(x + v.x, y + v.y); }

inline void vec2::operator+=(vec2 v)
{
    x += v.x;
    y += v.y;
}

inline vec2 operator+(float a, const vec2 &v)
{ return vec2(a + v.x, a + v.y); }

inline vec2 vec2::operator-(float a) const
{ return vec2(x - a, y - a); }

inline vec2 vec2::operator-(vec2 v) const
{ return vec2(x - v.x, y - v.y); }

inline vec2 operator-(float a, const vec2 &v)
{ return vec2(a - v.x, a - v.y); }

inline vec2 vec2::operator*(float a) const
{ return vec2(x * a, y * a); }

inline vec2 vec2::operator*(vec2 v) const
{ return vec2(x * v.x, y * v.y); }

inline void vec2::operator*=(float a)
{
    x *= a;
    y *= a;
}

inline void vec2::operator*=(vec2 v)
{
    x *= v.x;
    y *= v.y;
}

inline vec2 operator*(float a, const vec2 &v)
{ return vec2(a * v.x, a * v.y); }

inline vec2 vec2::operator/(float a) const
{ return vec2(x / a, y / a); }

inline vec2 vec2::operator/(vec2 v) const
{ return vec2(x / v.x, y / v.y); }

/** vec2 non-member */

inline vec2 normalize(vec2 a)
{
    float len = sqrtf(a.x * a.x + a.y * a.y);
    return vec2(a.x / len, a.y / len);
}

inline vec2 floorf(vec2 v)
{ return vec2(floorf(v.x), floorf(v.y)); }

inline vec2 fractf(vec2 v)
{ return v - floorf(v); }

inline float dot(vec2 a, vec2 b)
{ return a.x * b.x + a.y * b.y; }

struct ivec2 {
    int32_t x, y;

    ivec2() = default;

    ivec2(int32_t a);

    ivec2(int32_t x, int32_t y);

    explicit ivec2(vec2 v);

    ivec2 operator+(ivec2 v) const;

    void operator+=(ivec2 v);

    ivec2 operator-(ivec2 v) const;

    ivec2 operator*(int32_t a) const;

    ivec2 operator/(int32_t a) const;

    ivec2 operator&(int32_t a) const;
};

inline ivec2::ivec2(int32_t a) : x(a), y(a)
{}

inline ivec2::ivec2(int32_t x, int32_t y) : x(x), y(y)
{}

inline ivec2::ivec2(vec2 v) : x(int32_t(v.x)), y(int32_t(v.y))
{}

inline ivec2 ivec2::operator+(ivec2 v) const
{ return ivec2(x + v.x, y + v.y); }

inline void ivec2::operator+=(ivec2 v)
{
    x += v.x;
    y += v.y;
}

inline ivec2 ivec2::operator-(ivec2 v) const
{ return ivec2(x - v.x, y - v.y); }

inline ivec2 ivec2::operator*(int32_t a) const
{ return ivec2(x * a, y * a); }

inline ivec2 ivec2::operator/(int32_t a) const
{ return ivec2(x / a, y / a); }

inline ivec2 ivec2::operator&(int32_t a) const
{ return ivec2(x & a, y & a); }

/** vec2 constructor, must be after ivec2 definition */

inline vec2::vec2(ivec2 v) : x(float(v.x)), y(float(v.y))
{}

struct uvec2 {
    uint32_t x, y;

    uvec2() = default;

    uvec2(uint32_t x, uint32_t y);

    explicit uvec2(vec2 v);

    uvec2 operator+(uvec2 v) const;

    uvec2 operator-(uint32_t a) const;

    uvec2 operator-(uvec2 v) const;

    uvec2 operator&(uint32_t a) const;
};

inline uvec2::uvec2(uint32_t x, uint32_t y) : x(x), y(y)
{}

inline uvec2::uvec2(vec2 v) : x(uint32_t(v.x)), y(uint32_t(v.y))
{}

inline uvec2 uvec2::operator+(uvec2 v) const
{ return uvec2(x + v.x, y + v.y); }

inline uvec2 uvec2::operator-(uint32_t a) const
{ return uvec2(x - a, y - a); }

inline uvec2 uvec2::operator-(uvec2 v) const
{ return uvec2(x - v.x, y - v.y); }

inline uvec2 uvec2::operator&(uint32_t a) const
{ return uvec2(x & a, y & a); }

/** vec2 constructor, must be after uvec2 definition */

inline vec2::vec2(uvec2 v) : x(float(v.x)), y(float(v.y))
{}

/** uvec2 non-member */

inline uvec2 maxu(uvec2 a, uvec2 b)
{ return uvec2(maxu(a.x, b.x), maxu(a.y, b.y)); }

struct vec3 {
    float x, y, z;

    vec3() = default;

    vec3(vec3 const &v);

    explicit vec3(float a);

    vec3(float x, float y, float z);

    vec3(float x, vec2 yz);

    static vec3 X();

    static vec3 Y();

    static vec3 Z();

    vec3 operator+(float a) const;

    vec3 operator+(vec3 v) const;

    void operator+=(vec3 v);

    friend vec3 operator+(float a, const vec3 &v);

    vec3 operator-(float a) const;

    vec3 operator-(vec3 v) const;

    void operator-=(vec3 v);

    friend vec3 operator-(float a, const vec3 &v);

    vec3 operator*(float a) const;

    void operator*=(float a);

    vec3 operator*(vec3 v) const;

    friend vec3 operator*(float a, const vec3 &v);

    vec3 operator/(float a) const;
};

inline vec3::vec3(vec3 const &v) : x(v.x), y(v.y), z(v.z)
{}

inline vec3::vec3(float a) : x(a), y(a), z(a)
{}

inline vec3::vec3(float x, float y, float z) : x(x), y(y), z(z)
{}

inline vec3::vec3(float x, vec2 yz) : x(x), y(yz.x), z(yz.y)
{}

inline vec3 vec3::X()
{ return vec3(1.f, 0.f, 0.f); }

inline vec3 vec3::Y()
{ return vec3(0.f, 1.f, 0.f); }

inline vec3 vec3::Z()
{ return vec3(0.f, 0.f, 1.f); }

inline vec3 vec3::operator+(float a) const
{ return vec3(x + a, y + a, z + a); }

inline vec3 vec3::operator+(vec3 v) const
{ return vec3(x + v.x, y + v.y, z + v.z); }

inline void vec3::operator+=(vec3 v)
{
    x += v.x;
    y += v.y;
    z += v.z;
}

inline vec3 operator+(float a, const vec3 &v)
{ return vec3(a + v.x, a + v.y, a + v.z); }

inline vec3 vec3::operator-(float a) const
{ return vec3(x - a, y - a, z - a); }

inline vec3 vec3::operator-(vec3 v) const
{ return vec3(x - v.x, y - v.y, z - v.z); }

inline void vec3::operator-=(vec3 v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
}

inline vec3 operator-(float a, const vec3 &v)
{ return vec3(a - v.x, a - v.y, a - v.z); }

inline vec3 vec3::operator*(float a) const
{ return vec3(x * a, y * a, z * a); }

inline void vec3::operator*=(float a)
{
    x *= a;
    y *= a;
    z *= a;
}

inline vec3 vec3::operator*(vec3 v) const
{ return vec3(x * v.x, y * v.y, z * v.z); }

inline vec3 operator*(float a, const vec3 &v)
{ return vec3(a * v.x, a * v.y, a * v.z); }

inline vec3 vec3::operator/(float a) const
{ return vec3(x / a, y / a, z / a); }

/** vec3 non-member */

inline vec3 floorf(vec3 v)
{ return vec3(floorf(v.x), floorf(v.y), floorf(v.z)); }

inline vec3 fractf(vec3 v)
{ return v - floorf(v); }

inline vec3 cross(vec3 a, vec3 b)
{
    return vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x);
}

inline float dot(vec3 a, vec3 b)
{ return a.x * b.x + a.y * b.y + a.z * b.z; }

inline vec3 normalize(vec3 a)
{
    float len = sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
    return vec3(a.x / len, a.y / len, a.z / len);
}

inline vec3 fminf(vec3 a, vec3 b)
{ return vec3(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z)); }

inline vec3 fmaxf(vec3 a, vec3 b)
{ return vec3(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z)); }

struct vec4 {
    float x, y, z, w;

    vec4() = default;

    explicit vec4(float a);

    vec4(float x, float y, float z, float w);

    vec4(float x, vec3 yzw);

    vec4(vec2 xy, vec2 zw);

    vec4(vec3 xyz, float w);

    vec4 operator+(vec4 v) const;

    void operator+=(vec4 v);

    vec4 operator-() const;

    vec4 operator-(vec4 v) const;

    friend vec4 operator*(const vec4 &v, float a);

    friend vec4 operator*(float a, const vec4 &v);

    void operator*=(float a);
};

inline vec4::vec4(float a) : x(a), y(a), z(a), w(a)
{}

inline vec4::vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{}

inline vec4::vec4(float x, vec3 yzw) : x(x), y(yzw.x), z(yzw.y), w(yzw.z)
{}

inline vec4::vec4(vec2 xy, vec2 zw) : x(xy.x), y(xy.y), z(zw.x), w(zw.y)
{}

inline vec4::vec4(vec3 xyz, float w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w)
{}

inline vec4 vec4::operator+(vec4 v) const
{ return vec4(x + v.x, y + v.y, z + v.z, w + v.w); }

inline void vec4::operator+=(vec4 v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
}

inline vec4 vec4::operator-() const
{ return vec4(-x, -y, -z, -w); }

inline vec4 vec4::operator-(vec4 v) const
{ return vec4(x - v.x, y - v.y, z - v.z, w - v.w); }

inline vec4 operator*(const vec4 &v, float a)
{ return vec4(v.x * a, v.y * a, v.z * a, v.w * a); }

inline vec4 operator*(float a, const vec4 &v)
{ return vec4(a * v.x, a * v.y, a * v.z, a * v.w); }

inline void vec4::operator*=(float a)
{
    x *= a;
    y *= a;
    z *= a;
    w *= a;
}

/** vec4 non-member */

inline float dot(vec4 a, vec4 b)
{ return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

#endif // NMUTIL_VECTOR_H
