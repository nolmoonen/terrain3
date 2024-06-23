#ifndef TERRAIN3_GUI_H
#define TERRAIN3_GUI_H

#include "window.hpp"

#include <nmutil/vector.h>

#include <chrono>

void gui_init(window* w);

void gui_cleanup();

void begin_frame_imgui();

void end_frame_imgui();

void display_text(const char *text, float x, float y);

void display_stats(
        std::chrono::duration<double> &update_time,
        std::chrono::duration<double> &render_time);

void display_pos(nm::fvec3 pos);

#endif //TERRAIN3_GUI_H
