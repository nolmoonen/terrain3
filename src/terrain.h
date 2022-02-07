#ifndef TERRAIN3_TERRAIN_H
#define TERRAIN3_TERRAIN_H

#include <nmutil/matrix.h>
#include <nmutil/gl.h>
#include "geometry.h"
#include "heightmap.h"
#include "comm.h"

/// This file and its implementation encapsulate the terrain generation and
/// representation system.

struct terrain {
    geometry geometry;
    heightmap heightmap;

    nmutil::shader_program default_program;
    nmutil::shader_program debug_program;
    nmutil::shader_program normal_program;
    nmutil::shader_program water_program;

    nmutil::tex grass_diff;
    nmutil::tex grass_norm;

    nmutil::tex cliff_diff;
    nmutil::tex cliff_norm;
};

nm_ret init(terrain *t);

void update(terrain *t, vec3 target);

/// Render the mesh and heightmap one time with a specified program.
void render(terrain *t, nmutil::shader_program *prog, mat4 vp, vec3 target);

/// Note: target should not have been changed between update and render
/// calls.
void render(
        terrain *t, mat4 vp, vec3 target, operation_draw draw_op,
        bool is_wireframe);

void cleanup(terrain *t);

#endif //TERRAIN3_TERRAIN_H
