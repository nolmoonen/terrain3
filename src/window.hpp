#ifndef TERRAIN4_WINDOW_HPP
#define TERRAIN4_WINDOW_HPP

#include <nmutil/defs.h>
#include <nmutil/vector.h>

struct window;

nm_ret init(window **w, uint32_t size_x, uint32_t size_y, const char *title);

void *get_handle(window *w);

void cleanup(window *w);

bool is_should_close(window *w);

void set_should_close(window *w);

void swap_buffers(window *w);

void reset_events(window *w);

/// Window-agnostic.
void poll_events();

bool is_active(window *w);

nm::uvec2 framebuffer_size(window *w);

nm::dvec2 mouse_pos(window *w);

nm::dvec2 scroll_pos_delta(window *w);

bool was_esc_pressed(window *w);

// todo is "is_down" a better name for these methods?

bool is_shift_pressed(window *w);

bool was_f1_pressed(window *w);

bool was_f2_pressed(window *w);

bool was_f3_pressed(window *w);

bool was_f4_pressed(window *w);

bool was_f5_pressed(window *w);

bool was_enter_pressed(window *w);

bool was_prtsc_pressed(window *w);

bool is_mmb_pressed(window *w);

#endif // TERRAIN4_WINDOW_HPP
