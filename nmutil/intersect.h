#ifndef NMUTIL_INTERSECT_H
#define NMUTIL_INTERSECT_H

#include "vector.h"
#include "matrix.h"

struct aabb {
    vec3 min;
    vec3 max;
};

struct frustum {
    /// ax + by + cz + d = 0, normal pointing outward
    vec4 planes[6];
//    /// points of the frustum
//    vec3 points[8];
};

/// vp = proj * view
inline void construct_frustum(frustum *f, mat4 vp)
{
    // matrix row
    vec4 m0(vp[0], vp[1], vp[2], vp[3]);
    vec4 m1(vp[4], vp[5], vp[6], vp[7]);
    vec4 m2(vp[8], vp[9], vp[10], vp[11]);
    vec4 m3(vp[12], vp[13], vp[14], vp[15]);

    f->planes[0] = -(m3 + m0); // left
    f->planes[1] = -(m3 - m0); // right
    f->planes[2] = -(m3 + m1); // bottom
    f->planes[3] = -(m3 - m1); // top
    f->planes[4] = -(m3 + m2); // near
    f->planes[5] = -(m3 - m2); // far
}

inline bool intersect(frustum *f, aabb *bb)
{
    // https://www.iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm

    // check if the box is inside or outside of the frustum
    for (uint32_t i = 0; i < 6; i++) {
        uint32_t out = 0; // number of points outside
        if (dot(f->planes[i], vec4(bb->min.x, bb->min.y, bb->min.z, 1.)) > 0.) {
            out++;
        }
        if (dot(f->planes[i], vec4(bb->max.x, bb->min.y, bb->min.z, 1.)) > 0.) {
            out++;
        }
        if (dot(f->planes[i], vec4(bb->min.x, bb->max.y, bb->min.z, 1.)) > 0.) {
            out++;
        }
        if (dot(f->planes[i], vec4(bb->max.x, bb->max.y, bb->min.z, 1.)) > 0.) {
            out++;
        }
        if (dot(f->planes[i], vec4(bb->min.x, bb->min.y, bb->max.z, 1.)) > 0.) {
            out++;
        }
        if (dot(f->planes[i], vec4(bb->max.x, bb->min.y, bb->max.z, 1.)) > 0.) {
            out++;
        }
        if (dot(f->planes[i], vec4(bb->min.x, bb->max.y, bb->max.z, 1.)) > 0.) {
            out++;
        }
        if (dot(f->planes[i], vec4(bb->max.x, bb->max.y, bb->max.z, 1.)) > 0.) {
            out++;
        }

        // all outside this plane, we are good
        if (out == 8) return false;
    }

    // todo find frustum points by multiplying inverse vp by projected points
    //  (+/-1,+/-1,+/-1)?
//    // box is inside frustum, check if frustum is inside or outside the box
//    // if it is outside, only then we know for sure it is not in the frustum
//    // as the box may intersect the plane behind the camera near plane
//    uint32_t out;
//    out = 0;
//    for (int i = 0; i < 8; i++) out += ((f->points[i].x > bb->max.x) ? 1 : 0);
//    if (out == 8) return false;
//    out = 0;
//    for (int i = 0; i < 8; i++) out += ((f->points[i].x < bb->min.x) ? 1 : 0);
//    if (out == 8) return false;
//    out = 0;
//    for (int i = 0; i < 8; i++) out += ((f->points[i].y > bb->max.y) ? 1 : 0);
//    if (out == 8) return false;
//    out = 0;
//    for (int i = 0; i < 8; i++) out += ((f->points[i].y < bb->min.y) ? 1 : 0);
//    if (out == 8) return false;
//    out = 0;
//    for (int i = 0; i < 8; i++) out += ((f->points[i].z > bb->max.z) ? 1 : 0);
//    if (out == 8) return false;
//    out = 0;
//    for (int i = 0; i < 8; i++) out += ((f->points[i].z < bb->min.z) ? 1 : 0);
//    if (out == 8) return false;

    return true;
}

#endif //NMUTIL_INTERSECT_H
