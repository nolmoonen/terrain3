#include "axis.h"
#include "nmutil/gl.h"
#include "nmutil/io.h"

/// Coordinate frame.
static const struct {
    float x, y, z;
    float r, g, b;
} vertices[12] = {
        // x
        {+.5f, 0.f,  0.f,  1.f, 0.f, 0.f},
        {0.f,  0.f,  0.f,  1.f, 0.f, 0.f},
        {-.5f, 0.f,  0.f,  1.f, .5f, .5f},
        {0.f,  0.f,  0.f,  1.f, .5f, .5f},
        // y
        {0.f,  +.5f, 0.f,  0.f, 1.f, 0.f},
        {0.f,  0.f,  0.f,  0.f, 1.f, 0.f},
        {0.f,  -.5f, 0.f,  .5f, 1.f, .5f},
        {0.f,  0.f,  0.f,  .5f, 1.f, .5f},
        // z
        {0.f,  0.f,  +.5f, 0.f, 0.f, 1.f},
        {0.f,  0.f,  0.f,  0.f, 0.f, 1.f},
        {0.f,  0.f,  -.5f, .5f, .5f, 1.f},
        {0.f,  0.f,  0.f,  .5f, .5f, 1.f}
};

static nmutil::shader_program program;
static GLuint lines_vao;
static GLuint lines_vbo;

nm_ret init_axis()
{
    nm_ret ret;

    nmutil::res_t vert_src{};
    ret = nmutil::read_file(
            &vert_src.text, &vert_src.len, "../res/shader/axis.vert");
    if (ret != NM_SUCCESS) {
        return -1;
    }

    nmutil::res_t frag_src{};
    ret = nmutil::read_file(
            &frag_src.text, &frag_src.len, "../res/shader/axis.frag");
    if (ret != NM_SUCCESS) {
        return -1;
    }

    ret = program.init(&vert_src, nullptr, &frag_src, nullptr);
    if (ret != NM_SUCCESS) {
        return -1;
    }

    GL_CHECK(glGenVertexArrays(1, &lines_vao));
    GL_CHECK(glBindVertexArray(lines_vao));

    GL_CHECK(glGenBuffers(1, &lines_vbo));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, lines_vbo));
    GL_CHECK(glBufferData(
            GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void *) 0));

    GL_CHECK(glEnableVertexAttribArray(1));
    GL_CHECK(glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6,
            (void *) (sizeof(float) * 3)));

    GL_CHECK(glBindVertexArray(0));

    return NM_SUCCESS;
}

void render_axis(mat4 mvp)
{
    program.use();
    glBindVertexArray(lines_vao);
    program.set_mat4("mvp", mvp);
    GL_CHECK(glDrawArrays(GL_LINES, 0, 12));
    glBindVertexArray(0);
    program.unuse();
}

void cleanup_axis()
{
    GL_CHECK(glDeleteBuffers(1, &lines_vbo));
    GL_CHECK(glDeleteBuffers(1, &lines_vao));
    program.cleanup();
}