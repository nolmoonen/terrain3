#include "geometry.h"
#include "nmutil/util.h"

void setup_uniform_buffer(geometry *g)
{
    GL_CHECK(glGenBuffers(1, &g->uniform_buffer));
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, g->uniform_buffer));

    // per level we draw at most 12 regular blocks, 4 fixups, 1 trim,
    // 4 degenerate strips. for level zero we may additionally draw a quadlet,
    // 4 fixups and 4 regular blocks (at most since frustum culling)
    // double the UBO size just in case we have very high levels for UBO buffer
    // alignment
    g->uniform_buffer_size =
            2 * ((12 + 4 + 1 + 4) * CLIPMAP_LEVEL_COUNT + 1 + 4 + 4) *
            sizeof(instance_data);
    GL_CHECK(glBufferData(
            GL_UNIFORM_BUFFER, g->uniform_buffer_size, NULL, GL_STREAM_DRAW));

    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void init(geometry *g)
{
    init_mesh(&g->mesh);
    setup_uniform_buffer(g);
}

void cleanup(geometry *g)
{
    GL_CHECK(glDeleteBuffers(1, &g->uniform_buffer));
    cleanup_mesh(&g->mesh);
}


static inline ivec2 idiv2(ivec2 n, ivec2 d)
{ return ivec2(idiv(n.x, d.x), idiv(n.y, d.y)); }

/// Snapping clipmap level to a grid.
/// The clipmap levels only move in steps of texture coordinates.
/// Computes (-x,-z)-most grid-space position for the levels.
static ivec2 get_offset_level(const vec2 &camera_pos, uint32_t level)
{
    // convert world-space position to grid space
    ivec2 scaled_pos(camera_pos / vec2(CLIPMAP_SCALE));

    // snap to grid of next level, such that it is always aligned
    const int32_t next_level_res = int32_t(1u << (level + 1));
    ivec2 snapped_pos = idiv2(scaled_pos, next_level_res) * next_level_res;

    // subtract one higher level block size from position, to go from the
    // 'center' of the higher level's 'hole', to the (-x,-z)-most point of it
    ivec2 pos = snapped_pos - int32_t((CLIPMAP_SIZE - 1u) << (level + 1));
    return pos;
}

void update_level_offsets(geometry *g, const vec2 &camera_pos)
{
    for (uint32_t i = 0; i < CLIPMAP_LEVEL_COUNT; i++) {
        g->level_offsets[i] = get_offset_level(camera_pos, i);
    }
}

/// Returns pointer to struct a number of offset bytes from the start of the
/// buffer.
template<typename T>
static inline T *buffer_offset(T *buffer, size_t offset)
{ return reinterpret_cast<T *>(reinterpret_cast<uint8_t *>(buffer) + offset); }

/// Returns true if a block with the given range, level, and offset
/// intersects the current frustum.
bool intersects_frustum(geometry *g, ivec2 offset, uvec2 range, uint32_t level)
{
    // grid-space level offset
    vec3 lvl_off = vec3(
            g->level_offsets[level].x, 0.f, g->level_offsets[level].y);

    // world-space mesh offset
    vec3 pos = (lvl_off + vec3(offset.x, 0.f, offset.y)) * CLIPMAP_SCALE;
    // world-space mesh extent
    vec3 extent =
            vec3(float(range.x), 0.f, float(range.y)) * float(1u << level) *
            CLIPMAP_SCALE;
    // noise will be in [0, <2]
    extent.y = TERRAIN_AMP * 2.f;

    // get two terrain points
    vec3 start = pos;
    vec3 end = pos + extent;

    // the aabb needs the actual minimum and maximum points
    aabb bb;
    bb.min = fminf(start, end);
    bb.max = fmaxf(start, end);

    // add some margin to deal with precision issues
    bb.min -= vec3(.02f);
    bb.max += vec3(.02f);

    return intersect(&g->frustum, &bb);
}

/// Sets the UBO offset of the passed-in draw info to the passed-in UBO
/// offset. Updates the passed-in UBO offset based on the draw info.
/// Returns a pointer to the next draw info.
draw_info *update_draw_list(
        geometry *g, draw_info *info, size_t *uniform_buffer_offset)
{
    info->uniform_buffer_offset = *uniform_buffer_offset;

    // have to ensure that the uniform buffer is always bound at aligned offsets
    *uniform_buffer_offset = realign_offset(
            *uniform_buffer_offset +
            info->instance_count * sizeof(instance_data),
            gl_ubo_alignment);

    return ++info;
}

/// The singular 3x3 quadlet.
draw_info get_draw_info_quadlet(geometry *g, instance_data *instances)
{
    draw_info info;
    info.instance_count = 0;
    info.index_buffer_offset = g->mesh.quadlet.offset;
    info.index_count = g->mesh.quadlet.count;

    instance_data instance;

    instance.level = 0;
    instance.offset = ivec2(2, 2) * (CLIPMAP_SIZE - 1);
    instance.id = 0;

    if (intersects_frustum(g, instance.offset, g->mesh.quadlet.range, 0)) {
        *instances++ = instance;
        info.instance_count++;
    }

    return info;
}

/// These are the basic MxM tesselated quads.
draw_info get_draw_info_quads(geometry *g, instance_data *instances)
{
    draw_info info;
    info.instance_count = 0;
    info.index_buffer_offset = g->mesh.quad.offset;
    info.index_count = g->mesh.quad.count;

    instance_data instance;
    instance.id = 1;

    // from level 1 and out, the four center blocks are already filled with the
    // lower clipmap level, so skip these.
    for (uint32_t i = 0; i < CLIPMAP_LEVEL_COUNT; i++) {
        for (uint32_t z = 0; z < 4; z++) {
            for (uint32_t x = 0; x < 4; x++) {
                if (i > 0 && z != 0 && z != 3 && x != 0 && x != 3) {
                    // already occupied, skip. (except for level 0)
                    continue;
                }

                instance.level = i;
                instance.offset = ivec2(x, z) * ((CLIPMAP_SIZE - 1) << i);

                // skip 2 texels horizontally and vertically at the middle to
                // get a symmetric structure. these regions are filled with
                // horizontal and vertical fixup regions
                if (x >= 2) instance.offset.x += 2 << i;
                if (z >= 2) instance.offset.y += 2 << i;

                if (intersects_frustum(g, instance.offset, g->mesh.quad.range,
                                       i)) {
                    *instances++ = instance;
                    info.instance_count++;
                }
            }
        }
    }

    return info;
}

draw_info get_draw_info_fixup_z(geometry *g, instance_data *instances)
{
    draw_info info;
    instance_data instance;

    // Vertical
    info.index_buffer_offset = g->mesh.fixup_z.offset;
    info.index_count = g->mesh.fixup_z.count;
    info.instance_count = 0;

    instance.level = 0;
    instance.id = 2;

    // +(CLIPMAP_SIZE - 1) offset in z from the -z one at level 0
    instance.offset = ivec2(2 * (CLIPMAP_SIZE - 1), (CLIPMAP_SIZE - 1));

    if (intersects_frustum(g, instance.offset, g->mesh.fixup_z.range, 0)) {
        *instances++ = instance;
        info.instance_count++;
    }

    // -(CLIPMAP_SIZE - 1) offset in z from the +z one at level 0
    instance.offset = ivec2(2 * (CLIPMAP_SIZE - 1), 2 * (CLIPMAP_SIZE - 1) + 2);

    if (intersects_frustum(g, instance.offset, g->mesh.fixup_z.range, 0)) {
        *instances++ = instance;
        info.instance_count++;
    }

    instance.id = 3;
    for (uint32_t i = 0; i < CLIPMAP_LEVEL_COUNT; i++) {
        // Top region
        instance.level = i;

        instance.offset = ivec2(2 * (CLIPMAP_SIZE - 1), 0) * (1 << i);

        if (intersects_frustum(g, instance.offset, g->mesh.fixup_z.range, i)) {
            *instances++ = instance;
            info.instance_count++;
        }

        // Bottom region
        instance.offset =
                ivec2(2 * (CLIPMAP_SIZE - 1), 3 * (CLIPMAP_SIZE - 1) + 2) *
                (1 << i);

        if (intersects_frustum(g, instance.offset, g->mesh.fixup_z.range, i)) {
            *instances++ = instance;
            info.instance_count++;
        }
    }

    return info;
}

draw_info get_draw_info_fixup_x(geometry *g, instance_data *instances)
{
    draw_info info;
    instance_data instance;

    // Horizontal
    info.index_buffer_offset = g->mesh.fixup_x.offset;
    info.index_count = g->mesh.fixup_x.count;
    info.instance_count = 0;

    // for the first level, we draw two more horizontal fixups
    instance.level = 0;
    instance.id = 2;

    // +(CLIPMAP_SIZE - 1) offset in x from the -x one at level 0
    instance.offset = ivec2(
            (CLIPMAP_SIZE - 1), 2 * (CLIPMAP_SIZE - 1));

    if (intersects_frustum(g, instance.offset, g->mesh.fixup_x.range, 0)) {
        *instances++ = instance;
        info.instance_count++;
    }

    // -(CLIPMAP_SIZE - 1) offset in x from the +x one at level 0
    instance.offset = ivec2(
            2 * (CLIPMAP_SIZE - 1) + 2, 2 * (CLIPMAP_SIZE - 1));

    if (intersects_frustum(g, instance.offset, g->mesh.fixup_x.range, 0)) {
        *instances++ = instance;
        info.instance_count++;
    }

    // for each level, follow the same process and draw two fixups
    instance.id = 3;
    for (uint32_t i = 0; i < CLIPMAP_LEVEL_COUNT; i++) {
        // Left side horizontal fixup region.
        // Texel coordinates are derived by just dividing the world space offset with texture size.
        // The 0.5 texel offset required to sample exactly at the texel center is done in vertex shader.
        instance.level = i;

        instance.offset = ivec2(0, 2 * (CLIPMAP_SIZE - 1)) * (1 << i);

        // only add the instance if it's visible
        if (intersects_frustum(g, instance.offset, g->mesh.fixup_x.range, i)) {
            *instances++ = instance;
            info.instance_count++;
        }

        // ight side horizontal fixup region
        instance.offset =
                ivec2(3 * (CLIPMAP_SIZE - 1) + 2, 2 * (CLIPMAP_SIZE - 1)) *
                (1 << i);

        // only add the instance if it's visible
        if (intersects_frustum(g, instance.offset, g->mesh.fixup_x.range, i)) {
            *instances++ = instance;
            info.instance_count++;
        }
    }

    return info;
}

draw_info get_draw_info_degenerate(
        geometry *g, instance_data *instances, const block &block,
        const ivec2 &offset, const ivec2 &ring_offset, int id)
{
    draw_info info;
    info.instance_count = 0;
    info.index_buffer_offset = block.offset;
    info.index_count = block.count;

    instance_data instance;
    instance.id = id;

    // no need to connect the last clipmap level to next level (there is none)
    for (uint32_t i = 0; i < CLIPMAP_LEVEL_COUNT - 1; i++) {
        instance.level = i;
        instance.offset = offset * (1 << i);

        // due to horizontal and vertical fixup region,
        // additional offset is (2 extra texels) is required.
        instance.offset += ring_offset * (1 << i);

        if (intersects_frustum(g, instance.offset, block.range, i)) {
            *instances++ = instance;
            info.instance_count++;
        }
    }

    return info;
}

draw_info get_draw_info_degenerate_neg_x(geometry *g, instance_data *instances)
{
    return get_draw_info_degenerate(
            g, instances, g->mesh.degenerate_neg_x, ivec2(0), ivec2(0), 4);
}

draw_info get_draw_info_degenerate_pos_x(geometry *g, instance_data *instances)
{
    return get_draw_info_degenerate(
            g, instances, g->mesh.degenerate_pos_x,
            ivec2(4 * (CLIPMAP_SIZE - 1), 0), ivec2(2, 0), 5);
}

draw_info get_draw_info_degenerate_neg_z(geometry *g, instance_data *instances)
{
    return get_draw_info_degenerate(
            g, instances, g->mesh.degenerate_neg_z, ivec2(0), ivec2(0), 6);
}

draw_info get_draw_info_degenerate_pos_z(geometry *g, instance_data *instances)
{
    return get_draw_info_degenerate(
            g, instances, g->mesh.degenerate_pos_z,
            ivec2(0, 4 * (CLIPMAP_SIZE - 1)), ivec2(0, 2), 7);
}

draw_info get_draw_info_trim(
        geometry *g, instance_data *instances, const block &block,
        trim_cond cond)
{
    draw_info info;
    info.index_buffer_offset = block.offset;
    info.index_count = block.count;
    info.instance_count = 0;

    instance_data instance;
    instance.id = 7;

    // from level 1 and out, we only need a single L-shaped trim region
    for (uint32_t i = 1; i < CLIPMAP_LEVEL_COUNT; i++) {
        ivec2 offset_prev_level = g->level_offsets[i - 1];
        ivec2 offset_current_level =
                g->level_offsets[i] + ((CLIPMAP_SIZE - 1) << i);

        // there are four different ways (top-right, bottom-right, top-left,
        // bottom-left) to apply a trim region depending on how camera snapping
        // is done in get_offset_level(). a function pointer is introduced here
        // so we can check if a particular trim type should be used for this
        // level. only one conditional will return true for a given level.

        ivec2 off =
                (offset_prev_level - offset_current_level) / int32_t(1u << i);
        if (!cond(off)) continue;

        instance.level = i;
        instance.offset = ivec2((CLIPMAP_SIZE - 1) << i);

        if (intersects_frustum(g, instance.offset, block.range, i)) {
            *instances++ = instance;
            info.instance_count++;
        }
    }

    return info;
}

// offset.x and offset.y are either 0 or 1 (width of trim)

static inline bool trim_cond_pos_x_neg_z(const ivec2 &offset)
{ return offset.x == 0 && offset.y == 1; }

static inline bool trim_cond_neg_x_neg_z(const ivec2 &offset)
{ return offset.x == 1 && offset.y == 1; }

static inline bool trim_cond_pos_x_pos_z(const ivec2 &offset)
{ return offset.x == 0 && offset.y == 0; }

static inline bool trim_cond_neg_x_pos_z(const ivec2 &offset)
{ return offset.x == 1 && offset.y == 0; }

draw_info get_draw_info_trim_pos_x_neg_z(geometry *g, instance_data *instances)
{
    return get_draw_info_trim(
            g, instances, g->mesh.trim_neg_z_pos_x, trim_cond_pos_x_neg_z);
}

draw_info get_draw_info_trim_neg_x_neg_z(geometry *g, instance_data *instances)
{
    return get_draw_info_trim(
            g, instances, g->mesh.trim_neg_z_neg_x, trim_cond_neg_x_neg_z);
}

draw_info get_draw_info_trim_pos_x_pos_z(geometry *g, instance_data *instances)
{
    return get_draw_info_trim(
            g, instances, g->mesh.trim_pos_z_pos_x, trim_cond_pos_x_pos_z);
}

draw_info get_draw_info_trim_neg_x_pos_z(geometry *g, instance_data *instances)
{
    return get_draw_info_trim(
            g, instances, g->mesh.trim_pos_z_neg_x, trim_cond_neg_x_pos_z);
}

void update_draw_list(geometry *g)
{
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, g->uniform_buffer));
    instance_data *data = (instance_data *) glMapBufferRange(
            GL_UNIFORM_BUFFER, 0, g->uniform_buffer_size,
            GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT);
    GL_CHECK_ERRORS();

    if (!data) {
        nmutil::log(nmutil::LOG_ERROR, "failed to map uniform buffer\n");
        return;
    }

    // byte offset, multiples of uniform_buffer_align
    size_t uniform_buffer_offset = 0;

    // create a draw list. the number of draw calls is equal to the different
    // types of blocks. the blocks are instanced as necessary in the
    // get_draw_info* calls.

    draw_info *info = g->draw_infos;

    // 3x3 block
    *info = get_draw_info_quadlet(g,
                                  buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // main blocks
    *info = get_draw_info_quads(g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // z direction ring fixups
    *info = get_draw_info_fixup_z(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // x direction ring fixups
    *info = get_draw_info_fixup_x(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // -x degenerates
    *info = get_draw_info_degenerate_neg_x(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // +x degenerates
    *info = get_draw_info_degenerate_pos_x(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // -z degenerates
    *info = get_draw_info_degenerate_neg_z(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // +z degenerates
    *info = get_draw_info_degenerate_pos_z(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // +x,-z trim
    *info = get_draw_info_trim_pos_x_neg_z(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // -x,-z trim
    *info = get_draw_info_trim_neg_x_neg_z(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // +x,+z trim
    *info = get_draw_info_trim_pos_x_pos_z(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    // -x,+z trim
    *info = get_draw_info_trim_neg_x_pos_z(
            g, buffer_offset(data, uniform_buffer_offset));
    info = update_draw_list(g, info, &uniform_buffer_offset);

    GL_CHECK(glUnmapBuffer(GL_UNIFORM_BUFFER));
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void render(geometry *g)
{
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, g->uniform_buffer));
    for (uint32_t i = 0; i < BLOCK_COUNT; i++) {
        draw_info di = g->draw_infos[i];
        if (di.instance_count == 0) continue;

        // todo put index in define
        // bind uniform buffer at correct offset
        GL_CHECK(glBindBufferRange(
                GL_UNIFORM_BUFFER, 0, g->uniform_buffer,
                di.uniform_buffer_offset, realign_offset(
                        di.instance_count * sizeof(instance_data),
                        gl_ubo_alignment)));

        // draw all instances
        render_mesh(&g->mesh, di);
    }
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0));
}