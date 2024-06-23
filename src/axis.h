#ifndef TERRAIN3_AXIS_H
#define TERRAIN3_AXIS_H

#include "nmutil/defs.h"
#include "nmutil/matrix.h"

/// Only call this once.
nm_ret init_axis();

/// Only call this after init_axis().
void render_axis(nm::mat4 mvp);

/// Only call this once, after init_axis().
void cleanup_axis();

#endif // TERRAIN3_AXIS_H
