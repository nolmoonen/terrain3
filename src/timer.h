#ifndef TERRAIN3_TIMER_H
#define TERRAIN3_TIMER_H

#include "windows.h"

struct timer {
    LARGE_INTEGER start;
};

static LARGE_INTEGER frequency{};

void timer_start(timer* t)
{
    if (!frequency.QuadPart) {
        // cannot fail on WinXP or later and gets non-zero value
        QueryPerformanceFrequency(&frequency);
    }

    QueryPerformanceCounter(&t->start);
}

/// Resets timer and returns elapsed time in seconds.
float timer_lap(timer* t)
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    LARGE_INTEGER elapsed;
    elapsed.QuadPart = now.QuadPart - t->start.QuadPart;
    t->start         = now;

    return float(elapsed.QuadPart) / float(frequency.QuadPart);
}

#endif //TERRAIN3_TIMER_H
