#ifndef TERRAIN3_GEOMETRY_H
#define TERRAIN3_GEOMETRY_H

#include <nmutil/matrix.h>
#include <nmutil/gl.h>
#include "terrain_defs.h"
#include "mesh.h"
#include "nmutil/intersect.h"

/// This file and its implementation encapsulate the creation of the geometry
/// of the terrain representation. It maintains the state of which parts of the
/// mesh need to be drawn and at which places.

/// With instanced drawing we can draw each type of mesh with a single call.

struct geometry {
    /// Contains the static mesh which is used to represent the geometry.
    mesh mesh;

    /// UBO maintains the positions and the levels for all meshes.
    GLuint uniform_buffer;
    size_t uniform_buffer_size;

    /// As many draw calls as there are blocks.
    draw_info draw_infos[BLOCK_COUNT];

    /// (-x,-z)-most point of the level's mesh in grid coordinates.
    ivec2 level_offsets[CLIPMAP_LEVEL_COUNT];

    frustum frustum;
};

/// Operates on a grid coordinate.
typedef bool (*trim_cond)(const ivec2 &offset);

void init(geometry *g);

void cleanup(geometry *g);

/// Sets the offset of each level, based on the camera position.
void update_level_offsets(geometry *g, const vec2 &camera_pos);

/// Updates the draw list, which maintains which parts of the mesh are drawn
/// and where.
void update_draw_list(geometry *g);

void render(geometry *g);

#endif //TERRAIN3_GEOMETRY_H
