#include "terrain.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "terrain.h"
#include "comm.h"
#include "timer.h"
#include "gui.h"
#include "axis.h"
#include "stb_wrapper.h"
#include "app.h"
#include "window.hpp"

#include <nmutil/log.h>
#include <nmutil/matrix.h>
#include "nmutil/gl.h"
#include "nmutil/camera.h"
#include "nmutil/io.h"

#include <filesystem>
#include <cstdint>
#include <chrono>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const char *APP_TITLE = "terrain3";

enum operation_edit {
    TRANSLATING,
    ROTATING,
    NONE
};

camera camera{};
static bool is_demo;
static bool is_debug;
static bool is_wireframe;
static operation_draw curr_draw_op = DEFAULT;
static operation_edit curr_edit_op = NONE;

// todo is this still needed with mouse_delta?
// can be negative
int32_t last_mouse_x;
int32_t last_mouse_y;

const std::filesystem::path TERRAIN3_RESOURCE_DIR = RESOURCE_DIR;

void update_state(window *);

/// Time delta in seconds.
void update_camera_pos(float dt, terrain *terrain);

/// Application entry point.
nm_ret run()
{
    nm::set_log_level(nm::LOG_TRACE);

    nm_ret ret;
    window *window;
    ret = init(&window, SCREEN_WIDTH, SCREEN_HEIGHT, APP_TITLE);
    if (ret != NM_SUCCESS) return ret;

    // do one-time initialization
    GL_CHECK(glEnable(GL_DEPTH_TEST));
    GL_CHECK(glEnable(GL_CULL_FACE));
    GL_CHECK(glEnable(GL_PRIMITIVE_RESTART));
    // todo replace with skybox
    GL_CHECK(glClearColor(.5f, .6f, .7f, 1.f));
    GL_CHECK(glEnable(GL_MULTISAMPLE));

    gui_init(window);

    nm::uvec2 frame_size = framebuffer_size(window);
    float aspect = float(frame_size.x) / float(frame_size.y);
    camera.init(aspect, nm::to_rad(70.f), 1.f, 1e5f);

    // initialize axes to draw them later
    if (init_axis() != NM_SUCCESS) return -1;

    // create terrain
    terrain terrain;
    if (init(&terrain) == NM_FAIL) return -1;

    std::chrono::duration<double> update_time(0.0);
    std::chrono::duration<double> render_time(0.0);
    std::chrono::time_point<std::chrono::steady_clock> t0, t1;

    // high-resolution timer to govern when to update with constant timestep
    timer update_timer;
    timer_start(&update_timer);

    // in seconds
    const float dt = 1.f / 60.f;
    // to cause an immediate update before rendering
    float time = dt;
    while (true) {
        poll_events();
        if (is_should_close(window)) break;

        nm::uvec2 size = framebuffer_size(window);
        camera.set_aspect((float) size.x / (float) size.y);

        // don't update or render if the app is not active
        if (!is_active(window)) continue;

        t0 = std::chrono::steady_clock::now();
        update_state(window);

        // update time with actual time
        time += timer_lap(&update_timer);
        // check if it is time to perform a fixed time step
        while (time >= dt) {
            time -= dt;

            // update camera position, if demoing
            if (is_demo) update_camera_pos(dt, &terrain);
        }

        // update terrain with (potentionally) new camera pos
        update(&terrain, camera.target);
        t1 = std::chrono::steady_clock::now();
        update_time += t1 - t0;

        t0 = std::chrono::steady_clock::now();
        GL_CHECK(glViewport(0, 0, (GLsizei) size.x, (GLsizei) size.y));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        nm::mat4 v = camera.get_view_matrix();
        nm::mat4 p = camera.get_proj_matrix();

        nm::mat4 vp = p * v;

        if (is_debug) {
            // origin
            nm::mat4 m = nm::mat4::scaling(10.f);
            render_axis(vp * m);
            // camera target
            m = nm::mat4::translation(camera.target);
            render_axis(vp * m);
        }

        render(&terrain, vp, camera.target, curr_draw_op, is_wireframe);

        if (is_debug) {
            display_stats(update_time, render_time);
            display_pos(camera.target);
        }

        t1 = std::chrono::steady_clock::now();
        render_time += t1 - t0;

        // switch the front and back buffers to display the updated scene
        swap_buffers(window);
        reset_events(window);
    }

    cleanup(&terrain);
    cleanup_axis();
    gui_cleanup();
    cleanup(window);

    return 0;
}

void update_state(window *w)
{
    if (was_esc_pressed(w)) {
        set_should_close(w);
    }

    // determine drawing/debug state -------------------------------------------
    if (was_f1_pressed(w)) {
        is_wireframe = !is_wireframe;
    }

    if (was_f2_pressed(w)) {
        if (curr_draw_op == DEBUG) {
            curr_draw_op = DEFAULT;
        } else {
            curr_draw_op = DEBUG;
        }
    }

    if (was_f3_pressed(w)) {
        if (curr_draw_op == NORMALS) {
            curr_draw_op = DEFAULT;
        } else {
            curr_draw_op = NORMALS;
        }
    }

    if (was_f4_pressed(w)) {
        is_debug = !is_debug;
    }

    if (was_enter_pressed(w)) {
        if (!is_demo) {
            // enter demo mode, reset camera
            camera.target = nm::fvec3(0.f);
            // look at positive x, slightly downwards
            camera.angles = nm::fvec3(
                    -nm::pi4 * .5f,
                    -nm::pi2,
                    0.f);
            camera.zoom_level = 20.f;
        }

        is_demo = !is_demo;
    }

    if (was_prtsc_pressed(w)) {
        nm::uvec2 size = framebuffer_size(w);

        uint8_t *data = (uint8_t *) malloc(4u * size.x * size.y);

        GL_CHECK(glReadPixels(
                0, 0, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, data));

        write_png("prtsc.png", size.x, size.y, 4, data);

        free(data);
    }

    // determine operation state -----------------------------------------------
    if (is_mmb_pressed(w)) {
        if (is_shift_pressed(w)) {
            curr_edit_op = TRANSLATING;
        } else {
            curr_edit_op = ROTATING;
        }
    } else {
        if (curr_edit_op == ROTATING || curr_edit_op == TRANSLATING) {
            curr_edit_op = NONE;
        }
    }

    // perform operation if needed ---------------------------------------------
    nm::dvec2 mouse = mouse_pos(w);
    float d_x = (float) (mouse.x - last_mouse_x);
    float d_y = (float) (mouse.y - last_mouse_y);
    if (d_x != 0.f || d_y != 0.f) {
        switch (curr_edit_op) {
            case TRANSLATING:
                camera.translate(d_x, d_y);
                break;
            case ROTATING:
                camera.rotate(d_x, d_y);
                break;
            case NONE:
            default:
                break;
        }
        last_mouse_x = mouse.x;
        last_mouse_y = mouse.y;
    }

    nm::dvec2 scroll_delta = scroll_pos_delta(w);
    camera.add_zoom(float(scroll_delta.y));
}

void update_camera_pos(float dt, terrain *terrain)
{
    // required to ensure only updating with a fixed timestep
    static float time = 0.f;
    time += dt;
    // speed of the camera movement
    float speed = 200.f;
    // size of the movement interpolation grid
    float scale = 200.f;
    // map time to x position
    float x = (speed * time) / scale;
    // find the four control points
    // v0 --- v1 -x- v2 --- v3
    float it = floorf(x); // align with grid
    float ft = nm::fractf(x); // value used for interpolation between controls
    // mult by scale to get world-space position of control points
    float x1 = scale * (it);
    float x2 = scale * (it + 1.f);

    // get heights at these positions.
    float water_height = TERRAIN_WATER_LVL * TERRAIN_AMP;
    float y1 = fmaxf(get_height(&terrain->heightmap, nm::fvec2(
            x1, camera.target.z)).x, water_height);
    float y2 = fmaxf(get_height(&terrain->heightmap, nm::fvec2(
            x2, camera.target.z)).x, water_height);

    // bezier control points in x,y-plane, evenly spaced in x dimension
    nm::fvec2 b0(x1, y1);
    nm::fvec2 b1(x1 + .33f * scale, y1);
    nm::fvec2 b2(x2 - .33f * scale, y2);
    nm::fvec2 b3(x2, y2);

    // evaluate cubic bezier curve
    float ti = 1.f - ft;
    nm::fvec2 b =
        ti * ti * ti * b0 + 3.f * ti * ti * ft * b1 +
        3.f * ti * ft * ft * b2 + ft * ft * ft * b3;

    // a few units above the terrain, bezier curve may go below terrain
    float height_offset = .2f * TERRAIN_AMP;
    camera.target.x = b.x;
    camera.target.y = b.y + height_offset;
}