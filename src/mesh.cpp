#include <cassert>
#include <cstdlib>
#include "mesh.h"
#include "terrain_defs.h"
#include "nmutil/gl.h"
#include "nmutil/util.h"

/// Represents a rectangular, tessellated mesh.
/// The rect will be rendered using triangle strips, where the strips go in the
/// x-dimension. It is important that the winding is CCW and the triangles all
/// have the same orientation in the plane. Due to these requirements, we cannot
/// generate a triangle strip in the x- or z-dimension, only one dimension can
/// be chosen, which we choose to be the x-dimension.
struct rect {
    /** The (-x,-z)-most point of the mesh. */
    uint32_t origin_x;
    uint32_t origin_z;
    uint32_t size_x;
    uint32_t size_z;
};

/// Calculates the number of vertices in the rect.
uint32_t calculate_vertex_count(rect rect)
{ return rect.size_x * rect.size_z; }

/// Sets vertex coordinates for a rect in the x/z-plane. Vertices are spaced
/// one unit apart.
/// Coordinates are x-major and from negative to positive:
/// ^   for o_x=0,o_z=0,size_x=3,size_z=3
/// |   v6 v7 v8
/// z   v3 v4 v5
/// x-> v0 v1 v2
/// Input is pointer to array of x/z-coordinates which is incremented and
/// returned.
GLubyte *generate_vertices(rect rect, GLubyte *vertices)
{
    uint32_t end_x = rect.origin_x + rect.size_x;
    uint32_t end_z = rect.origin_z + rect.size_z;

    // assert that the coordinates can be represented by a byte
    assert(end_x - 1 <= UINT8_MAX && end_z - 1 <= UINT8_MAX);

    // strips will be created in x-direction
    for (uint32_t z = rect.origin_z; z < end_z; z++) {
        for (uint32_t x = rect.origin_x; x < end_x; x++) {
            *(vertices++) = x;
            *(vertices++) = z;
        }
    }

    return vertices;
}

/// Calculates the number of indices required to represent a rect.
uint32_t calculate_index_count(rect rect)
{
    // length of strips in number of vertices
    uint32_t strip_length = rect.size_x;
    // number of strips, strips span two vertices in height
    uint32_t strip_count = rect.size_z - 1u;

    // a strip requires one index for every vertex it covers,
    // +1 for primitive restart
    return strip_count * (2u * strip_length + 1u);
}

/// Calculate the range for the given rect, in grid coordinates.
static nm::uvec2 calculate_range(rect rect)
{ return nm::uvec2(rect.size_x, rect.size_z) - 1; }

/// Triangle winding is to face positive y.
GLushort *generate_indices(GLushort *indices, rect rect, uint32_t offset)
{
    // see calculate_index_count
    uint32_t strip_length = rect.size_x;
    uint32_t strip_count = rect.size_z - 1u;

    GLushort pos = offset;

    // complete a number of strips
    for (uint32_t i = 0; i < strip_count; i++) {
        // the strip has twice the length of the strip
        for (uint32_t j = 0; j < 2u * strip_length; j++) {
            *(indices++) = pos;
            // on even, jump to next row; on odd, jump to prev row and next col
            pos += (j & 1) ? 1 - strip_length : strip_length;
        }

        // indicate that the current strip is complete
        *(indices++) = UINT16_MAX;
    }

    // return the updated pointer to the index buffer
    return indices;
}

void setup_mesh(mesh *mesh)
{
    // note:
    // assume that size is at most 64, saves some vertex space since we can use
    // 8-bit vertex coordinates.

    // create the vertices for the triangle strips -----------------------------

    // 3x3 block, only used once in the middle of level 0
    rect quadlet;
    quadlet.origin_x = 0;
    quadlet.origin_z = 0;
    quadlet.size_x = 3;
    quadlet.size_z = 3;
    uint32_t vertex_count_quadlet = calculate_vertex_count(quadlet);

    // regular MxM block
    rect quad;
    quad.origin_x = 0;
    quad.origin_z = 0;
    quad.size_x = CLIPMAP_SIZE;
    quad.size_z = CLIPMAP_SIZE;
    uint32_t vertex_count_block = calculate_vertex_count(quad);

    // we require 3xM fixup regions between the blocks of every level

    // ring fixup in the z dimension
    rect fixup_z;
    fixup_z.origin_x = 0;
    fixup_z.origin_z = 0;
    fixup_z.size_x = 3;
    fixup_z.size_z = CLIPMAP_SIZE;
    uint32_t vertex_count_fixup_z = calculate_vertex_count(fixup_z);

    // ring fixup in the x dimension
    rect fixup_x;
    fixup_x.origin_x = 0;
    fixup_x.origin_z = 0;
    fixup_x.size_x = CLIPMAP_SIZE;
    fixup_x.size_z = 3;
    uint32_t vertex_count_fixup_x = calculate_vertex_count(fixup_x);

    // we require trim regions of size 2 x (2xM + 1) that surround blocks from a
    // lower LOD, we need four sides

    // todo could do one for top/bottom and one for left/right, although this
    //  increases the number of draw calls

    // trim in the -z direction (previously called top)
    rect trim_neg_z;
    trim_neg_z.origin_x = 0;
    trim_neg_z.origin_z = 0;
    trim_neg_z.size_x = 2 * CLIPMAP_SIZE + 1;
    trim_neg_z.size_z = 2;
    uint32_t vertex_count_trim_neg_z = calculate_vertex_count(trim_neg_z);

    // trim in the +x direction (previously called right)
    rect trim_pos_x;
    trim_pos_x.origin_x = 2 * CLIPMAP_SIZE - 1;
    trim_pos_x.origin_z = 0;
    trim_pos_x.size_x = 2;
    trim_pos_x.size_z = 2 * CLIPMAP_SIZE + 1;
    uint32_t vertex_count_trim_pos_x = calculate_vertex_count(trim_pos_x);

    // trim in the +z direction (previously called bottom)
    rect trim_pos_z;
    trim_pos_z.origin_x = 0;
    trim_pos_z.origin_z = 2 * CLIPMAP_SIZE - 1;
    trim_pos_z.size_x = 2 * CLIPMAP_SIZE + 1;
    trim_pos_z.size_z = 2;
    uint32_t vertex_count_trim_pos_z = calculate_vertex_count(trim_pos_z);

    // trim in the -x direction (previously called left)
    rect trim_neg_x;
    trim_neg_x.origin_x = 0;
    trim_neg_x.origin_z = 0;
    trim_neg_x.size_x = 2;
    trim_neg_x.size_z = 2 * CLIPMAP_SIZE + 1;
    uint32_t vertex_count_trim_neg_x = calculate_vertex_count(trim_neg_x);

    uint32_t vertex_count =
            vertex_count_quadlet + vertex_count_block +
            vertex_count_fixup_z + vertex_count_fixup_x +
            vertex_count_trim_neg_z + vertex_count_trim_pos_x +
            vertex_count_trim_pos_z + vertex_count_trim_neg_x;

    // Degenerate triangles. These are run on the edge between clipmap levels.
    // These are needed to avoid occasional "missing pixels" between clipmap
    // levels as imperfections in precision can cause the terrain to not
    // perfectly overlap at the clipmap level boundary.

    // 5 vertices are used per triangle to create a suitable triangle strip.
    // (This is somewhat redundant, but it simplifies the implementation).
    // Two different strips are needed for left/right and top/bottom.

    uint32_t degenerate_vertices = (2 * (CLIPMAP_SIZE - 1) + 1) * 5;
    vertex_count += degenerate_vertices * 2;

    // assert that we can store the indices in uint16_t with one space reserved
    // for primitive restart
    assert(vertex_count < UINT16_MAX - 1);

    size_t vertex_buffer_size = sizeof(GLubyte) * 2u * vertex_count;
    GLubyte *vertices = (GLubyte *) malloc(vertex_buffer_size);
    GLubyte *p_v = vertices;

    p_v = generate_vertices(quadlet, p_v);
    p_v = generate_vertices(quad, p_v);
    p_v = generate_vertices(fixup_z, p_v);
    p_v = generate_vertices(fixup_x, p_v);
    p_v = generate_vertices(trim_neg_z, p_v);
    p_v = generate_vertices(trim_pos_x, p_v);
    p_v = generate_vertices(trim_pos_z, p_v);
    p_v = generate_vertices(trim_neg_x, p_v);

    // degenerate triangles ----------------------------------------------------

    // for both left and right
    for (uint32_t z = 0; z < (CLIPMAP_SIZE - 1) * 2 + 1; z++) {
        *(p_v++) = 0;
        *(p_v++) = z * 2;
        *(p_v++) = 0;
        *(p_v++) = z * 2;
        *(p_v++) = 0;
        *(p_v++) = z * 2 + 1;
        *(p_v++) = 0;
        *(p_v++) = z * 2 + 2;
        *(p_v++) = 0;
        *(p_v++) = z * 2 + 2;
    }

    // for both top and bottom
    for (uint32_t x = 0; x < (CLIPMAP_SIZE - 1) * 2 + 1; x++) {
        *(p_v++) = x * 2;
        *(p_v++) = 0;
        *(p_v++) = x * 2;
        *(p_v++) = 0;
        *(p_v++) = x * 2 + 1;
        *(p_v++) = 0;
        *(p_v++) = x * 2 + 2;
        *(p_v++) = 0;
        *(p_v++) = x * 2 + 2;
        *(p_v++) = 0;
    }

    // assert we created exactly the same amount as expected
    assert(p_v - vertices == vertex_count * 2u);

    GL_CHECK(glGenBuffers(1, &mesh->vertex_buffer));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer));
    GL_CHECK(glBufferData(
            GL_ARRAY_BUFFER, vertex_buffer_size, vertices, GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));

    free(vertices);

    // create the index buffers for the triangle strips ------------------------

    mesh->quadlet.count = calculate_index_count(quadlet);
    mesh->quadlet.range = calculate_range(quadlet);
    mesh->quad.count = calculate_index_count(quad);
    mesh->quad.range = calculate_range(quad);

    mesh->fixup_z.count = calculate_index_count(fixup_z);
    mesh->fixup_z.range = calculate_range(fixup_z);
    mesh->fixup_x.count = calculate_index_count(fixup_x);
    mesh->fixup_x.range = calculate_range(fixup_x);

    uint32_t neg_z = calculate_index_count(trim_neg_z);
    nm::uvec2 neg_z_range = calculate_range(trim_neg_z);
    uint32_t pos_x = calculate_index_count(trim_pos_x);
    nm::uvec2 pos_x_range = calculate_range(trim_pos_x);
    uint32_t pos_z = calculate_index_count(trim_pos_z);
    nm::uvec2 pos_z_range = calculate_range(trim_pos_z);
    uint32_t neg_x = calculate_index_count(trim_neg_x);
    nm::uvec2 neg_x_range = calculate_range(trim_neg_x);
    mesh->trim_neg_z_neg_x.count = neg_z + neg_x;
    mesh->trim_neg_z_neg_x.range = nm::max(neg_z_range, neg_x_range);
    mesh->trim_pos_z_pos_x.count = pos_z + pos_x;
    mesh->trim_pos_z_pos_x.range = nm::max(pos_z_range, pos_x_range);
    mesh->trim_pos_z_neg_x.count = pos_z + neg_x;
    mesh->trim_pos_z_neg_x.range = nm::max(pos_z_range, neg_x_range);
    mesh->trim_neg_z_pos_x.count = neg_z + pos_x;
    mesh->trim_neg_z_pos_x.range = nm::max(neg_z_range, pos_x_range);

    // 6 indices are used here per vertex.
    // Need to repeat one vertex to get correct winding when
    // connecting the triangle strips.
    uint32_t degenerate_count = ((CLIPMAP_SIZE - 1) * 2 + 1) * 6;
    mesh->degenerate_neg_x.count = degenerate_count;
    mesh->degenerate_neg_x.range = nm::uvec2(0, CLIPMAP_LEVEL_SIZE - 1u);
    mesh->degenerate_pos_x.count = degenerate_count;
    mesh->degenerate_pos_x.range = nm::uvec2(0, CLIPMAP_LEVEL_SIZE - 1u);
    mesh->degenerate_neg_z.count = degenerate_count;
    mesh->degenerate_neg_z.range = nm::uvec2(CLIPMAP_LEVEL_SIZE - 1u, 0);
    mesh->degenerate_pos_z.count = degenerate_count;
    mesh->degenerate_pos_z.range = nm::uvec2(CLIPMAP_LEVEL_SIZE - 1u, 0);

    mesh->index_count =
            mesh->quadlet.count + mesh->quad.count +
            mesh->fixup_z.count + mesh->fixup_x.count +
            mesh->trim_neg_z_neg_x.count + mesh->trim_pos_z_pos_x.count +
            mesh->trim_pos_z_neg_x.count + mesh->trim_neg_z_pos_x.count +
            mesh->degenerate_neg_x.count + mesh->degenerate_pos_x.count +
            mesh->degenerate_neg_z.count + mesh->degenerate_pos_z.count;
    // largest index (#ind - 1) must be smaller than the max value
    // which is used for primitive restarting
    assert(mesh->index_count - 1 < UINT16_MAX);

    size_t index_buffer_size = sizeof(GLushort) * mesh->index_count;
    GLushort *indices = (GLushort *) malloc(index_buffer_size);
    GLushort *pi = indices;

    uint32_t vertex_buffer_offset = 0u;

    // 3x3 block
    mesh->quadlet.offset = pi - indices;
    pi = generate_indices(pi, quadlet, vertex_buffer_offset);
    vertex_buffer_offset += vertex_count_quadlet;

    // main block
    mesh->quad.offset = pi - indices;
    pi = generate_indices(pi, quad, vertex_buffer_offset);
    vertex_buffer_offset += vertex_count_block;

    // fixup in z dimension
    mesh->fixup_z.offset = pi - indices;
    pi = generate_indices(pi, fixup_z, vertex_buffer_offset);
    vertex_buffer_offset += 3 * CLIPMAP_SIZE;

    // fixup in x dimension
    mesh->fixup_x.offset = pi - indices;
    pi = generate_indices(pi, fixup_x, vertex_buffer_offset);
    vertex_buffer_offset += 3 * CLIPMAP_SIZE;

    // one of the trim regions will be used to connect level N with level N + 1.
    uint32_t trim_vertices = (2 * CLIPMAP_SIZE + 1) * 2;

    // +x,-z l-shaped interior trim
    mesh->trim_neg_z_pos_x.offset = pi - indices;
    // Top
    pi = generate_indices(pi, trim_neg_z, vertex_buffer_offset);
    // Right
    pi = generate_indices(
            pi, trim_pos_x, vertex_buffer_offset + (2 * CLIPMAP_SIZE + 1) * 2);
    vertex_buffer_offset += trim_vertices;

    // +x,+z l-shaped interior trim
    mesh->trim_pos_z_pos_x.offset = pi - indices;
    // Right
    pi = generate_indices(pi, trim_pos_x, vertex_buffer_offset);
    // Bottom
    pi = generate_indices(
            pi, trim_pos_z, vertex_buffer_offset + (2 * CLIPMAP_SIZE + 1) * 2);
    vertex_buffer_offset += trim_vertices;

    // -x,+z l-shaped interior trim
    mesh->trim_pos_z_neg_x.offset = pi - indices;
    // Bottom
    pi = generate_indices(pi, trim_pos_z, vertex_buffer_offset);
    // Left
    pi = generate_indices(
            pi, trim_neg_x, vertex_buffer_offset + (2 * CLIPMAP_SIZE + 1) * 2);
    vertex_buffer_offset += trim_vertices;

    // -x,-z l-shaped interior trim
    mesh->trim_neg_z_neg_x.offset = pi - indices;
    // Left
    pi = generate_indices(pi, trim_neg_x, vertex_buffer_offset);
    // Top
    pi = generate_indices(
            pi, trim_neg_z, vertex_buffer_offset - 6 * (2 * CLIPMAP_SIZE + 1));
    vertex_buffer_offset += trim_vertices;

    // degenerates
    // left and right share vertices (with different offsets in vertex shader)
    // top and bottom also share vertices

    // left
    mesh->degenerate_neg_x.offset = pi - indices;
    for (uint32_t z = 0; z < (CLIPMAP_SIZE - 1) * 2 + 1; z++) {
        *(pi++) = (5 * z) + 0 + vertex_buffer_offset;
        *(pi++) = (5 * z) + 1 + vertex_buffer_offset;
        *(pi++) = (5 * z) + 2 + vertex_buffer_offset;
        *(pi++) = (5 * z) + 3 + vertex_buffer_offset;
        *(pi++) = (5 * z) + 4 + vertex_buffer_offset;
        *(pi++) = (5 * z) + 4 + vertex_buffer_offset;
    }

    // right
    mesh->degenerate_pos_x.offset = pi - indices;
    uint32_t start_z = (CLIPMAP_SIZE - 1) * 2;
    for (uint32_t z = 0; z < (CLIPMAP_SIZE - 1) * 2 + 1; z++) {
        // windings are in reverse order on this side
        *(pi++) = (5 * (start_z - z)) + 4 + vertex_buffer_offset;
        *(pi++) = (5 * (start_z - z)) + 3 + vertex_buffer_offset;
        *(pi++) = (5 * (start_z - z)) + 2 + vertex_buffer_offset;
        *(pi++) = (5 * (start_z - z)) + 1 + vertex_buffer_offset;
        *(pi++) = (5 * (start_z - z)) + 0 + vertex_buffer_offset;
        *(pi++) = (5 * (start_z - z)) + 0 + vertex_buffer_offset;
    }

    vertex_buffer_offset += ((CLIPMAP_SIZE - 1) * 2 + 1) * 5;

    // top
    // note: swapped with bottom w.r.t. original implementation,
    // to fix windings of vertices.
    uint32_t start_x = (CLIPMAP_SIZE - 1) * 2;
    mesh->degenerate_neg_z.offset = pi - indices;
    for (uint32_t x = 0; x < (CLIPMAP_SIZE - 1) * 2 + 1; x++) {
        *(pi++) = (5 * (start_x - x)) + 4 + vertex_buffer_offset;
        *(pi++) = (5 * (start_x - x)) + 3 + vertex_buffer_offset;
        *(pi++) = (5 * (start_x - x)) + 2 + vertex_buffer_offset;
        *(pi++) = (5 * (start_x - x)) + 1 + vertex_buffer_offset;
        *(pi++) = (5 * (start_x - x)) + 0 + vertex_buffer_offset;
        *(pi++) = (5 * (start_x - x)) + 0 + vertex_buffer_offset;
    }

    // bottom
    mesh->degenerate_pos_z.offset = pi - indices;
    for (uint32_t x = 0; x < (CLIPMAP_SIZE - 1) * 2 + 1; x++) {
        // windings are in reverse order on this side
        *(pi++) = (5 * x) + 0 + vertex_buffer_offset;
        *(pi++) = (5 * x) + 1 + vertex_buffer_offset;
        *(pi++) = (5 * x) + 2 + vertex_buffer_offset;
        *(pi++) = (5 * x) + 3 + vertex_buffer_offset;
        *(pi++) = (5 * x) + 4 + vertex_buffer_offset;
        *(pi++) = (5 * x) + 4 + vertex_buffer_offset;
    }

    assert(pi - indices == mesh->index_count);

    GL_CHECK(glGenBuffers(1, &mesh->index_buffer));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer));
    GL_CHECK(glBufferData(
            GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices,
            GL_STATIC_DRAW));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    free(indices);
}

// Already defined in the shader.
#define LOCATION_VERTEX 0

static void setup_vertex_array(mesh *mesh)
{
    GL_CHECK(glGenVertexArrays(1, &mesh->vertex_array));
    GL_CHECK(glBindVertexArray(mesh->vertex_array));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer));

    // note: integer data
    GL_CHECK(glVertexAttribIPointer(
            LOCATION_VERTEX, 2, GL_UNSIGNED_BYTE, 0, 0));
    GL_CHECK(glEnableVertexAttribArray(LOCATION_VERTEX));

    GL_CHECK(glBindVertexArray(0));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    // Element array buffer state is part of the vertex array object, have to
    // unbind it after the vertex array.
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void init_mesh(mesh *mesh)
{
    setup_mesh(mesh);
    setup_vertex_array(mesh);

    // todo do this before rendering and disable after rendering?
    // GLushort is used, use its max value to restart the primitive
    GL_CHECK(glPrimitiveRestartIndex(UINT16_MAX));
}

void cleanup_mesh(mesh *mesh)
{
    GL_CHECK(glDeleteBuffers(1, &mesh->vertex_buffer));
    GL_CHECK(glDeleteBuffers(1, &mesh->index_buffer));
    GL_CHECK(glDeleteVertexArrays(1, &mesh->vertex_array));
}

void render_mesh(mesh *mesh, draw_info di)
{
    GL_CHECK(glBindVertexArray(mesh->vertex_array));

    GL_CHECK(glDrawElementsInstanced(
            GL_TRIANGLE_STRIP, di.index_count, GL_UNSIGNED_SHORT,
            reinterpret_cast<const GLvoid *>(
                    di.index_buffer_offset * sizeof(GLushort)),
            di.instance_count));

    GL_CHECK(glBindVertexArray(0));
}