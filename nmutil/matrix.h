#ifndef NMUTIL_MATRIX_H
#define NMUTIL_MATRIX_H

#include <cmath>
#include <cstdint>
#include "vector.h"

struct mat2 {
    mat2() = default;

    mat2(float m00, float m01, float m10, float m11);

    /// Identity matrix.
    static mat2 identity();

    mat2 operator*(float a) const;

    friend mat2 operator*(float a, const mat2 &m);

    /// Vector is column, right hand side.
    vec2 operator*(vec2 v) const;

    mat2 operator*(mat2 m) const;

    float operator[](uint32_t i) const;

    float &operator[](uint32_t i);

    /// The data array is stored in row-major order.
    float data[4];
};

inline mat2::mat2(float m00, float m01, float m10, float m11)
{
    data[0] = m00;
    data[1] = m01;
    data[2] = m10;
    data[3] = m11;
}

inline mat2 mat2::identity()
{ return mat2(1.f, 0.f, 0.f, 1.f); }

inline mat2 mat2::operator*(float a) const
{ return mat2(data[0] * a, data[1] * a, data[2] * a, data[3] * a); }

inline mat2 operator*(float a, const mat2 &m)
{ return mat2(a * m[0], a * m[1], a * m[2], a * m[3]); }

inline vec2 mat2::operator*(vec2 v) const
{ return vec2(data[0] * v.x + data[1] * v.y, data[2] * v.x + data[3] * v.y); }

inline mat2 mat2::operator*(mat2 m) const
{
    mat2 ret;

    for (uint32_t i = 0; i < 2; i++) {
        for (uint32_t j = 0; j < 2; j++) {
            ret[2 * i + j] = 0.f;
            for (uint32_t k = 0; k < 2; k++) {
                ret[2 * i + j] += data[2 * i + k] * m[2 * k + j];
            }
        }
    }

    return ret;
}

inline float mat2::operator[](uint32_t i) const
{ return data[i]; }

inline float &mat2::operator[](uint32_t i)
{ return data[i]; }

struct mat3 {
    mat3() = default;

    mat3(float m00, float m01, float m02,
         float m10, float m11, float m12,
         float m20, float m21, float m22);

    /// Identity matrix.
    static mat3 identity();

    mat3 operator*(float a) const;

    friend mat3 operator*(float a, const mat3 &m);

    /// Vector is column, right hand side.
    vec3 operator*(vec3 v) const;

    mat3 operator*(mat3 m) const;

    float operator[](uint32_t i) const;

    float &operator[](uint32_t i);

    /// The data array is stored in row-major order.
    float data[9];
};

inline mat3::mat3(
        float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22)
{
    data[0] = m00;
    data[1] = m01;
    data[2] = m02;
    data[3] = m10;
    data[4] = m11;
    data[5] = m12;
    data[6] = m20;
    data[7] = m21;
    data[8] = m22;
}

inline mat3 mat3::identity()
{ return mat3(1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f); }

inline mat3 mat3::operator*(float a) const
{
    return mat3(
            data[0] * a, data[1] * a, data[2] * a,
            data[3] * a, data[4] * a, data[5] * a,
            data[6] * a, data[7] * a, data[8] * a);
}

inline mat3 operator*(float a, const mat3 &m)
{
    return mat3(
            a * m[0], a * m[1], a * m[2],
            a * m[3], a * m[4], a * m[5],
            a * m[6], a * m[7], a * m[8]);
}

inline vec3 mat3::operator*(vec3 v) const
{
    return vec3(
            data[0] * v.x + data[1] * v.y + data[2] * v.z,
            data[3] * v.x + data[4] * v.y + data[5] * v.z,
            data[6] * v.x + data[7] * v.y + data[8] * v.z);
}

inline mat3 mat3::operator*(mat3 m) const
{
    mat3 ret;

    for (uint32_t i = 0; i < 3; i++) {
        for (uint32_t j = 0; j < 3; j++) {
            ret[3 * i + j] = 0.f;
            for (uint32_t k = 0; k < 3; k++) {
                ret[3 * i + j] += data[3 * i + k] * m[3 * k + j];
            }
        }
    }

    return ret;
}

inline float mat3::operator[](uint32_t i) const
{ return data[i]; }

inline float &mat3::operator[](uint32_t i)
{ return data[i]; }

struct mat4 {
    /// Identity matrix.
    static mat4 identity();

    static mat4 translation(vec3 v);

    static mat4 scaling(float a);

    static mat4 scaling(vec3 v);

    static mat4 ortho(
            float left, float right, float bottom, float top,
            float near_plane, float far_plane);

    static mat4 look_at(vec3 eye, vec3 center, vec3 up);

    /// Fov in radians.
    static mat4 perspective(
            float fovy, float aspect, float near_plane, float far_plane);

    static mat4 euler_angle_yxz(float yaw, float pitch, float roll);

    static mat4 rotate(mat4 m, float angle, vec3 v);

    /// Vector is column, right hand side.
    vec4 operator*(vec4 v) const;

    mat4 operator*(mat4 a) const;

    float &operator[](uint32_t i);

    /// The data array is stored in row-major order.
    float data[16];
};

inline mat4 mat4::identity()
{
    mat4 tmp;
    tmp.data[0] = 1.f;
    tmp.data[1] = 0.f;
    tmp.data[2] = 0.f;
    tmp.data[3] = 0.f;
    tmp.data[4] = 0.f;
    tmp.data[5] = 1.f;
    tmp.data[6] = 0.f;
    tmp.data[7] = 0.f;
    tmp.data[8] = 0.f;
    tmp.data[9] = 0.f;
    tmp.data[10] = 1.f;
    tmp.data[11] = 0.f;
    tmp.data[12] = 0.f;
    tmp.data[13] = 0.f;
    tmp.data[14] = 0.f;
    tmp.data[15] = 1.f;

    return tmp;
}

inline mat4 mat4::translation(vec3 v)
{
    mat4 ret = identity();
    ret[3] = v.x;
    ret[7] = v.y;
    ret[11] = v.z;

    return ret;
}

inline mat4 mat4::scaling(float a)
{ return scaling(vec3(a, a, a)); }

inline mat4 mat4::scaling(vec3 v)
{
    mat4 ret = identity();
    ret[0] = v.x;
    ret[5] = v.y;
    ret[10] = v.z;

    return ret;
}

inline mat4 mat4::ortho(
        float left, float right, float bottom, float top,
        float near_plane, float far_plane)
{
    // https://en.wikipedia.org/wiki/Orthographic_projection
    mat4 tmp{};
    tmp.data[0] = 2.f / (right - left);
    tmp.data[1] = 0.f;
    tmp.data[2] = 0.f;
    tmp.data[3] = -(right + left) / (right - left);

    tmp.data[4] = 0.f;
    tmp.data[5] = 2.f / (top - bottom);
    tmp.data[6] = 0.f;
    tmp.data[7] = -(top + bottom) / (top - bottom);

    tmp.data[8] = 0.f;
    tmp.data[9] = 0.f;
    tmp.data[10] = -2.f / (far_plane - near_plane);
    tmp.data[11] = -(far_plane + near_plane) / (far_plane - near_plane);

    tmp.data[12] = 0.f;
    tmp.data[13] = 0.f;
    tmp.data[14] = 0.f;
    tmp.data[15] = 1.f;

    return tmp;
}

inline mat4 mat4::look_at(vec3 eye, vec3 center, vec3 up)
{
    // https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl
    // (lookAtRH)
    vec3 f(normalize(center - eye)); // forward
    vec3 s(normalize(cross(f, up))); // right
    vec3 u(cross(s, f));             // up

    mat4 tmp{};
    tmp.data[0] = s.x;
    tmp.data[1] = s.y;
    tmp.data[2] = s.z;
    tmp.data[3] = -dot(s, eye);

    tmp.data[4] = u.x;
    tmp.data[5] = u.y;
    tmp.data[6] = u.z;
    tmp.data[7] = -dot(u, eye);

    tmp.data[8] = -f.x;
    tmp.data[9] = -f.y;
    tmp.data[10] = -f.z;
    tmp.data[11] = dot(f, eye);

    tmp.data[12] = 0.f;
    tmp.data[13] = 0.f;
    tmp.data[14] = 0.f;
    tmp.data[15] = 1.f;

    return tmp;
}

inline mat4 mat4::perspective(
        float fovy, float aspect, float near_plane, float far_plane)
{
    // https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl
    // (perspectiveRH_NO)
    float tan_half_fov_y = tanf(fovy / 2.f);

    mat4 tmp{};
    // [0][0]
    tmp.data[0] = 1.f / (aspect * tan_half_fov_y);
    tmp.data[1] = 0.f;
    tmp.data[2] = 0.f;
    tmp.data[3] = 0.f;

    tmp.data[4] = 0.f;
    // [1][1]
    tmp.data[5] = 1.f / tan_half_fov_y;
    tmp.data[6] = 0.f;
    tmp.data[7] = 0.f;

    tmp.data[8] = 0.f;
    tmp.data[9] = 0.f;
    // [2][2]
    tmp.data[10] = -(far_plane + near_plane) / (far_plane - near_plane);
    // [2][3]
    tmp.data[11] = -(2.f * far_plane * near_plane) / (far_plane - near_plane);

    tmp.data[12] = 0.f;
    tmp.data[13] = 0.f;
    // [3][2]
    tmp.data[14] = -1.f;
    tmp.data[15] = 0.f;

    return tmp;
}

inline mat4 mat4::euler_angle_yxz(float yaw, float pitch, float roll)
{
    // https://github.com/g-truc/glm/blob/master/glm/gtx/euler_angles.inl
    float tmp_ch = cosf(yaw);
    float tmp_sh = sinf(yaw);
    float tmp_cp = cosf(pitch);
    float tmp_sp = sinf(pitch);
    float tmp_cb = cosf(roll);
    float tmp_sb = sinf(roll);

    mat4 tmp{};
    tmp.data[0] = tmp_ch * tmp_cb + tmp_sh * tmp_sp * tmp_sb;
    tmp.data[4] = tmp_sb * tmp_cp;
    tmp.data[8] = -tmp_sh * tmp_cb + tmp_ch * tmp_sp * tmp_sb;
    tmp.data[12] = 0.f;

    tmp.data[1] = -tmp_ch * tmp_sb + tmp_sh * tmp_sp * tmp_cb;
    tmp.data[5] = tmp_cb * tmp_cp;
    tmp.data[9] = tmp_sb * tmp_sh + tmp_ch * tmp_sp * tmp_cb;
    tmp.data[13] = 0.f;

    tmp.data[2] = tmp_sh * tmp_cp;
    tmp.data[6] = -tmp_sp;
    tmp.data[10] = tmp_ch * tmp_cp;
    tmp.data[14] = 0.f;

    tmp.data[3] = 0.f;
    tmp.data[7] = 0.f;
    tmp.data[11] = 0.f;
    tmp.data[15] = 1.f;

    return tmp;
}

inline vec4 mat4::operator*(vec4 v) const
{
    return vec4(
            data[0] * v.x + data[1] * v.y + data[2] * v.z + data[3] * v.w,
            data[4] * v.x + data[5] * v.y + data[6] * v.z + data[7] * v.w,
            data[8] * v.x + data[9] * v.y + data[10] * v.z + data[11] * v.w,
            data[12] * v.x + data[13] * v.y + data[14] * v.z + data[15] * v.w);
}

inline float &mat4::operator[](uint32_t i)
{
    return data[i];
}

inline mat4 mat4::rotate(mat4 m, float angle, vec3 v)
{
    // https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl
    float const a = angle;
    float const c = cosf(a);
    float const s = sinf(a);

    vec3 axis = normalize(v);
    vec3 tmp = (1.f - c) * axis;

    vec3 rotate0;
    rotate0.x = c + tmp.x * axis.x;
    rotate0.y = tmp.x * axis.y + s * axis.z;
    rotate0.z = tmp.x * axis.z - s * axis.y;

    vec3 rotate1;
    rotate1.x = tmp.y * axis.x - s * axis.z;
    rotate1.y = c + tmp.y * axis.y;
    rotate1.z = tmp.y * axis.z + s * axis.x;

    vec3 rotate2;
    rotate2.x = tmp.z * axis.x + s * axis.y;
    rotate2.y = tmp.z * axis.y - s * axis.x;
    rotate2.z = c + tmp.z * axis.z;

    vec4 m0 = vec4(m[0], m[4], m[8], m[12]);
    vec4 m1 = vec4(m[1], m[5], m[9], m[13]);
    vec4 m2 = vec4(m[2], m[6], m[10], m[14]);
    vec4 m3 = vec4(m[3], m[7], m[11], m[15]);

    vec4 res0 = m0 * rotate0.x + m1 * rotate0.y + m2 * rotate0.z;
    vec4 res1 = m0 * rotate1.x + m1 * rotate1.y + m2 * rotate1.z;
    vec4 res2 = m0 * rotate2.x + m1 * rotate2.y + m2 * rotate2.z;
    vec4 res3 = m3;

    mat4 res;
    res[0] = res0.x;
    res[1] = res1.x;
    res[2] = res2.x;
    res[3] = res3.x;
    res[4] = res0.y;
    res[5] = res1.y;
    res[6] = res2.y;
    res[7] = res3.y;
    res[8] = res0.z;
    res[9] = res1.z;
    res[10] = res2.z;
    res[11] = res3.z;
    res[12] = res0.w;
    res[13] = res1.w;
    res[14] = res2.w;
    res[15] = res3.w;

    return res;
}

inline mat4 mat4::operator*(mat4 a) const
{
    mat4 tmp{};

    for (uint32_t i = 0; i < 4; i++) {
        for (uint32_t j = 0; j < 4; j++) {
            tmp[4 * i + j] = 0.f;
            for (uint32_t k = 0; k < 4; k++) {
                tmp[4 * i + j] += data[4 * i + k] * a[4 * k + j];
            }
        }
    }

    return tmp;
}

#endif // NMUTIL_MATRIX_H
