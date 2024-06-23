#ifndef NMUTUL_GL_H
#define NMUTUL_GL_H

#include "defs.h"
#include "log.h"
#include "vector.h"
#include "matrix.h"

#include <glad/gl.h>

#include <cassert>
#include <cstdlib>

// todo shouldn't this do "err = call"?
#define GL_CHECK(call)                                                    \
    do                                                                    \
    {                                                                     \
        call;                                                             \
        GLenum err = glGetError();                                        \
        if (err != GL_NO_ERROR)                                           \
        {                                                                 \
            nm::log(nm::LOG_ERROR, "GL error %s at %s (%u): " #call "\n", \
                    nm::get_gl_error_string(err), __FILE__, __LINE__);    \
        }                                                                 \
    } while (0)

#define GL_CHECK_ERRORS()                                              \
    do                                                                 \
    {                                                                  \
        GLenum err = glGetError();                                     \
        if (err != GL_NO_ERROR)                                        \
        {                                                              \
            nm::log(nm::LOG_ERROR, "GL error %s at %s (%u)\n",         \
                    nm::get_gl_error_string(err), __FILE__, __LINE__); \
        }                                                              \
    } while (0)

namespace nm
{

    const char *get_gl_error_string(GLenum error);

    struct tex
    {
        GLuint id;
        GLenum type;

        nm_ret init(GLenum p_type);

        /// Takes in byte data, specify number of channels.
        /// Note: assumes GL_TEXTURE_2D.
        nm_ret load(u8 *data, u32 x, u32 y, u32 n);

        void cleanup();

        void use();

        void use(GLenum tex_unit);

        void unuse();

        void unuse(GLenum tex_unit);
    };

    struct shader
    {
        GLuint id;

        nm_ret init(const char *shader_text, GLint shader_size, GLenum shader_type);

        void cleanup();
    };

    /// Simple resource, can later be used to embed files in the executable.
    struct res_t
    {
        char *text;
        size_t len;
    };

    struct shader_program
    {
        GLuint id;

        /// Initializes from source. Creates shaders and destroys them.
        nm_ret init(const res_t *vert_src, const res_t *geom_src, const res_t *frag_src,
                    const res_t *comp_src, bool use_feedback = false);

        /// Initializes from existing shader objects. Does not destroy them
        /// when initialization of the program is done.
        nm_ret init(shader *vert_shader, shader *geom_shader, shader *frag_shader, shader *comp_shader,
                    bool use_feedback = false);

        void cleanup();

        void use();

        void unuse();

        void bind_uniform_block(const char *name, GLuint binding);

        GLint get_uniform_loc(const char *name);

        // todo is non-static the best here? OpenGL does not accept if
        //  the shader is not bound currently

        /** ivec2 */

        void set_ivec2_loc(GLint loc, ivec2 val);

        void set_ivec2(const char *name, ivec2 val);

        /** ivec3 */

        void set_ivec3_loc(GLint loc, ivec3 val);

        void set_ivec3(const char *name, ivec3 val);

        /** vec3 */

        void set_vec3_loc(GLint loc, fvec3 val);

        void set_vec3(const char *name, fvec3 val);

        /** mat4, assumes it is row-major. */

        void set_mat4_loc(GLint loc, mat4 val);

        void set_mat4(const char *name, mat4 val);

        /** float */

        void set_float(const char *name, float val);

        /** int */

        void set_int(const char *name, int32_t val);

        /** uint */

        void set_uint(const char *name, uint32_t val);

        /** float array */

        void set_float_array(const char *name, float *val, uint32_t count);

        /** vec2 array */

        void set_vec2_array_loc(GLint loc, fvec2 *val, uint32_t count);

        void set_vec2_array(const char *name, fvec2 *val, uint32_t count);

        /** ivec2 array */

        void set_ivec2_array_loc(GLint loc, ivec2 *val, uint32_t count);

        void set_ivec2_array(const char *name, ivec2 *val, uint32_t count);
    };

    inline const char *get_gl_error_string(GLenum error)
    {
        switch (error)
        {
        case GL_NO_ERROR:
            return "No error";
        case GL_INVALID_ENUM:
            return "Invalid enum";
        case GL_INVALID_VALUE:
            return "Invalid value";
        case GL_INVALID_OPERATION:
            return "Invalid operation";
        case GL_OUT_OF_MEMORY:
            return "Out of memory";
        default:
            return "Unknown GL error";
        }
    }

    inline nm_ret tex::init(GLenum p_type)
    {
        type = p_type;
        GL_CHECK(glGenTextures(1, &id));
        return NM_SUCCESS;
    }

    inline nm_ret tex::load(u8 *data, u32 x, u32 y, u32 n)
    {
        use();

        // number of color components in the texture
        GLint internal_format;
        // format of the pixel data
        GLenum format;
        switch (n)
        {
        case 1:
            internal_format = GL_RG;
            format = GL_RED;
            break;
        case 2:
            internal_format = GL_RG;
            format = GL_RG;
            break;
        case 3:
            internal_format = GL_RGBA;
            format = GL_RGB;
            break;
        case 4:
            internal_format = GL_RGBA;
            format = GL_RGBA;
            break;
        default:
            return NM_FAIL;
        }

        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, (GLsizei)x, (GLsizei)y, 0, format,
                              GL_UNSIGNED_BYTE, data));
        GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

        unuse();

        return NM_SUCCESS;
    }

    inline void tex::cleanup() { GL_CHECK(glDeleteTextures(1, &id)); }

    inline void tex::use() { GL_CHECK(glBindTexture(type, id)); }

    inline void tex::use(GLenum tex_unit)
    {
        GL_CHECK(glActiveTexture(tex_unit));
        GL_CHECK(glBindTexture(type, id));
    }

    inline void tex::unuse() { GL_CHECK(glBindTexture(type, 0)); }

    inline void tex::unuse(GLenum tex_unit)
    {
        GL_CHECK(glActiveTexture(tex_unit));
        GL_CHECK(glBindTexture(type, 0));
    }

    inline nm_ret shader::init(const char *shader_text, GLint shader_size, GLenum shader_type)
    {
        id = glCreateShader(shader_type);
        GL_CHECK_ERRORS();
        assert(id != 0);

        GL_CHECK(glShaderSource(id, 1, &shader_text, shader_size ? &shader_size : NULL));

        GL_CHECK(glCompileShader(id));

        GLint success = GL_FALSE;
        GL_CHECK(glGetShaderiv(id, GL_COMPILE_STATUS, &success));

        if (success == GL_FALSE)
        {
            GLint info_log_length = 0; // number of characters in info log
            GL_CHECK(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &info_log_length));

            GLchar *info_log = (GLchar *)malloc(info_log_length * sizeof(GLchar));

            if (info_log == NULL)
            {
                log(LOG_ERROR, "could not allocate memory for info log\n");

                GL_CHECK(glDeleteShader(id));

                return NM_FAIL;
            }

            GL_CHECK(glGetShaderInfoLog(id, info_log_length, NULL, info_log));

            info_log[info_log_length - 1] = '\0'; // null terminate
            log(LOG_ERROR, "shader compilation failed:\n%s\n", info_log);
            free(info_log);

            GL_CHECK(glDeleteShader(id));

            return NM_FAIL;
        }

        return NM_SUCCESS;
    }

    inline void shader::cleanup() { GL_CHECK(glDeleteShader(id)); }

    inline nm_ret shader_program::init(const res_t *vert_src, const res_t *geom_src,
                                       const res_t *frag_src, const res_t *comp_src, bool use_feedback)
    {
        nm_ret ret;

        // shaders
        shader vert_shader;
        shader geom_shader;
        shader frag_shader;
        shader comp_shader;

        // create vertex shader_id
        if (vert_src)
        {
            ret = vert_shader.init(vert_src->text, (GLint)vert_src->len, GL_VERTEX_SHADER);
            if (ret == NM_FAIL)
            {
                log(LOG_ERROR, "vert shader creation failed\n");

                return NM_FAIL;
            }
        }

        // create geometry shader_id
        if (geom_src)
        {
            ret = geom_shader.init(geom_src->text, (GLint)geom_src->len, GL_GEOMETRY_SHADER);
            if (ret == NM_FAIL)
            {
                log(LOG_ERROR, "geom shader creation failed\n");

                // delete created shaders
                if (vert_src)
                    vert_shader.cleanup();

                return NM_FAIL;
            }
        }

        // create fragment shader_id
        if (frag_src)
        {
            ret = frag_shader.init(frag_src->text, (GLint)frag_src->len, GL_FRAGMENT_SHADER);
            if (ret == NM_FAIL)
            {
                log(LOG_ERROR, "frag shader creation failed\n");

                // delete created shaders
                if (geom_src)
                    geom_shader.cleanup();
                if (vert_src)
                    vert_shader.cleanup();

                return NM_FAIL;
            }
        }
        // create compute shader_id
        if (comp_src)
        {
            ret = comp_shader.init(comp_src->text, (GLint)comp_src->len, GL_COMPUTE_SHADER);
            if (ret == NM_FAIL)
            {
                log(LOG_ERROR, "comp shader creation failed\n");

                // delete created shaders
                if (frag_src)
                    frag_shader.cleanup();
                if (geom_src)
                    geom_shader.cleanup();
                if (vert_src)
                    vert_shader.cleanup();

                return NM_FAIL;
            }
        }

        // safe to use res_t pointer here as a check, if we could not create
        // shader we exited
        ret = init(vert_src ? &vert_shader : nullptr, geom_src ? &geom_shader : nullptr,
                   frag_src ? &frag_shader : nullptr, comp_src ? &comp_shader : nullptr, use_feedback);
        // rely on init call to print specific error
        if (ret == NM_FAIL)
            return NM_FAIL;

        // delete shaders
        if (comp_src)
            comp_shader.cleanup();
        if (frag_src)
            frag_shader.cleanup();
        if (geom_src)
            geom_shader.cleanup();
        if (vert_src)
            vert_shader.cleanup();

        return NM_SUCCESS;
    }

    inline nm_ret shader_program::init(shader *vert_shader, shader *geom_shader, shader *frag_shader,
                                       shader *comp_shader, bool use_feedback)
    {
        // create shader program id
        id = glCreateProgram();
        GL_CHECK_ERRORS();

        if (vert_shader)
            GL_CHECK(glAttachShader(id, vert_shader->id));
        if (geom_shader)
            GL_CHECK(glAttachShader(id, geom_shader->id));
        if (frag_shader)
            GL_CHECK(glAttachShader(id, frag_shader->id));
        if (comp_shader)
            GL_CHECK(glAttachShader(id, comp_shader->id));

        // todo handle this better
        if (use_feedback)
        {
            const GLchar *feedback_varyings[] = {"rec_pos", "rec_nor"};
            GL_CHECK(glTransformFeedbackVaryings(id, 2, feedback_varyings, GL_INTERLEAVED_ATTRIBS));
        }

        GL_CHECK(glLinkProgram(id));

        GLint success = GL_FALSE;
        GL_CHECK(glGetProgramiv(id, GL_LINK_STATUS, (int *)&success));
        if (success == GL_FALSE)
        {
            GLint info_log_length = 0;
            GL_CHECK(glGetProgramiv(id, GL_INFO_LOG_LENGTH, &info_log_length));
            GLchar *info_log = (GLchar *)malloc(info_log_length * sizeof(GLchar));

            if (info_log == NULL)
            {
                log(LOG_ERROR, "could not allocate memory for log info\n");

                goto shader_program_fail;
            }

            GL_CHECK(glGetProgramInfoLog(id, info_log_length, &info_log_length, info_log));
            info_log[info_log_length - 1] = '\0'; // null terminate
            log(LOG_ERROR, "shader program linking failed:\n %s\n", info_log);
            free(info_log);

        shader_program_fail:

            if (comp_shader)
                GL_CHECK(glDetachShader(id, comp_shader->id));
            if (frag_shader)
                GL_CHECK(glDetachShader(id, frag_shader->id));
            if (geom_shader)
                GL_CHECK(glDetachShader(id, geom_shader->id));
            if (vert_shader)
                GL_CHECK(glDetachShader(id, vert_shader->id));

            GL_CHECK(glDeleteProgram(id));

            return NM_FAIL;
        }

        if (comp_shader)
            GL_CHECK(glDetachShader(id, comp_shader->id));
        if (frag_shader)
            GL_CHECK(glDetachShader(id, frag_shader->id));
        if (geom_shader)
            GL_CHECK(glDetachShader(id, geom_shader->id));
        if (vert_shader)
            GL_CHECK(glDetachShader(id, vert_shader->id));

        return NM_SUCCESS;
    }

    inline void shader_program::cleanup() { GL_CHECK(glDeleteProgram(id)); }

    inline void shader_program::use() { GL_CHECK(glUseProgram(id)); }

    inline void shader_program::unuse() { glUseProgram(0); }

    inline void shader_program::bind_uniform_block(const char *name, GLuint binding)
    {
        GLuint loc = glGetUniformBlockIndex(id, name);
        GL_CHECK_ERRORS();
        GL_CHECK(glUniformBlockBinding(id, loc, binding));
    }

    inline GLint shader_program::get_uniform_loc(const char *name)
    {
        GLint location = glGetUniformLocation(id, name);
        GL_CHECK_ERRORS();
        return location;
    }

// todo do this with all
#define SET_INT(id, name, val)                                                                     \
    do {                                                                                           \
        GLint loc = glGetUniformLocation(id, name);                                                \
        GL_CHECK_ERRORS();                                                                         \
        GL_CHECK(glUniform1i(loc, val));                                                           \
    } while (0)

inline void shader_program::set_ivec2_loc(GLint loc, ivec2 val)
{
    GL_CHECK(glUniform2i(loc, val.x, val.y));
}

inline void shader_program::set_ivec2(const char *name, ivec2 val)
{
    set_ivec2_loc(get_uniform_loc(name), val);
}

inline void shader_program::set_ivec3_loc(GLint loc, ivec3 val)
{
    GL_CHECK(glUniform3i(loc, val.x, val.y, val.z));
}

inline void shader_program::set_ivec3(const char *name, ivec3 val)
{
    set_ivec3_loc(get_uniform_loc(name), val);
}

inline void shader_program::set_vec3_loc(GLint loc, fvec3 val)
{
    GL_CHECK(glUniform3f(loc, val.x, val.y, val.z));
}

inline void shader_program::set_vec3(const char *name, fvec3 val)
{
    set_vec3_loc(get_uniform_loc(name), val);
}

inline void shader_program::set_mat4_loc(GLint loc, mat4 val)
{
    GL_CHECK(glUniformMatrix4fv(loc, 1, GL_TRUE, val.data));
}

inline void shader_program::set_mat4(const char *name, mat4 val)
{
    set_mat4_loc(get_uniform_loc(name), val);
}

inline void shader_program::set_float(const char *name, float val)
{
    GL_CHECK(glUniform1f(get_uniform_loc(name), val));
}

inline void shader_program::set_int(const char *name, int32_t val)
{
    GL_CHECK(glUniform1i(get_uniform_loc(name), val));
}

inline void shader_program::set_uint(const char *name, uint32_t val)
{
    GL_CHECK(glUniform1ui(get_uniform_loc(name), val));
}

inline void shader_program::set_float_array(const char *name, float *val, uint32_t count)
{
    GL_CHECK(glUniform1fv(get_uniform_loc(name), (GLsizei)count, val));
}

inline void shader_program::set_vec2_array_loc(GLint loc, fvec2 *val, uint32_t count)
{
    GL_CHECK(glUniform2fv(loc, (GLsizei)count, &val->x));
}

inline void shader_program::set_vec2_array(const char *name, fvec2 *val, uint32_t count)
{
    set_vec2_array_loc(get_uniform_loc(name), val, count);
}

inline void shader_program::set_ivec2_array_loc(GLint loc, ivec2 *val, uint32_t count)
{
    GL_CHECK(glUniform2iv(loc, (GLsizei)count, &val->x));
}

inline void shader_program::set_ivec2_array(const char *name, ivec2 *val, uint32_t count)
{
    set_ivec2_array_loc(get_uniform_loc(name), val, count);
}

inline void load_gl_constants(GLint &gl_ubo_alignment, GLint (&gl_max_compute_work_group_count)[3])
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

#endif // NMUTUL_GL_H
