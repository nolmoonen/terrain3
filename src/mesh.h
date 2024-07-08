#ifndef TERRAIN3_MESH_H
#define TERRAIN3_MESH_H

#include "nmutil/vector.h"

#include <glad/gl.h>

#include <cstdint>
#include <vector>

/// This file and its implementation encapsulate the generation and rendering
/// of several vertex mesh "blocks" of which the clipmap mesh exists.

struct instance_data {
    /// Grid-space offset of the mesh in the x/z-plane: (-x,-z)-most point.
    nm::ivec2 offset;
    /// Clipmap LOD level of block.
    uint32_t level;
    /// Debug information.
    /// 0=quadlet, 1=quad, 2=level zero fixup, 3=fixup,
    /// 3=degen -x, 4=degen +x, 5=degen -z, 6=degen +z, 7=trim
    uint32_t id;
    /// Padding to ensure vec4 alignment per std140 rules.
    /// Do not use array, these have different rules altogether.
};

struct draw_info {
    /// Amount of indices this mesh consists of.
    uint32_t index_count;
    /// The offset of the indices for this mesh from the index buffer.
    size_t index_buffer_offset;
    /// Amount of instances to draw.
    uint32_t instance_count;
    /// The offset of this mesh instances of the instance buffer.
    /// Aligned as a multiple of GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT.
    size_t uniform_buffer_offset;
};

struct block {
    /// Offset from start of index buffer.
    size_t offset;
    /// Number of indices in the block.
    size_t count;
    /// Range in grid cells covered by this block, in grid coordinates.
    /// Used for frustum culling.
    nm::uvec2 range;
};

/// Handles the mesh associated with the terrain.
struct mesh {
    /**
     * The terrain is made of many small rectangular, tessellated quads. These
     * quads are instances to create a clipmap where quads further from the
     * target are larger and less detailed. This grid is flat in the x/z-plane
     * and offset in the y-dimension by the vertex shader. */

#define BLOCK_COUNT 12
    /// 3x3 block only used once in level 0.
    block quadlet;
    /// Regular block. Each level has 12, except level 0 which has 16.
    block quad;
    /// Ring fixup in z dimension. Each ring has two between the inner blocks.
    /// Except level 0, which has two.
    block fixup_z;
    /// Ring fixup in x dimension.  Each ring has two between the inner blocks.
    /// Except level 0, which has two.
    block fixup_x;
    /// Interior L-shaped trim -z and +x.
    block trim_neg_z_pos_x;
    /// Interior L-shaped trim +z and +x.
    block trim_pos_z_pos_x;
    /// Interior L-shaped trim +z and -x.
    block trim_pos_z_neg_x;
    /// Interior L-shaped trim -z and -x.
    block trim_neg_z_neg_x;
    /// Degenerate triangle strip -x. Each level has one.
    block degenerate_neg_x;
    /// Degenerate triangle strip -z. Each level has one.
    block degenerate_neg_z;
    /// Degenerate triangle strip +x. Each level has one.
    block degenerate_pos_x;
    /// Degenerate triangle strip +z. Each level has one.
    block degenerate_pos_z;

    GLuint vertex_buffer;
    GLuint index_buffer;
    uint32_t index_count;
    GLuint vertex_array;
};

/// Sets up both the vertex buffer and the index buffer.
void init_mesh(mesh* mesh);

void cleanup_mesh(mesh* mesh);

void render_mesh(mesh* mesh, draw_info di);

#endif //TERRAIN3_MESH_H
