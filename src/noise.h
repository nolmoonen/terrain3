#ifndef TERRAIN3_NOISE_H
#define TERRAIN3_NOISE_H

#include "nmutil/math.h"
#include "nmutil/matrix.h"
#include "nmutil/vector.h"

/// return value noise (in x) in [0,1] and its derivatives (in yz)
/// based on https://www.shadertoy.com/view/4dXBRH
inline nm::fvec3 noised_value(nm::fvec2 p, uint8_t* noise, uint32_t noise_dim, nm::fvec4* dd)
{
    nm::ivec2 i = nm::ivec2(floorf(p));
    nm::fvec2 f = nm::fractf(p);

#if 1
    // quintic interpolation
    nm::fvec2 u   = f * f * f * (f * (f * 6.f - 15.f) + 10.f);
    nm::fvec2 du  = 30.f * f * f * (f * (f - 2.f) + 1.f);
    nm::fvec2 ddu = 60.f * f * (f * (2.f * f - 3.f) + 1.f);
#else
    // cubic interpolation
    nm::fvec2 u  = f * f * (3.f - 2.f * f);
    nm::fvec2 du = 6.f * f * (1.f - f);
#endif

    // samples in [0, 1)
    // AND is valid since noise_dim is power of two
    int32_t s       = int32_t(noise_dim) - 1;
    nm::ivec2 idx_a = (i + nm::ivec2(0, 0)) & s;
    float va        = (float)noise[idx_a.y * noise_dim + idx_a.x] / (float)UINT8_MAX;
    nm::ivec2 idx_b = (i + nm::ivec2(1, 0)) & s;
    float vb        = (float)noise[idx_b.y * noise_dim + idx_b.x] / (float)UINT8_MAX;
    nm::ivec2 idx_c = (i + nm::ivec2(0, 1)) & s;
    float vc        = (float)noise[idx_c.y * noise_dim + idx_c.x] / (float)UINT8_MAX;
    nm::ivec2 idx_d = (i + nm::ivec2(1, 1)) & s;
    float vd        = (float)noise[idx_d.y * noise_dim + idx_d.x] / (float)UINT8_MAX;

    float k0 = va;
    float k1 = vb - va;
    float k2 = vc - va;
    float k4 = va - vb - vc + vd;

    // n
    float value = k0 + k1 * u.x + k2 * u.y + k4 * u.x * u.y;

    // n/dx and n/dy
    nm::fvec2 derivatives(du * (nm::fvec2(u.y, u.x) * k4 + nm::fvec2(k1, k2)));

    // (n/dx)/dx
    dd->x = ddu.x * u.y * k4 + ddu.x * k1;
    // (n/dx)/dy
    dd->y = du.y * k4 * du.x;
    // (n/dy)/dx
    dd->z = du.y * k4 * du.x;
    // (n/dy)/dy
    dd->w = ddu.y * u.x * k4 + ddu.y * k2;

    return nm::fvec3(value, derivatives.x, derivatives.y);
}

/// Note: not normalized, ranges from [0, <2].
inline nm::fvec3 terrain_noise(nm::fvec2 p, uint8_t* noise, uint32_t noise_dim)
{
    float f = 1.9f; // lacunarity
    float s = .55f; // gain

    const nm::mat2 m2  = nm::mat2(.8f, -.6f, .6f, .8f); // rotation matrix
    const nm::mat2 m2t = nm::mat2(.8f, .6f, -.6f, .8f); // and its transpose

    float a     = 0.f; // accumulated height
    float b     = 1.f; // current height multiplier
    nm::fvec2 e = nm::fvec2(0.f); // accumulated helper value to scale the noise with
    nm::fvec2 d = nm::fvec2(0.f); // accumulated derivative

    // inverse of running derivative transform
    nm::mat2 m = nm::mat2::identity();
    // accumulated (modified) partial derivative
    nm::fvec2 t = nm::fvec2(0.f);
    // accumulated (modified) 2nd partial derivative
    nm::fvec4 r = nm::fvec4(0.f);

    // every iteration, the original position is multiplied by f
    // let p' be (f * m2)^i * p
    for (uint32_t i = 0u; i < 12; i++) {
        // second order derivative
        nm::fvec4 dd;
        // get value for p' = m * p and derivative
        nm::fvec3 n = noised_value(p, noise, noise_dim, &dd);
        // the noise derivative for p' (transform by m is part of chain rule)
        // this is actually a row vector, hence m involves the transpose of m2
        nm::fvec2 duxy = m * nm::fvec2(n.y, n.z);
        // accumulate the derivative for p' (without transform by m)
        t += nm::fvec2(n.y, n.z);
        // accumulate the second derivative for p' (without transform by m)
        // since it is the second derivative, it still has to be transformed
        // once (would be twice if we did cancel out one transform, also due
        // to the chain rule)
        const nm::fvec2 xy = m * nm::fvec2(dd.x, dd.y);
        const nm::fvec2 zw = m * nm::fvec2(dd.z, dd.w);
        r += nm::fvec4(xy.x, xy.y, zw.x, zw.y);
        // this is the accumulated factor, which is the
        // derivative for p', but with the transform by m cancelled out
        e += nm::fvec2(n.y, n.z);
        // the term to scale the noise value by
        float term = 1.f + dot(e, e);
        // accumulate values
        a += b * n.x / term;
        // factors to calculate the derivative of (b * n.x / term)
        // this involves the quotient rule
        float x = 2.f * (t.x * r.x + t.y * r.z);
        float y = 2.f * (t.x * r.y + t.y * r.w);
        // accumulate derivative of (b * n.x / term)
        d += b * (term * duxy - n.x * nm::fvec2(x, y)) / (term * term);
        // scaling factor for next iteration
        b *= s;
        // accumulated transform of input point p
        // used to sample a different noise value each octave
        p = f * m2 * p;
        // derivative of p. we use the transpose of m as the matrix m is
        // applied to a row vector
        m = f * m2t * m;
    }

    return nm::fvec3(a, d.x, d.y);
}

#endif // TERRAIN3_NOISE_H
