#ifndef NMUTIL_CAMERA_H
#define NMUTIL_CAMERA_H

#include "math.h"
#include "vector.h"
#include "matrix.h"

struct camera {
    const float MIN_PITCH = -.49f * float(M_PI);
    const float MAX_PITCH = +.49f * float(M_PI);
    const float MAX_ZOOM = 32.f;
    const float MIN_ZOOM = .1f;

    float aspect_ratio;
    float fov;          // vertical field of view (radians!)

    /// Target location around which this camera orbits.
    vec3 target;

    /// Yaw, pitch and roll respectively represented in a vector (in radians).
    vec3 angles;

    /// Used to calculate distance from the camera to the target.
    /// (Further away is higher).
    float zoom_level;
    float near_clipping_dist;
    float far_clipping_dist;

    void init(
            float p_aspect_ratio, float p_fov, float p_near_clipping_dist,
            float p_far_clipping_dist);

    /// Add a given value to the zoom level.
    void add_zoom(float zoom_delta);

    /// Constructs and returns the view matrix for this camera.
    mat4 get_view_matrix();

    /// Constructs and returns the projection matrix for this camera.
    mat4 get_proj_matrix() const;

    /// Calculates camera position.
    vec3 get_camera_position();

    /// Sets the aspect ratio to a specific value.
    void set_aspect(float p_aspect_ratio);

    /// Translate in screen plane using mouse offsets.
    void translate(float offset_xpos, float offset_ypos);

    /// Rotate using mouse offsets.
    void rotate(float offset_xpos, float offset_ypos);
};

inline void camera::init(
        float p_aspect_ratio, float p_fov, float p_near_clipping_dist,
        float p_far_clipping_dist)
{
    aspect_ratio = p_aspect_ratio;
    fov = p_fov;
    near_clipping_dist = p_near_clipping_dist;
    far_clipping_dist = p_far_clipping_dist;

    target = vec3(0.f);
    angles = vec3(0.f);
    zoom_level = 10.f;
}

inline void camera::add_zoom(float zoom_delta)
{
    const float ZOOM_SENSITIVITY = .003f;
    zoom_level -= ZOOM_SENSITIVITY * zoom_delta;
    zoom_level = clampf(zoom_level, MIN_ZOOM, MAX_ZOOM);
}

inline mat4 camera::get_view_matrix()
{
    return mat4::look_at(get_camera_position(), target, vec3(0.f, 1.f, 0.f));
}

inline mat4 camera::get_proj_matrix() const
{
    return mat4::perspective(
            fov, aspect_ratio, near_clipping_dist, far_clipping_dist);
}

inline vec3 camera::get_camera_position()
{
    // distance to focus point is computed as (1.2^zoomConstant)
    float dist = powf(1.2f, zoom_level);

    // pitch, roll and yaw rotation to find world position
    // calculate the rotation matrix
    mat4 rotation = mat4::euler_angle_yxz(angles.y, angles.x, angles.z);

    // apply rotation matrix
    vec4 above(0.f, 0.f, dist, 0.f);
    vec4 pos4 = rotation * above;

    vec3 position(pos4.x, pos4.y, pos4.z);

    return position + target;
}

inline void camera::set_aspect(float p_aspect_ratio)
{
    aspect_ratio = p_aspect_ratio;
}

inline void camera::translate(float offset_xpos, float offset_ypos)
{
    float PANNING_SENSITIVITY = .1f;

    vec3 center = get_camera_position();
    vec3 forward = target - center;
    vec3 world_up = vec3(0.f, 1.f, 0.f);
    vec3 right = normalize(cross(forward, world_up));
    vec3 screen_up = normalize(cross(right, forward));

    // to "move" right, we translate to the left
    target -= PANNING_SENSITIVITY * offset_xpos * right;
    target += PANNING_SENSITIVITY * offset_ypos * screen_up;
}

inline void camera::rotate(float offset_xpos, float offset_ypos)
{
    const float ROTATION_SENSITIVITY = .03f;

    angles.x += (float) -offset_ypos * ROTATION_SENSITIVITY; // pitch
    // enforce max and min pitch
    angles.x = clampf(angles.x, MIN_PITCH, MAX_PITCH);
    angles.y += (float) -offset_xpos * ROTATION_SENSITIVITY; // yaw
}

#endif // NMUTIL_CAMERA_H
