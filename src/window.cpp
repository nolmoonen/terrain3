#include "window.hpp"

#include <nmutil/gl.h>
#include <nmutil/log.h>
#include <nmutil/vector.h>

// clang-format off
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// clang-format on

#include <cstdlib>

// todo window iconify callback?
// todo glfw check macro

constexpr size_t glfw_key_count          = GLFW_KEY_LAST + 1;
constexpr size_t glfw_mouse_button_count = GLFW_MOUSE_BUTTON_LAST + 1;
static_assert(glfw_key_count + glfw_mouse_button_count <= 512,
              "this system assumes the last glfw key values are not too large\n");

struct keybutton_state {
    bool is_pressed;
    // these two bools form the delta
    bool was_pressed;
    bool was_released;
};

struct window_state {
    /// <> current state.
    /// <>_delta change between last and second to last call to poll events.

    nm::uvec2 framebuffer_size;
    nm::ivec2 framebuffer_size_delta;

    nm::dvec2 mouse_pos;
    nm::dvec2 mouse_pos_delta;

    // glfw does not define an absolute scroll position, only deltas
    nm::dvec2 scroll_pos_delta;

    keybutton_state key_states[glfw_key_count];
    keybutton_state mouse_button_states[glfw_mouse_button_count];
};

struct window {
    GLFWwindow *handle;
    window_state state;
};

static void callback_error(int error, const char *description)
{
    nm::log(nm::LOG_ERROR, "glfw %s\n", description);
}

static void callback_key(GLFWwindow *glfw_window, int key, int scancode, int action, int mods)
{
    window *w = (window *)glfwGetWindowUserPointer(glfw_window);

    if (key < 0 || key > GLFW_KEY_LAST) {
        // unknown key and has no slot in the state array
        return;
    }

    // GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT
    // if GLFW_REPEAT, always also has PRESS or RELEASE

    keybutton_state *kbs = &(w->state.key_states[key]);
    if (action == GLFW_PRESS) {
        kbs->was_pressed = true;
        kbs->is_pressed  = true;
    } else if (action == GLFW_RELEASE) {
        kbs->was_released = true;
        kbs->is_pressed   = false;
    }
}

static void callback_framebuffer_size(GLFWwindow *glfw_window, int size_x, int size_y)
{
    window *w = (window *)glfwGetWindowUserPointer(glfw_window);

    nm::ivec2 framebuffer_size_new(size_x, size_y);
    w->state.framebuffer_size_delta = framebuffer_size_new - nm::ivec2(w->state.framebuffer_size);
    w->state.framebuffer_size       = nm::uvec2(framebuffer_size_new);
}

static void callback_mouse_button(GLFWwindow *glfw_window, int button, int action, int mods)
{
    window *w = (window *)glfwGetWindowUserPointer(glfw_window);

    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) {
        // unknown key and has no slot in the state array
        return;
    }

    // GLFW_PRESS or GLFW_RELEASE

    keybutton_state *kbs = &(w->state.mouse_button_states[button]);
    if (action == GLFW_PRESS) {
        kbs->was_pressed = true;
        kbs->is_pressed  = true;
    } else if (action == GLFW_RELEASE) {
        kbs->was_released = true;
        kbs->is_pressed   = false;
    }
}

static void callback_scroll(GLFWwindow *glfw_window, double xoffset, double yoffset)
{
    window *w = (window *)glfwGetWindowUserPointer(glfw_window);

    w->state.scroll_pos_delta = nm::dvec2(xoffset, yoffset);
}

static void callback_cursor_position(GLFWwindow *glfw_window, double xpos, double ypos)
{
    window *w = (window *)glfwGetWindowUserPointer(glfw_window);

    nm::dvec2 mouse_pos_new(xpos, ypos);
    w->state.mouse_pos_delta = mouse_pos_new - w->state.mouse_pos;
    w->state.mouse_pos       = mouse_pos_new;
}

static void reset_deltas(window *w)
{
    w->state.framebuffer_size_delta = nm::ivec2(0);
    w->state.mouse_pos_delta        = nm::dvec2(0);
    w->state.scroll_pos_delta       = nm::dvec2(0);

    for (uint32_t i = 0; i < glfw_key_count; i++) {
        w->state.key_states[i].was_pressed  = false;
        w->state.key_states[i].was_released = false;
    }

    for (uint32_t i = 0; i < glfw_mouse_button_count; i++) {
        w->state.mouse_button_states[i].was_pressed  = false;
        w->state.mouse_button_states[i].was_released = false;
    }
}

static void init_state(window *w)
{
    int32_t size_x, size_y;
    glfwGetWindowSize(w->handle, &size_x, &size_y);
    w->state.framebuffer_size = nm::uvec2(size_x, size_y);

    double pos_x, pos_y;
    glfwGetCursorPos(w->handle, &pos_x, &pos_y);
    w->state.mouse_pos = nm::dvec2(pos_x, pos_y);

    // glfw does not define an absolute value for scrolling

    for (uint32_t i = 0; i < glfw_key_count; i++) {
        // todo: this is technically invalid as it includes invalid key values
        w->state.key_states[i].is_pressed = glfwGetKey(w->handle, (int)i) == GLFW_PRESS;
    }

    for (uint32_t i = 0; i < glfw_mouse_button_count; i++) {
        // todo: this is technically invalid as it includes invalid
        //  mouse button values
        w->state.mouse_button_states[i].is_pressed =
            glfwGetMouseButton(w->handle, (int)i) == GLFW_PRESS;
    }

    reset_deltas(w);
}

nm_ret init(window **w, uint32_t size_x, uint32_t size_y, const char *title)
{
    *w = (window *)malloc(sizeof(window));

    glfwSetErrorCallback(callback_error);

    if (glfwInit() != GLFW_TRUE) {
        nm::log(nm::LOG_ERROR, "glfwInit failed\n");
        goto fail_on_init;
    }

    // minimum requirement for compute shaders
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 16);

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    (*w)->handle = glfwCreateWindow(size_x, size_y, title, NULL, NULL);
    if (!(*w)->handle) {
        nm::log(nm::LOG_ERROR, "glfwCreateWindow failed\n");
        goto fail_on_create_window;
    }

    init_state(*w);

    glfwSetWindowUserPointer((*w)->handle, *w);

    glfwSetKeyCallback((*w)->handle, callback_key);
    glfwSetFramebufferSizeCallback((*w)->handle, callback_framebuffer_size);
    glfwSetScrollCallback((*w)->handle, callback_scroll);
    glfwSetMouseButtonCallback((*w)->handle, callback_mouse_button);
    glfwSetCursorPosCallback((*w)->handle, callback_cursor_position);

    glfwMakeContextCurrent((*w)->handle);
    if (!gladLoadGL(glfwGetProcAddress)) {
        nm::log(nm::LOG_ERROR, "gladLoadGLLoader failed\n");
        goto fail_glad_load;
    }
    glfwSwapInterval(0);

    // call after making context current
    GLint channels[3];
    GL_CHECK(glGetFramebufferAttachmentParameteriv(
        GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &channels[0]));
    GL_CHECK(glGetFramebufferAttachmentParameteriv(
        GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &channels[1]));
    GL_CHECK(glGetFramebufferAttachmentParameteriv(
        GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &channels[2]));
    nm::log(nm::LOG_INFO, "r: %d, g: %d, b: %d\n", channels[0], channels[1], channels[2]);

    goto success;

fail_glad_load:
    glfwDestroyWindow((*w)->handle);
fail_on_create_window:
    glfwTerminate();
fail_on_init:
    return NM_FAIL;

success:
    return NM_SUCCESS;
}

void *get_handle(window *w) { return w->handle; }

void cleanup(window *w)
{
    // todo glad cleanup?
    glfwDestroyWindow(w->handle);
    glfwTerminate();

    free(w);
}

bool is_should_close(window *w) { return glfwWindowShouldClose(w->handle); }

void set_should_close(window *w) { glfwSetWindowShouldClose(w->handle, GLFW_TRUE); }

void swap_buffers(window *w) { glfwSwapBuffers(w->handle); }

void reset_events(window *w) { reset_deltas(w); }

void poll_events() { glfwPollEvents(); }

bool is_active(window *w)
{
    // todo: implement
    return true;
}

nm::uvec2 framebuffer_size(window *w) { return w->state.framebuffer_size; }

nm::dvec2 mouse_pos(window *w) { return w->state.mouse_pos; }

nm::dvec2 scroll_pos_delta(window *w) { return w->state.scroll_pos_delta; }

bool was_esc_pressed(window *w) { return w->state.key_states[GLFW_KEY_ESCAPE].was_pressed; }

bool is_shift_pressed(window *w) { return w->state.key_states[GLFW_KEY_LEFT_SHIFT].is_pressed; }

bool was_f1_pressed(window *w) { return w->state.key_states[GLFW_KEY_F1].was_pressed; }

bool was_f2_pressed(window *w) { return w->state.key_states[GLFW_KEY_F2].was_pressed; }

bool was_f3_pressed(window *w) { return w->state.key_states[GLFW_KEY_F3].was_pressed; }

bool was_f4_pressed(window *w) { return w->state.key_states[GLFW_KEY_F4].was_pressed; }

bool was_f5_pressed(window *w) { return w->state.key_states[GLFW_KEY_F5].was_pressed; }

bool was_enter_pressed(window *w) { return w->state.key_states[GLFW_KEY_ENTER].was_pressed; }

bool was_prtsc_pressed(window *w) { return w->state.key_states[GLFW_KEY_PRINT_SCREEN].was_pressed; }

bool is_mmb_pressed(window *w)
{
    return w->state.mouse_button_states[GLFW_MOUSE_BUTTON_MIDDLE].is_pressed;
}
