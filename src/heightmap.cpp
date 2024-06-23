#include <cstring>
#include "heightmap.h"

#define _USE_MATH_DEFINES

#include "math.h"
#include "noise.h"
#include "nmutil/io.h"
#include "nmutil/util.h"
#include <filesystem>
#include "app.h"

/// The compute shader program.
nm::shader_program comp_program;

nm_ret init(heightmap *hm)
{
    // create texture that represents the heightmap
    hm->texture.init(GL_TEXTURE_2D_ARRAY);
    hm->texture.use();

    // R is height, G and B are terrain gradients, A is padding
    GL_CHECK(glTexStorage3D(
            GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F,
            CLIPMAP_LEVEL_SIZE, CLIPMAP_LEVEL_SIZE, CLIPMAP_LEVEL_COUNT));

    GL_CHECK(glTexParameteri(
            GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(
            GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST));

    // the repeat is crucial here. this allows us to update small sections of
    // the texture when moving the camera
    GL_CHECK(glTexParameteri(
            GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(
            GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT));

    hm->texture.unuse();

    // uniform buffer for compute shader
    GL_CHECK(glGenBuffers(1, &hm->uniform_buffer));
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, hm->uniform_buffer));

    // allocate space
    hm->uniform_buffer_size = sizeof(update_info) * MAX_UPDATE_COUNT;
    GL_CHECK(glBufferData(
            GL_UNIFORM_BUFFER, hm->uniform_buffer_size, NULL, GL_STREAM_DRAW));

    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));

    // compute shader
    nm::res_t comp_src;
    const std::filesystem::path comp_path = TERRAIN3_RESOURCE_DIR / std::filesystem::path("shader/lod.comp");
    nm_ret ret = nm::read_file(&comp_src.text, &comp_src.len, comp_path.u8string().c_str());
    if (ret != NM_SUCCESS) return NM_FAIL;

    nm::shader comp_shader;
    ret = comp_shader.init(
            comp_src.text, comp_src.len, GL_COMPUTE_SHADER);
    if (ret != NM_SUCCESS) {
        nm::log(nm::LOG_ERROR, "compute shader failed\n");
        return NM_FAIL;
    }

    free(comp_src.text);
    ret = comp_program.init(nullptr, nullptr, nullptr, &comp_shader);
    if (ret != NM_SUCCESS) return NM_FAIL;

    comp_shader.cleanup();

    comp_program.use();
    // todo put binding point in variable/define
    comp_program.bind_uniform_block("uni_data", 0);

    comp_program.set_int("uni_noise", 1);
    comp_program.set_uint("DEF_CLIPMAP_LEVEL_SIZE", CLIPMAP_LEVEL_SIZE);
    comp_program.set_float("DEF_CLIPMAP_SCALE", CLIPMAP_SCALE);
    comp_program.set_float("DEF_TERRAIN_AMP", TERRAIN_AMP);
    comp_program.set_float("DEF_TERRAIN_SCA", TERRAIN_SCA);
    comp_program.set_uint("DEF_NOISE_SIZE", NOISE_SIZE);
    comp_program.unuse();

    // initialize noise
    srand(2);
    uint32_t noise_count = NOISE_SIZE * NOISE_SIZE;
    hm->noise = (uint8_t *) malloc(sizeof(uint8_t) * noise_count);

    for (uint32_t i = 0u; i < noise_count; i++) {
        hm->noise[i] = uint8_t(float(rand()) / float(RAND_MAX) * UINT8_MAX);
    }

    // state: initialize level infos
    for (uint32_t i = 0; i < CLIPMAP_LEVEL_COUNT; i++) {
        hm->level_infos[i].cleared = true;
    }

    // create noise texture
    hm->noise_tex.init(GL_TEXTURE_2D);
    hm->noise_tex.use();

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    GL_CHECK(glTexImage2D(
            GL_TEXTURE_2D, 0, GL_R32F, NOISE_SIZE, NOISE_SIZE, 0, GL_RED,
            GL_UNSIGNED_BYTE, hm->noise));

    GL_CHECK(glBindImageTexture(
            1, hm->noise_tex.id, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F));

    hm->noise_tex.unuse();

    return NM_SUCCESS;
}

void cleanup(heightmap *hm)
{
    GL_CHECK(glDeleteBuffers(1, &hm->uniform_buffer));
    comp_program.cleanup(); // todo only do if not already done
    free(hm->noise);
    hm->texture.cleanup();
}

nm::fvec3 get_height(heightmap *hm, nm::fvec2 pos)
{
    // returns in [0,1]
    nm::fvec3 t = terrain_noise(TERRAIN_SCA * pos, hm->noise, NOISE_SIZE);
    float height = TERRAIN_AMP * t.x;
    nm::fvec2 grad = TERRAIN_AMP * TERRAIN_SCA * nm::fvec2(t.y, t.z);

    return nm::fvec3(height, grad.x, grad.y);
}

/// Register that the following region must be updated:
/// At texture position [tex_x,tex_y] compute a block of [size_x,size_y]
/// that starts in (world) texture space [start_x,start_y].
void register_update_region(
        update_info *infos, uint32_t *info_index,
        int32_t tex_x, int32_t tex_y, int32_t size_x, int32_t size_y,
        int32_t start_x, int32_t start_y, int32_t level)
{
    // do not create update info when nothing needs updating
    if (size_x == 0 || size_y == 0) return;

    update_info info;
    info.tex = nm::ivec2(tex_x, tex_y);
    info.size = nm::ivec2(size_x, size_y);
    info.start = nm::ivec2(start_x, start_y);
    info.level = level;

    infos[(*info_index)++] = info;
}

/// Find out what parts of this level's texture need to be updated.
/// Changes array of update info structs and index to next.
void update_level(
        heightmap *hm, nm::ivec2 offset, uint32_t level, update_info *u_infos,
        uint32_t *info_index)
{
    level_info *info = &hm->level_infos[level];
    // (-x,-z)-most world texture coordinate
    int32_t start_x = offset.x >> level;
    int32_t start_y = offset.y >> level;

    // nothing to do for this level, not moved and not cleared
    if (start_x == info->x && start_y == info->y && !info->cleared) return;

    // movement in texture coordinates
    int32_t delta_x = start_x - info->x;
    int32_t delta_y = start_y - info->y;

    // old (local) texture origin
    int32_t old_base_x = nm::idiv(info->x, CLIPMAP_LEVEL_SIZE) * CLIPMAP_LEVEL_SIZE;
    int32_t old_base_y = nm::idiv(info->y, CLIPMAP_LEVEL_SIZE) * CLIPMAP_LEVEL_SIZE;
    // new (local) texture origin
    int32_t base_x = nm::idiv(start_x, CLIPMAP_LEVEL_SIZE) * CLIPMAP_LEVEL_SIZE;
    int32_t base_y = nm::idiv(start_y, CLIPMAP_LEVEL_SIZE) * CLIPMAP_LEVEL_SIZE;

    // check if we must compute the complete texture or just parts of it
    if (abs(delta_x) >= int32_t(CLIPMAP_LEVEL_SIZE) ||
        abs(delta_y) >= int32_t(CLIPMAP_LEVEL_SIZE) || info->cleared) {
        // we have suddenly moved to a completely different place in the
        // heightmap, or we need to recompute everything

        //        /--------------------------|seam|-------------------------/
        // world: wrapped  start+CLIP_LVL_SIZE    start    base+CLIP_LVL_SIZE
        // local: 0        wrapped                wrapped  CLIP_LVL_SIZE
        // (base and start might be the same)

        int32_t wrapped_x = start_x - base_x;
        int32_t wrapped_y = start_y - base_y;

        register_update_region(
                u_infos, info_index, 0, 0, wrapped_x, wrapped_y,
                base_x + CLIPMAP_LEVEL_SIZE, base_y + CLIPMAP_LEVEL_SIZE,
                level);

        register_update_region(
                u_infos, info_index, wrapped_x, 0,
                CLIPMAP_LEVEL_SIZE - wrapped_x, wrapped_y, start_x,
                base_y + CLIPMAP_LEVEL_SIZE, level);

        register_update_region(
                u_infos, info_index, 0, wrapped_y, wrapped_x,
                CLIPMAP_LEVEL_SIZE - wrapped_y, base_x + CLIPMAP_LEVEL_SIZE,
                start_y, level);

        register_update_region(
                u_infos, info_index, wrapped_x, wrapped_y,
                CLIPMAP_LEVEL_SIZE - wrapped_x, CLIPMAP_LEVEL_SIZE - wrapped_y,
                start_x, start_y, level);

        info->cleared = false;
    } else {
        // incremental update: upload what we need
        int32_t old_wrapped_x = info->x - old_base_x;
        int32_t old_wrapped_y = info->y - old_base_y;
        int32_t wrapped_x = start_x - base_x;
        int32_t wrapped_y = start_y - base_y;

        int32_t wrap_delta_x = wrapped_x - old_wrapped_x;
        int32_t wrap_delta_y = wrapped_y - old_wrapped_y;

        // there can be significant overlap between X-delta and Y-delta uploads
        // if deltas are large. Avoiding this would add even more complexities
        // and is therefore ignored here.

        // Do this in two steps:

        // First, update as we're moving in X, then move in Y.
        if (wrap_delta_x >= 0 && delta_x >= 0) {
            // One update region for X, simple case. Have to update both Y regions however.
            register_update_region(
                    u_infos, info_index, old_wrapped_x, 0, wrap_delta_x,
                    old_wrapped_y, info->x + CLIPMAP_LEVEL_SIZE,
                    old_base_y + CLIPMAP_LEVEL_SIZE, level);

            register_update_region(
                    u_infos, info_index, old_wrapped_x, old_wrapped_y,
                    wrap_delta_x, CLIPMAP_LEVEL_SIZE - old_wrapped_y,
                    info->x + CLIPMAP_LEVEL_SIZE, info->y, level);
        } else if (wrap_delta_x < 0 && delta_x < 0) {
            // One update region for X, simple case. Have to update both Y regions however.
            register_update_region(
                    u_infos, info_index, wrapped_x, 0, -wrap_delta_x,
                    old_wrapped_y, start_x, old_base_y + CLIPMAP_LEVEL_SIZE,
                    level);

            register_update_region(
                    u_infos, info_index, wrapped_x, old_wrapped_y,
                    -wrap_delta_x, CLIPMAP_LEVEL_SIZE - old_wrapped_y, start_x,
                    info->y, level);
        } else if (wrap_delta_x < 0 && delta_x >= 0) {
            // Two update regions in X, and also have to update both Y regions.
            register_update_region(
                    u_infos, info_index, 0, 0, wrapped_x, old_wrapped_y,
                    base_x + CLIPMAP_LEVEL_SIZE,
                    old_base_y + CLIPMAP_LEVEL_SIZE, level);

            register_update_region(
                    u_infos, info_index, old_wrapped_x, 0,
                    CLIPMAP_LEVEL_SIZE - old_wrapped_x, old_wrapped_y,
                    base_x + old_wrapped_x, old_base_y + CLIPMAP_LEVEL_SIZE,
                    level);

            register_update_region(
                    u_infos, info_index, 0, old_wrapped_y, wrapped_x,
                    CLIPMAP_LEVEL_SIZE - old_wrapped_y,
                    base_x + CLIPMAP_LEVEL_SIZE, info->y, level);

            register_update_region(
                    u_infos, info_index, old_wrapped_x, old_wrapped_y,
                    CLIPMAP_LEVEL_SIZE - old_wrapped_x,
                    CLIPMAP_LEVEL_SIZE - old_wrapped_y, base_x + old_wrapped_x,
                    info->y, level);
        } else if (wrap_delta_x >= 0 && delta_x < 0) {
            // Two update regions in X, and also have to update both Y regions.
            register_update_region(
                    u_infos, info_index, 0, 0, old_wrapped_x, old_wrapped_y,
                    base_x + CLIPMAP_LEVEL_SIZE,
                    old_base_y + CLIPMAP_LEVEL_SIZE, level);

            register_update_region(
                    u_infos, info_index, wrapped_x, 0,
                    CLIPMAP_LEVEL_SIZE - wrapped_x, old_wrapped_y, start_x,
                    old_base_y + CLIPMAP_LEVEL_SIZE, level);

            register_update_region(
                    u_infos, info_index, 0, old_wrapped_y, old_wrapped_x,
                    CLIPMAP_LEVEL_SIZE - old_wrapped_y,
                    base_x + CLIPMAP_LEVEL_SIZE, info->y, level);

            register_update_region(
                    u_infos, info_index, wrapped_x, old_wrapped_y,
                    CLIPMAP_LEVEL_SIZE - wrapped_x,
                    CLIPMAP_LEVEL_SIZE - old_wrapped_y, start_x, info->y,
                    level);
        }

        // Second, update as we're moving in Y.
        if (wrap_delta_y >= 0 && delta_y >= 0) {
            register_update_region(
                    u_infos, info_index, 0, old_wrapped_y, wrapped_x,
                    wrap_delta_y, base_x + CLIPMAP_LEVEL_SIZE,
                    info->y + CLIPMAP_LEVEL_SIZE, level);

            register_update_region(
                    u_infos, info_index, wrapped_x, old_wrapped_y,
                    CLIPMAP_LEVEL_SIZE - wrapped_x, wrap_delta_y, start_x,
                    info->y + CLIPMAP_LEVEL_SIZE, level);
        } else if (wrap_delta_y < 0 && delta_y < 0) {
            register_update_region(
                    u_infos, info_index, 0, wrapped_y, wrapped_x, -wrap_delta_y,
                    base_x + CLIPMAP_LEVEL_SIZE, start_y, level);

            register_update_region(
                    u_infos, info_index, wrapped_x, wrapped_y,
                    CLIPMAP_LEVEL_SIZE - wrapped_x, -wrap_delta_y, start_x,
                    start_y, level);
        } else if (wrap_delta_y < 0 && delta_y >= 0) {
            register_update_region(
                    u_infos, info_index, 0, 0, wrapped_x, wrapped_y,
                    base_x + CLIPMAP_LEVEL_SIZE, base_y + CLIPMAP_LEVEL_SIZE,
                    level);

            register_update_region(
                    u_infos, info_index, 0, old_wrapped_y, wrapped_x,
                    CLIPMAP_LEVEL_SIZE - old_wrapped_y,
                    base_x + CLIPMAP_LEVEL_SIZE, base_y + old_wrapped_y, level);

            register_update_region(
                    u_infos, info_index, wrapped_x, 0,
                    CLIPMAP_LEVEL_SIZE - wrapped_x, wrapped_y, start_x,
                    base_y + CLIPMAP_LEVEL_SIZE, level);

            register_update_region(
                    u_infos, info_index, wrapped_x, old_wrapped_y,
                    CLIPMAP_LEVEL_SIZE - wrapped_x,
                    CLIPMAP_LEVEL_SIZE - old_wrapped_y, start_x,
                    base_y + old_wrapped_y, level);
        } else if (wrap_delta_y >= 0 && delta_y < 0) {
            register_update_region(
                    u_infos, info_index, 0, 0, wrapped_x, old_wrapped_y,
                    base_x + CLIPMAP_LEVEL_SIZE, base_y + CLIPMAP_LEVEL_SIZE,
                    level);

            register_update_region(
                    u_infos, info_index, 0, wrapped_y, wrapped_x,
                    CLIPMAP_LEVEL_SIZE - wrapped_y, base_x + CLIPMAP_LEVEL_SIZE,
                    start_y, level);

            register_update_region(
                    u_infos, info_index, wrapped_x, 0,
                    CLIPMAP_LEVEL_SIZE - wrapped_x, old_wrapped_y, start_x,
                    base_y + CLIPMAP_LEVEL_SIZE, level);

            register_update_region(
                    u_infos, info_index, wrapped_x, wrapped_y,
                    CLIPMAP_LEVEL_SIZE - wrapped_x,
                    CLIPMAP_LEVEL_SIZE - wrapped_y, start_x, start_y, level);
        }
    }

    info->x = start_x;
    info->y = start_y;
}

void update(heightmap *hm, nm::ivec2 level_offsets[CLIPMAP_LEVEL_COUNT])
{
    // map buffer to gpu
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, hm->uniform_buffer));
    update_info *info = (update_info *) glMapBufferRange(
            GL_UNIFORM_BUFFER, 0, hm->uniform_buffer_size,
            GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT);
    GL_CHECK_ERRORS();

    // find out what needs to be updated for each level, set in buffer
    uint32_t update_region_count = 0;
    for (uint32_t i = 0; i < CLIPMAP_LEVEL_COUNT; i++) {
        update_level(hm, level_offsets[i], i, info, &update_region_count);
    }

    GL_CHECK(glUnmapBuffer(GL_UNIFORM_BUFFER));

    comp_program.use();
    GL_CHECK(glBindImageTexture(
            0, hm->texture.id, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA32F));

    GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, hm->uniform_buffer));

    hm->noise_tex.use(GL_TEXTURE1);

    // ability to compute at most all possible update buffers in a single call
    GL_CHECK(glDispatchCompute(
            CLIPMAP_LEVEL_SIZE, CLIPMAP_LEVEL_SIZE, update_region_count));

    hm->noise_tex.unuse(GL_TEXTURE1);

    comp_program.unuse();
    GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void use_texture(heightmap *hm)
{
    // make sure sync happens and that the compute shader is done
    GL_CHECK(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));

    hm->texture.use(GL_TEXTURE0);
}

void unuse_texture(heightmap *hm)
{
    hm->texture.unuse(GL_TEXTURE0);
}