#include "terrain.h"
#include "nmutil/io.h"

#include "app.h"
#include "stb_wrapper.h"
#include <filesystem>

nm_ret init(terrain* t)
{
    init(&t->geometry);

    if (init(&t->heightmap) != NM_SUCCESS) return NM_FAIL;

    nm_ret ret;

    // todo find a better system for representing resources

    // todo it should be enforced that these shaders are only initialized once
    //  doing so would need a check to make sure that they are independent of
    //  the terrain instance

    // todo should the above even be the case? since terrain is never
    //  instantiated more than once. should we enforce that somehow?

    /** shader text */

    nm::res_t default_vert_src, default_frag_src, debug_vert_src, debug_frag_src, norm_geom_src,
        norm_frag_src, water_vert_src, water_frag_src;

    const std::filesystem::path default_vert_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("shader/lod.vert");
    ret = nm::read_file(
        &default_vert_src.text, &default_vert_src.len, default_vert_path.u8string().c_str());
    if (ret != NM_SUCCESS) return NM_FAIL;
    const std::filesystem::path default_frag_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("shader/lod.frag");
    ret = nm::read_file(
        &default_frag_src.text, &default_frag_src.len, default_frag_path.u8string().c_str());
    if (ret != NM_SUCCESS) return NM_FAIL;
    const std::filesystem::path debug_vert_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("shader/lod_debug.vert");
    ret = nm::read_file(
        &debug_vert_src.text, &debug_vert_src.len, debug_vert_path.u8string().c_str());
    if (ret != NM_SUCCESS) return NM_FAIL;
    const std::filesystem::path debug_frag_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("shader/lod_debug.frag");
    ret = nm::read_file(
        &debug_frag_src.text, &debug_frag_src.len, debug_frag_path.u8string().c_str());
    if (ret != NM_SUCCESS) return NM_FAIL;
    const std::filesystem::path norm_geom_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("shader/lod_norm.geom");
    ret = nm::read_file(&norm_geom_src.text, &norm_geom_src.len, norm_geom_path.u8string().c_str());
    if (ret != NM_SUCCESS) return NM_FAIL;
    const std::filesystem::path norm_frag_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("shader/lod_norm.frag");
    ret = nm::read_file(&norm_frag_src.text, &norm_frag_src.len, norm_frag_path.u8string().c_str());
    if (ret != NM_SUCCESS) return NM_FAIL;
    const std::filesystem::path water_vert_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("shader/lod_water.vert");
    ret = nm::read_file(
        &water_vert_src.text, &water_vert_src.len, water_vert_path.u8string().c_str());
    if (ret != NM_SUCCESS) return NM_FAIL;
    const std::filesystem::path water_frag_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("shader/lod_water.frag");
    ret = nm::read_file(
        &water_frag_src.text, &water_frag_src.len, water_frag_path.u8string().c_str());
    if (ret != NM_SUCCESS) return NM_FAIL;

    /** shader */

    nm::shader default_vert_shader, default_frag_shader, debug_vert_shader, debug_frag_shader,
        norm_geom_shader, norm_frag_shader, water_vert_shader, water_frag_shader;

    ret = default_vert_shader.init(default_vert_src.text, default_vert_src.len, GL_VERTEX_SHADER);
    if (ret != NM_SUCCESS) {
        nm::log(nm::LOG_ERROR, "default vert shader failed\n");
        return NM_FAIL;
    }
    ret = default_frag_shader.init(default_frag_src.text, default_frag_src.len, GL_FRAGMENT_SHADER);
    if (ret != NM_SUCCESS) {
        nm::log(nm::LOG_ERROR, "default frag shader failed\n");
        return NM_FAIL;
    }
    ret = debug_vert_shader.init(debug_vert_src.text, debug_vert_src.len, GL_VERTEX_SHADER);
    if (ret != NM_SUCCESS) {
        nm::log(nm::LOG_ERROR, "debug vert shader failed\n");
        return NM_FAIL;
    }
    ret = debug_frag_shader.init(debug_frag_src.text, debug_frag_src.len, GL_FRAGMENT_SHADER);
    if (ret != NM_SUCCESS) {
        nm::log(nm::LOG_ERROR, "debug frag shader failed\n");
        return NM_FAIL;
    }
    ret = norm_geom_shader.init(norm_geom_src.text, norm_geom_src.len, GL_GEOMETRY_SHADER);
    if (ret != NM_SUCCESS) {
        nm::log(nm::LOG_ERROR, "norm geom shader failed\n");
        return NM_FAIL;
    }
    ret = norm_frag_shader.init(norm_frag_src.text, norm_frag_src.len, GL_FRAGMENT_SHADER);
    if (ret != NM_SUCCESS) {
        nm::log(nm::LOG_ERROR, "norm frag shader failed\n");
        return NM_FAIL;
    }
    ret = water_vert_shader.init(water_vert_src.text, water_vert_src.len, GL_VERTEX_SHADER);
    if (ret != NM_SUCCESS) {
        nm::log(nm::LOG_ERROR, "water vert shader failed\n");
        return NM_FAIL;
    }
    ret = water_frag_shader.init(water_frag_src.text, water_frag_src.len, GL_FRAGMENT_SHADER);
    if (ret != NM_SUCCESS) {
        nm::log(nm::LOG_ERROR, "water frag shader failed\n");
        return NM_FAIL;
    }

    free(water_frag_src.text);
    free(water_vert_src.text);
    free(norm_frag_src.text);
    free(norm_geom_src.text);
    free(debug_frag_src.text);
    free(debug_vert_src.text);
    free(default_frag_src.text);
    free(default_vert_src.text);

    /** programs */

    ret = t->default_program.init(&default_vert_shader, nullptr, &default_frag_shader, nullptr);
    if (ret != NM_SUCCESS) return NM_FAIL;
    ret = t->debug_program.init(&debug_vert_shader, nullptr, &debug_frag_shader, nullptr);
    if (ret != NM_SUCCESS) return NM_FAIL;
    ret =
        t->normal_program.init(&default_vert_shader, &norm_geom_shader, &norm_frag_shader, nullptr);
    if (ret != NM_SUCCESS) return NM_FAIL;
    ret = t->water_program.init(&water_vert_shader, nullptr, &water_frag_shader, nullptr);
    if (ret != NM_SUCCESS) return NM_FAIL;

    water_frag_shader.cleanup();
    water_vert_shader.cleanup();
    norm_frag_shader.cleanup();
    norm_geom_shader.cleanup();
    debug_frag_shader.cleanup();
    debug_vert_shader.cleanup();
    default_frag_shader.cleanup();
    default_vert_shader.cleanup();

    /** variables */

    // constant array of inverse level sizes
    // used to calculate blending factor in shader
    GLfloat* inv_level_sizes = (GLfloat*)malloc(sizeof(GLfloat) * CLIPMAP_LEVEL_COUNT);
    float inv_size           = 1.f / (CLIPMAP_SCALE * CLIPMAP_LEVEL_SIZE);
    for (uint32_t i = 0; i < CLIPMAP_LEVEL_COUNT; i++) {
        inv_level_sizes[i] = inv_size;
        inv_size *= .5f;
    }

    nm::shader_program* programs[] = {
        &t->default_program, &t->debug_program, &t->normal_program, &t->water_program};

    // set the uniform values for all programs that share these
    for (uint32_t i = 0u; i < sizeof(programs) / sizeof(nm::shader_program*); i++) {
        nm::shader_program* prog = programs[i];

        prog->use();

        // todo put binding point in variable/define
        prog->bind_uniform_block("uni_instance_data", 0);

        // todo put values in variable/define
        prog->set_int("uni_heightmap", 0);
        prog->set_int("grass_diff", 1);
        prog->set_int("grass_norm", 2);
        prog->set_int("cliff_diff", 3);
        prog->set_int("cliff_norm", 4);

        prog->set_float_array("uni_inv_lvl_size", inv_level_sizes, CLIPMAP_LEVEL_COUNT);

        /** set all defines */

        prog->set_uint("DEF_CLIPMAP_SIZE", CLIPMAP_SIZE);
        prog->set_uint("DEF_CLIPMAP_LEVEL_SIZE", CLIPMAP_LEVEL_SIZE);
        prog->set_uint("DEF_CLIPMAP_LEVEL_COUNT", CLIPMAP_LEVEL_COUNT);
        prog->set_float("DEF_CLIPMAP_SCALE", CLIPMAP_SCALE);
        prog->set_float("DEF_TEXTURE_SCALE", TEXTURE_SCALE);
        prog->set_float("DEF_TERRAIN_AMP", TERRAIN_AMP);
        prog->set_float("DEF_TERRAIN_SCA", TERRAIN_SCA);
        prog->set_float("DEF_TERRAIN_WATER_LVL", TERRAIN_WATER_LVL);

        prog->unuse();
    }

    // cleanup
    free(inv_level_sizes);

    /** misc */
    uint8_t* data;
    int32_t x, y, n;

    const std::filesystem::path grass_diff_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("tex/aerial_grass_rock_1k/diff.png");
    data = load_img(grass_diff_path.u8string().c_str(), &x, &y, &n, 3);
    t->grass_diff.init(GL_TEXTURE_2D);
    ret = t->grass_diff.load(data, x, y, 3);
    free_img(data);
    if (ret == NM_FAIL) return NM_FAIL;

    const std::filesystem::path grass_norm_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("tex/aerial_grass_rock_1k/norm.png");
    data = load_img(grass_norm_path.u8string().c_str(), &x, &y, &n, 3);
    t->grass_norm.init(GL_TEXTURE_2D);
    ret = t->grass_norm.load(data, x, y, 3);
    free_img(data);
    if (ret == NM_FAIL) return NM_FAIL;

    const std::filesystem::path cliff_diff_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("tex/rock_wall_02_1k/diff.png");
    data = load_img(cliff_diff_path.u8string().c_str(), &x, &y, &n, 3);
    t->cliff_diff.init(GL_TEXTURE_2D);
    ret = t->cliff_diff.load(data, x, y, 3);
    free_img(data);
    if (ret == NM_FAIL) return NM_FAIL;

    const std::filesystem::path cliff_norm_path =
        TERRAIN3_RESOURCE_DIR / std::filesystem::path("tex/rock_wall_02_1k/norm.png");
    data = load_img(cliff_norm_path.u8string().c_str(), &x, &y, &n, 3);
    t->cliff_norm.init(GL_TEXTURE_2D);
    ret = t->cliff_norm.load(data, x, y, 3);
    free_img(data);
    if (ret == NM_FAIL) return NM_FAIL;

    return NM_SUCCESS;
}

void update(terrain* t, nm::fvec3 target)
{
    nm::fvec2 camera_pos = nm::fvec2(target.x, target.z);

    // the clipmap moves along with the camera
    update_level_offsets(&t->geometry, camera_pos);

    // as we move around, the heightmap textures are updated incrementally,
    // allowing for an "endless" terrain.
    update(&t->heightmap, t->geometry.level_offsets);
}

void render(terrain* t, nm::shader_program* prog, nm::mat4 vp, nm::fvec3 target)
{
    prog->use();

    // could cache these locations, but we have multiple shaders
    prog->set_mat4("uni_view_proj", vp);
    prog->set_vec3("uni_camera_pos", target);

    // set level offsets
    prog->set_ivec2_array("uni_lvl_off", &t->geometry.level_offsets[0], CLIPMAP_LEVEL_COUNT);

    use_texture(&t->heightmap);
    t->grass_diff.use(GL_TEXTURE1);
    t->grass_norm.use(GL_TEXTURE2);
    t->cliff_diff.use(GL_TEXTURE3);
    t->cliff_norm.use(GL_TEXTURE4);

    render(&t->geometry);

    t->cliff_norm.unuse(GL_TEXTURE4);
    t->cliff_diff.unuse(GL_TEXTURE3);
    t->grass_norm.unuse(GL_TEXTURE2);
    t->grass_diff.unuse(GL_TEXTURE1);
    unuse_texture(&t->heightmap);

    prog->unuse();
}

void render(terrain* t, nm::mat4 vp, nm::fvec3 target, operation_draw draw_op, bool is_wireframe)
{
    // calculate frustum for culling
    construct_frustum(&t->geometry.frustum, vp);

    // create a list of draw calls using the frustum
    update_draw_list(&t->geometry);

    if (is_wireframe) {
        GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    } else {
        GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    switch (draw_op) {
    case DEFAULT:
        render(t, &t->default_program, vp, target);
        render(t, &t->water_program, vp, target);
        break;
    case DEBUG:
        render(t, &t->debug_program, vp, target);
        break;
    case NORMALS:
        // normal program renders only the normal vectors
        render(t, &t->default_program, vp, target);
        render(t, &t->normal_program, vp, target);
        break;
    default:
        break;
    }
}

void cleanup(terrain* t)
{
    t->cliff_norm.cleanup();
    t->cliff_diff.cleanup();
    t->grass_norm.cleanup();
    t->grass_diff.cleanup();
    cleanup(&t->geometry);
    cleanup(&t->heightmap);
    t->water_program.cleanup();
    t->normal_program.cleanup();
    t->debug_program.cleanup();
    t->default_program.cleanup();
}