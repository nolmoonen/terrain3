#include "gl.h"

namespace nmutil {
    GLint gl_ubo_alignment;
    GLint gl_max_compute_work_group_count[3];

    void load_gl_constants()
    {
        GL_CHECK(glGetIntegerv(
                GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &gl_ubo_alignment));
        GL_CHECK(glGetIntegeri_v(
                GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0,
                &gl_max_compute_work_group_count[0]));
        GL_CHECK(glGetIntegeri_v(
                GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1,
                &gl_max_compute_work_group_count[1]));
        GL_CHECK(glGetIntegeri_v(
                GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2,
                &gl_max_compute_work_group_count[2]));
    }
}