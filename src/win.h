#ifndef TERRAIN3_WIN_H
#define TERRAIN3_WIN_H

#include <cstdint>
#include "nmutil/defs.h"

/// Zero is initialization.
struct messages {
    bool f1_down;
    bool f2_down;
    bool f3_down;
    bool f4_down;
    bool enter_down;
    // windows only reports prtsc for key up
    bool prtsc_up;
    bool lmb_down;
    bool lmb_up;
    bool mmb_down;
    bool mmb_up;
    bool rmb_down;
    bool rmb_up;
    int32_t wheel_delta;
};

/// Create the window and everything else we need, including the device and
/// rendering context. If a fullscreen window has been requested but can't be
/// created, the user will be prompted to attempt windowed mode. Finally,
/// initializeScene is called for application-specific setup.
///
/// Returns TRUE if everything goes well, or FALSE if an unrecoverable error
/// occurs. Note that if this is called twice within a program_id, cleanup_window
/// needs to be called before subsequent calls to setup_window.
nm_ret setup_window(const char *title, int width, int height);

/// Deletes the DC, RC, and Window, and restores the original display.
void cleanup_window();

void swap_buffers();

void poll_events();

void *get_hwnd();

bool is_active();

bool should_stop();

uint32_t get_size_x();

uint32_t get_size_y();

int32_t get_mouse_x();

int32_t get_mouse_y();

messages get_messages();

bool get_shift_down();

#endif //TERRAIN3_WIN_H
