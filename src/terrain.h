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

    nm::shader_program default_program;
    nm::shader_program debug_program;
    nm::shader_program normal_program;
    nm::shader_program water_program;

    nm::tex grass_diff;
    nm::tex grass_norm;

    nm::tex cliff_diff;
    nm::tex cliff_norm;
};

nm_ret init(terrain *t);

void update(terrain *t, nm::fvec3 target);

/// Render the mesh and heightmap one time with a specified program.
void render(terrain *t, nm::shader_program *prog, nm::mat4 vp, nm::fvec3 target);

/// Note: target should not have been changed between update and render
/// calls.
void render(
        terrain *t, nm::mat4 vp, nm::fvec3 target, operation_draw draw_op,
        bool is_wireframe);

void cleanup(terrain *t);

#endif //TERRAIN3_TERRAIN_H
