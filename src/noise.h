#ifndef TERRAIN3_NOISE_H
#define TERRAIN3_NOISE_H

#include "nmutil/vector.h"
#include "nmutil/math.h"
#include "nmutil/matrix.h"

/// return value noise (in x) in [0,1] and its derivatives (in yz)
/// based on https://www.shadertoy.com/view/4dXBRH
inline vec3 noised_value(vec2 p, uint8_t *noise, uint32_t noise_dim, vec4 *dd)
{
    ivec2 i = ivec2(floorf(p));
    vec2 f = fractf(p);

#if 1
    // quintic interpolation
    vec2 u = f * f * f * (f * (f * 6.f - 15.f) + 10.f);
    vec2 du = 30.f * f * f * (f * (f - 2.f) + 1.f);
    vec2 ddu = 60.f * f * (f * (2.f * f - 3.f) + 1.f);
#else
    // cubic interpolation
    vec2 u = f * f * (3.f - 2.f * f);
    vec2 du = 6.f * f * (1.f - f);
#endif

    // samples in [0, 1)
    // AND is valid since noise_dim is power of two
    int32_t s = int32_t(noise_dim) - 1;
    ivec2 idx_a = (i + ivec2(0, 0)) & s;
    float va = (float) noise[idx_a.y * noise_dim + idx_a.x] / (float) UINT8_MAX;
    ivec2 idx_b = (i + ivec2(1, 0)) & s;
    float vb = (float) noise[idx_b.y * noise_dim + idx_b.x] / (float) UINT8_MAX;
    ivec2 idx_c = (i + ivec2(0, 1)) & s;
    float vc = (float) noise[idx_c.y * noise_dim + idx_c.x] / (float) UINT8_MAX;
    ivec2 idx_d = (i + ivec2(1, 1)) & s;
    float vd = (float) noise[idx_d.y * noise_dim + idx_d.x] / (float) UINT8_MAX;

    float k0 = va;
    float k1 = vb - va;
    float k2 = vc - va;
    float k4 = va - vb - vc + vd;

    // n
    float value = k0 + k1 * u.x + k2 * u.y + k4 * u.x * u.y;

    // n/dx and n/dy
    vec2 derivatives(du * (vec2(u.y, u.x) * k4 + vec2(k1, k2)));

    // (n/dx)/dx
    dd->x = ddu.x * u.y * k4 + ddu.x * k1;
    // (n/dx)/dy
    dd->y = du.y * k4 * du.x;
    // (n/dy)/dx
    dd->z = du.y * k4 * du.x;
    // (n/dy)/dy
    dd->w = ddu.y * u.x * k4 + ddu.y * k2;

    return vec3(value, derivatives);
}

/// Note: not normalized, ranges from [0, <2].
inline vec3 terrain_noise(
        vec2 p, uint8_t *noise, uint32_t noise_dim)
{
    float f = 1.9f; // lacunarity
    float s = .55f; // gain

    const mat2 m2 = mat2(.8f, -.6f, .6f, .8f);  // rotation matrix
    const mat2 m2t = mat2(.8f, .6f, -.6f, .8f); // and its transpose

    float a = 0.f;      // accumulated height
    float b = 1.f;      // current height multiplier
    vec2 e = vec2(0.f); // accumulated helper value to scale the noise with
    vec2 d = vec2(0.f); // accumulated derivative

    // inverse of running derivative transform
    mat2 m = mat2::identity();
    // accumulated (modified) partial derivative
    vec2 t = vec2(0.f);
    // accumulated (modified) 2nd partial derivative
    vec4 r = vec4(0.f);

    // every iteration, the original position is multiplied by f
    // let p' be (f * m2)^i * p
    for (uint32_t i = 0u; i < 12; i++) {
        // second order derivative
        vec4 dd;
        // get value for p' = m * p and derivative
        vec3 n = noised_value(p, noise, noise_dim, &dd);
        // the noise derivative for p' (transform by m is part of chain rule)
        // this is actually a row vector, hence m involves the transpose of m2
        vec2 duxy = m * vec2(n.y, n.z);
        // accumulate the derivative for p' (without transform by m)
        t += vec2(n.y, n.z);
        // accumulate the second derivative for p' (without transform by m)
        // since it is the second derivative, it still has to be transformed
        // once (would be twice if we did cancel out one transform, also due
        // to the chain rule)
        r += vec4(m * vec2(dd.x, dd.y), m * vec2(dd.z, dd.w));
        // this is the accumulated factor, which is the
        // derivative for p', but with the transform by m cancelled out
        e += vec2(n.y, n.z);
        // the term to scale the noise value by
        float term = 1.f + dot(e, e);
        // accumulate values
        a += b * n.x / term;
        // factors to calculate the derivative of (b * n.x / term)
        // this involves the quotient rule
        float x = 2.f * (t.x * r.x + t.y * r.z);
        float y = 2.f * (t.x * r.y + t.y * r.w);
        // accumulate derivative of (b * n.x / term)
        d += b * (term * duxy - n.x * vec2(x, y)) / (term * term);
        // scaling factor for next iteration
        b *= s;
        // accumulated transform of input point p
        // used to sample a different noise value each octave
        p = f * m2 * p;
        // derivative of p. we use the transpose of m as the matrix m is
        // applied to a row vector
        m = f * m2t * m;
    }

    return vec3(a, d);
}

#endif // TERRAIN3_NOISE_H
