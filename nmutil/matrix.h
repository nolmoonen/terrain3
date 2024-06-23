#ifndef NMUTIL_MATRIX_H
#define NMUTIL_MATRIX_H

#include "math.h"
#include "vector.h"
#ifndef NM_RTC
#include <cmath>
#include <cstdint>
#endif

namespace nm {

enum class coord { left_handed = 0, right_handed, opengl = right_handed };

struct mat2 {
    mat2() = default;

    NM_SPECIFIERS mat2(float m00, float m01, float m10, float m11);

    /// Identity matrix.
    NM_SPECIFIERS static mat2 identity();

    NM_SPECIFIERS mat2 operator*(float a) const;

    NM_SPECIFIERS friend mat2 operator*(float a, const mat2& m);

    /// Vector is column, right hand side.
    NM_SPECIFIERS fvec2 operator*(fvec2 v) const;

    NM_SPECIFIERS mat2 operator*(mat2 m) const;

    NM_SPECIFIERS float operator[](u32 i) const;

    NM_SPECIFIERS float& operator[](u32 i);

    /// The data array is stored in row-major order.
    float data[4];
};

NM_SPECIFIERS inline mat2::mat2(float m00, float m01, float m10, float m11)
{
    data[0] = m00;
    data[1] = m01;
    data[2] = m10;
    data[3] = m11;
}

NM_SPECIFIERS inline mat2 mat2::identity() { return mat2(1.f, 0.f, 0.f, 1.f); }

NM_SPECIFIERS inline mat2 mat2::operator*(float a) const
{
    return mat2(data[0] * a, data[1] * a, data[2] * a, data[3] * a);
}

NM_SPECIFIERS inline mat2 operator*(float a, const mat2& m)
{
    return mat2(a * m[0], a * m[1], a * m[2], a * m[3]);
}

NM_SPECIFIERS inline fvec2 mat2::operator*(fvec2 v) const
{
    return fvec2(data[0] * v.x + data[1] * v.y, data[2] * v.x + data[3] * v.y);
}

NM_SPECIFIERS inline mat2 mat2::operator*(mat2 m) const
{
    mat2 ret;

    for (u32 i = 0; i < 2; i++) {
        for (u32 j = 0; j < 2; j++) {
            ret[2 * i + j] = 0.f;
            for (u32 k = 0; k < 2; k++) {
                ret[2 * i + j] += data[2 * i + k] * m[2 * k + j];
            }
        }
    }

    return ret;
}

NM_SPECIFIERS inline float mat2::operator[](u32 i) const { return data[i]; }

NM_SPECIFIERS inline float& mat2::operator[](u32 i) { return data[i]; }

struct mat3 {
    mat3() = default;

    NM_SPECIFIERS mat3(
        float m00,
        float m01,
        float m02,
        float m10,
        float m11,
        float m12,
        float m20,
        float m21,
        float m22);

    /// Identity matrix.
    NM_SPECIFIERS static mat3 identity();

    NM_SPECIFIERS mat3 operator*(float a) const;

    NM_SPECIFIERS friend mat3 operator*(float a, const mat3& m);

    /// Vector is column, right hand side.
    NM_SPECIFIERS fvec3 operator*(fvec3 v) const;

    NM_SPECIFIERS mat3 operator*(mat3 m) const;

    NM_SPECIFIERS float operator[](u32 i) const;

    NM_SPECIFIERS float& operator[](u32 i);

    /// The data array is stored in row-major order.
    float data[9];
};

NM_SPECIFIERS inline mat3::mat3(
    float m00,
    float m01,
    float m02,
    float m10,
    float m11,
    float m12,
    float m20,
    float m21,
    float m22)
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

NM_SPECIFIERS inline mat3 mat3::identity()
{
    return mat3(1.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f);
}

NM_SPECIFIERS inline mat3 mat3::operator*(float a) const
{
    return mat3(
        data[0] * a,
        data[1] * a,
        data[2] * a,
        data[3] * a,
        data[4] * a,
        data[5] * a,
        data[6] * a,
        data[7] * a,
        data[8] * a);
}

NM_SPECIFIERS inline mat3 operator*(float a, const mat3& m)
{
    return mat3(
        a * m[0], a * m[1], a * m[2], a * m[3], a * m[4], a * m[5], a * m[6], a * m[7], a * m[8]);
}

NM_SPECIFIERS inline fvec3 mat3::operator*(fvec3 v) const
{
    return fvec3(
        data[0] * v.x + data[1] * v.y + data[2] * v.z,
        data[3] * v.x + data[4] * v.y + data[5] * v.z,
        data[6] * v.x + data[7] * v.y + data[8] * v.z);
}

NM_SPECIFIERS inline mat3 mat3::operator*(mat3 m) const
{
    mat3 ret;

    for (u32 i = 0; i < 3; i++) {
        for (u32 j = 0; j < 3; j++) {
            ret[3 * i + j] = 0.f;
            for (u32 k = 0; k < 3; k++) {
                ret[3 * i + j] += data[3 * i + k] * m[3 * k + j];
            }
        }
    }

    return ret;
}

NM_SPECIFIERS inline float mat3::operator[](u32 i) const { return data[i]; }

NM_SPECIFIERS inline float& mat3::operator[](u32 i) { return data[i]; }

struct mat4 {
    /// Identity matrix.
    NM_SPECIFIERS static mat4 identity();

    NM_SPECIFIERS static mat4 translation(fvec3 v);

    NM_SPECIFIERS static mat4 scaling(float a);

    NM_SPECIFIERS static mat4 scaling(fvec3 v);

    NM_SPECIFIERS static mat4 ortho(
        float left, float right, float bottom, float top, float near_plane, float far_plane);

    NM_SPECIFIERS static mat4 look_at(fvec3 eye, fvec3 center, fvec3 up);

    /// Fov in radians. Flip can be 1 for compatibility with different coordinate system.
    NM_SPECIFIERS static mat4 perspective(
        float fovy, float aspect, float near_plane, float far_plane, coord c);

    NM_SPECIFIERS static mat4 euler_angle_yxz(float yaw, float pitch, float roll);

    NM_SPECIFIERS static mat4 rotate(mat4 m, float angle, fvec3 v);

    /// Vector is column, right hand side.
    NM_SPECIFIERS fvec4 operator*(fvec4 v) const;

    NM_SPECIFIERS fvec3 apply(const fvec3&) const;

    NM_SPECIFIERS mat4 operator*(mat4 a) const;

    NM_SPECIFIERS float& operator[](u32 i);

    NM_SPECIFIERS float& get(int i, int j);

    NM_SPECIFIERS float get(int i, int j) const;

    NM_SPECIFIERS void set_row(int i, const fvec4& v);

    /// The data array is stored in row-major order.
    float data[16];
};

typedef mat4 fmat4; // todo generalize

NM_SPECIFIERS inline mat4 mat4::identity()
{
    mat4 tmp;
    tmp.data[0]  = 1.f;
    tmp.data[1]  = 0.f;
    tmp.data[2]  = 0.f;
    tmp.data[3]  = 0.f;
    tmp.data[4]  = 0.f;
    tmp.data[5]  = 1.f;
    tmp.data[6]  = 0.f;
    tmp.data[7]  = 0.f;
    tmp.data[8]  = 0.f;
    tmp.data[9]  = 0.f;
    tmp.data[10] = 1.f;
    tmp.data[11] = 0.f;
    tmp.data[12] = 0.f;
    tmp.data[13] = 0.f;
    tmp.data[14] = 0.f;
    tmp.data[15] = 1.f;

    return tmp;
}

NM_SPECIFIERS inline mat4 mat4::translation(fvec3 v)
{
    mat4 ret = identity();
    ret[3]   = v.x;
    ret[7]   = v.y;
    ret[11]  = v.z;

    return ret;
}

NM_SPECIFIERS inline mat4 mat4::scaling(float a) { return scaling(fvec3(a, a, a)); }

NM_SPECIFIERS inline mat4 mat4::scaling(fvec3 v)
{
    mat4 ret = identity();
    ret[0]   = v.x;
    ret[5]   = v.y;
    ret[10]  = v.z;

    return ret;
}

NM_SPECIFIERS inline mat4 mat4::ortho(
    float left, float right, float bottom, float top, float near_plane, float far_plane)
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

    tmp.data[8]  = 0.f;
    tmp.data[9]  = 0.f;
    tmp.data[10] = -2.f / (far_plane - near_plane);
    tmp.data[11] = -(far_plane + near_plane) / (far_plane - near_plane);

    tmp.data[12] = 0.f;
    tmp.data[13] = 0.f;
    tmp.data[14] = 0.f;
    tmp.data[15] = 1.f;

    return tmp;
}

NM_SPECIFIERS inline mat4 mat4::look_at(fvec3 eye, fvec3 center, fvec3 up)
{
    // https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl
    // (lookAtRH)
    fvec3 f(normalize(center - eye)); // forward
    fvec3 s(normalize(cross(f, up))); // right
    fvec3 u(cross(s, f));             // up

    mat4 tmp{};
    tmp.data[0] = s.x;
    tmp.data[1] = s.y;
    tmp.data[2] = s.z;
    tmp.data[3] = -dot(s, eye);

    tmp.data[4] = u.x;
    tmp.data[5] = u.y;
    tmp.data[6] = u.z;
    tmp.data[7] = -dot(u, eye);

    tmp.data[8]  = -f.x;
    tmp.data[9]  = -f.y;
    tmp.data[10] = -f.z;
    tmp.data[11] = dot(f, eye);

    tmp.data[12] = 0.f;
    tmp.data[13] = 0.f;
    tmp.data[14] = 0.f;
    tmp.data[15] = 1.f;

    return tmp;
}

NM_SPECIFIERS inline mat4 mat4::perspective(
    float fovy, float aspect, float near_plane, float far_plane, coord c)
{
    // https://github.com/g-truc/glm/blob/33b4a621a697a305bc3a7610d290677b96beb181/glm/ext/matrix_clip_space.inl
    // (perspectiveRH_NO/perspectiveLH_NO)
    float tan_half_fov_y = tanf(fovy / 2.f);

    float flip = coord::right_handed == c ? -1.f : 1.f;

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

    tmp.data[8]  = 0.f;
    tmp.data[9]  = 0.f;
    // [2][2]
    tmp.data[10] = (far_plane + near_plane) / (far_plane - near_plane) * flip;
    // [2][3]
    tmp.data[11] = -(2.f * far_plane * near_plane) / (far_plane - near_plane);

    tmp.data[12] = 0.f;
    tmp.data[13] = 0.f;
    // [3][2]
    tmp.data[14] = flip;
    tmp.data[15] = 0.f;

    return tmp;
}

NM_SPECIFIERS inline mat4 mat4::euler_angle_yxz(float yaw, float pitch, float roll)
{
    // https://github.com/g-truc/glm/blob/master/glm/gtx/euler_angles.inl
    float tmp_ch = cosf(yaw);
    float tmp_sh = sinf(yaw);
    float tmp_cp = cosf(pitch);
    float tmp_sp = sinf(pitch);
    float tmp_cb = cosf(roll);
    float tmp_sb = sinf(roll);

    mat4 tmp{};
    tmp.data[0]  = tmp_ch * tmp_cb + tmp_sh * tmp_sp * tmp_sb;
    tmp.data[4]  = tmp_sb * tmp_cp;
    tmp.data[8]  = -tmp_sh * tmp_cb + tmp_ch * tmp_sp * tmp_sb;
    tmp.data[12] = 0.f;

    tmp.data[1]  = -tmp_ch * tmp_sb + tmp_sh * tmp_sp * tmp_cb;
    tmp.data[5]  = tmp_cb * tmp_cp;
    tmp.data[9]  = tmp_sb * tmp_sh + tmp_ch * tmp_sp * tmp_cb;
    tmp.data[13] = 0.f;

    tmp.data[2]  = tmp_sh * tmp_cp;
    tmp.data[6]  = -tmp_sp;
    tmp.data[10] = tmp_ch * tmp_cp;
    tmp.data[14] = 0.f;

    tmp.data[3]  = 0.f;
    tmp.data[7]  = 0.f;
    tmp.data[11] = 0.f;
    tmp.data[15] = 1.f;

    return tmp;
}

NM_SPECIFIERS inline fvec4 mat4::operator*(fvec4 v) const
{
    return fvec4(
        data[0] * v.x + data[1] * v.y + data[2] * v.z + data[3] * v.w,
        data[4] * v.x + data[5] * v.y + data[6] * v.z + data[7] * v.w,
        data[8] * v.x + data[9] * v.y + data[10] * v.z + data[11] * v.w,
        data[12] * v.x + data[13] * v.y + data[14] * v.z + data[15] * v.w);
}

NM_SPECIFIERS inline float& mat4::operator[](u32 i) { return data[i]; }

NM_SPECIFIERS inline mat4 mat4::rotate(mat4 m, float angle, fvec3 v)
{
    // https://github.com/g-truc/glm/blob/master/glm/ext/matrix_transform.inl
    float const a = angle;
    float const c = cosf(a);
    float const s = sinf(a);

    fvec3 axis = normalize(v);
    fvec3 tmp  = axis * (1.f - c);

    fvec3 rotate0;
    rotate0.x = c + tmp.x * axis.x;
    rotate0.y = tmp.x * axis.y + s * axis.z;
    rotate0.z = tmp.x * axis.z - s * axis.y;

    fvec3 rotate1;
    rotate1.x = tmp.y * axis.x - s * axis.z;
    rotate1.y = c + tmp.y * axis.y;
    rotate1.z = tmp.y * axis.z + s * axis.x;

    fvec3 rotate2;
    rotate2.x = tmp.z * axis.x + s * axis.y;
    rotate2.y = tmp.z * axis.y - s * axis.x;
    rotate2.z = c + tmp.z * axis.z;

    fvec4 m0 = fvec4(m[0], m[4], m[8], m[12]);
    fvec4 m1 = fvec4(m[1], m[5], m[9], m[13]);
    fvec4 m2 = fvec4(m[2], m[6], m[10], m[14]);
    fvec4 m3 = fvec4(m[3], m[7], m[11], m[15]);

    fvec4 res0 = m0 * rotate0.x + m1 * rotate0.y + m2 * rotate0.z;
    fvec4 res1 = m0 * rotate1.x + m1 * rotate1.y + m2 * rotate1.z;
    fvec4 res2 = m0 * rotate2.x + m1 * rotate2.y + m2 * rotate2.z;
    fvec4 res3 = m3;

    mat4 res;
    res[0]  = res0.x;
    res[1]  = res1.x;
    res[2]  = res2.x;
    res[3]  = res3.x;
    res[4]  = res0.y;
    res[5]  = res1.y;
    res[6]  = res2.y;
    res[7]  = res3.y;
    res[8]  = res0.z;
    res[9]  = res1.z;
    res[10] = res2.z;
    res[11] = res3.z;
    res[12] = res0.w;
    res[13] = res1.w;
    res[14] = res2.w;
    res[15] = res3.w;

    return res;
}

NM_SPECIFIERS inline mat4 mat4::operator*(mat4 a) const
{
    mat4 tmp{};

    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            tmp[4 * i + j] = 0.f;
            for (u32 k = 0; k < 4; k++) {
                tmp[4 * i + j] += data[4 * i + k] * a[4 * k + j];
            }
        }
    }

    return tmp;
}

NM_SPECIFIERS inline fvec3 mat4::apply(const fvec3& a) const
{
    float w = get(3, 3);

    w += get(3, 0) * a.x;
    w += get(3, 1) * a.y;
    w += get(3, 2) * a.z;
    const float invW = 1.f / w;

    fvec3 res(0);

    res.x = get(0, 3);
    res.x += a.x * get(0, 0);
    res.x += a.y * get(0, 1);
    res.x += a.z * get(0, 2);
    res.x *= invW;

    res.y = get(1, 3);
    res.y += a.x * get(1, 0);
    res.y += a.y * get(1, 1);
    res.y += a.z * get(1, 2);
    res.y *= invW;

    res.z = get(2, 3);
    res.z += a.x * get(2, 0);
    res.z += a.y * get(2, 1);
    res.z += a.z * get(2, 2);
    res.z *= invW;

    return res;
}

NM_SPECIFIERS inline void mat4::set_row(int i, const fvec4& v)
{
    data[i * 4 + 0] = v.x;
    data[i * 4 + 1] = v.y;
    data[i * 4 + 2] = v.z;
    data[i * 4 + 3] = v.w;
}

NM_SPECIFIERS inline float& mat4::get(int i, int j) { return data[i * 4 + j]; }

NM_SPECIFIERS inline float mat4::get(int i, int j) const { return data[i * 4 + j]; }

// Code for inversion taken from:
// http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
NM_SPECIFIERS inline fmat4 invert(const fmat4& aMatrix)
{
    const float* m = aMatrix.data;
    float inv[16], det;
    int i;

    inv[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
             m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] -
             m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
             m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] -
              m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] -
             m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
             m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
             m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] +
              m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
             m[13] * m[2] * m[7] - m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
             m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
              m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] -
              m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
             m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] - m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
              m[8] * m[1] * m[7] + m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0) {
        return fmat4::identity();
    }

    det = 1.f / det;

    fmat4 res;
    for (i = 0; i < 16; i++) {
        res[i] = inv[i] * det;
    }

    return res;
}

} // namespace nm

#endif // NMUTIL_MATRIX_H