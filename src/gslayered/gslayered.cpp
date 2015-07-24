/*
 * Copyright © 2012-2015 Graham Sellers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <sb7.h>
#include <vmath.h>
#include <sb7ktx.h>
#include <shader.h>
#include <object.h>

#include <string>
static void print_shader_log(GLuint shader)
{
    std::string str;
    GLint len;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    str.resize(len);
    glGetShaderInfoLog(shader, len, NULL, &str[0]);

#ifdef _WIN32
    OutputDebugStringA(str.c_str());
#endif
}

class gslayered_app : public sb7::application
{
public:
    gslayered_app()
        : program_gslayers(0),
          program_showlayers(0),
          mode(0)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Layered Rendering";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup(void)
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        load_shaders();

        obj.load("media/objects/torus.sbm");

        glGenBuffers(1, &transform_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, transform_ubo);
        glBufferData(GL_UNIFORM_BUFFER, 17 * sizeof(vmath::mat4), NULL, GL_DYNAMIC_DRAW);

        glGenTextures(1, &array_texture);
        glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, 256, 256, 16);

        glGenTextures(1, &array_depth);
        glBindTexture(GL_TEXTURE_2D_ARRAY, array_depth);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32, 256, 256, 16);

        glGenFramebuffers(1, &layered_fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, layered_fbo);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, array_texture, 0);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, array_depth, 0);
    }

    void shutdown(void)
    {
        glDeleteProgram(program_showlayers);
        glDeleteProgram(program_gslayers);
        glDeleteVertexArrays(1, &vao);
    }

    void render(double t)
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat gray[] =  { 0.1f, 0.1f, 0.1f, 1.0f };
        static const GLfloat one = 1.0f;

        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f) *
                                vmath::rotate((float)t * 5.0f, 0.0f, 0.0f, 1.0f) *
                                vmath::rotate((float)t * 30.0f, 1.0f, 0.0f, 0.0f);
        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        vmath::mat4 mvp = proj_matrix * mv_matrix;

        struct TRANSFORM_BUFFER
        {
            vmath::mat4 proj_matrix;
            vmath::mat4 mv_matrix[16];
        };

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);

        TRANSFORM_BUFFER * buffer = (TRANSFORM_BUFFER *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(TRANSFORM_BUFFER), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        buffer->proj_matrix = vmath::perspective(50.0f, 1.0f, 0.1f, 1000.0f); // proj_matrix;
        int i;

        for (i = 0; i < 16; i++)
        {
            float fi = (float)(i + 12) / 16.0f;
            buffer->mv_matrix[i] = vmath::translate(0.0f, 0.0f, -4.0f) *
                                   vmath::rotate((float)t * 25.0f * fi, 0.0f, 0.0f, 1.0f) *
                                   vmath::rotate((float)t * 30.0f * fi, 1.0f, 0.0f, 0.0f);
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        static const GLenum ca0 = GL_COLOR_ATTACHMENT0;

        glBindFramebuffer(GL_FRAMEBUFFER, layered_fbo);
        glDrawBuffers(1, &ca0);
        glViewport(0, 0, 256, 256);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glUseProgram(program_gslayers);

        obj.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDrawBuffer(GL_BACK);
        glUseProgram(program_showlayers);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, gray);

        glBindTexture(GL_TEXTURE_2D_ARRAY, array_texture);
        glDisable(GL_DEPTH_TEST);

        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 16);

        glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    }

    void onKey(int key, int action)
    {
        if (!action)
            return;

        switch (key)
        {
            case '1':
            case '2':
                    mode = key - '1';
                break;
            case 'R':
                    load_shaders();
                break;
            case 'M':
                    mode = (mode + 1) % 2;
                break;
        }
    }

    void load_shaders()
    {
        GLuint vs;
        GLuint gs;
        GLuint fs;

        if (program_showlayers)
            glDeleteProgram(program_showlayers);

        program_showlayers = glCreateProgram();

        vs = sb7::shader::load("media/shaders/gslayers/showlayers.vs.glsl", GL_VERTEX_SHADER);
        fs = sb7::shader::load("media/shaders/gslayers/showlayers.fs.glsl", GL_FRAGMENT_SHADER);

        glAttachShader(program_showlayers, vs);
        glAttachShader(program_showlayers, fs);

        glLinkProgram(program_showlayers);

        glDeleteShader(vs);
        glDeleteShader(fs);

        vs = sb7::shader::load("media/shaders/gslayers/gslayers.vs.glsl", GL_VERTEX_SHADER);
        gs = sb7::shader::load("media/shaders/gslayers/gslayers.gs.glsl", GL_GEOMETRY_SHADER);
        fs = sb7::shader::load("media/shaders/gslayers/gslayers.fs.glsl", GL_FRAGMENT_SHADER);

        if (program_gslayers)
            glDeleteProgram(program_gslayers);

        program_gslayers = glCreateProgram();

        glAttachShader(program_gslayers, vs);
        glAttachShader(program_gslayers, gs);
        glAttachShader(program_gslayers, fs);

        glLinkProgram(program_gslayers);

        glDeleteShader(vs);
        glDeleteShader(gs);
        glDeleteShader(fs);
    }

private:
    GLuint      program_gslayers;
    GLuint      program_showlayers;
    GLuint      vao;
    int         mode;
    GLuint      transform_ubo;

    GLuint      layered_fbo;
    GLuint      array_texture;
    GLuint      array_depth;

    sb7::object obj;
};

DECLARE_MAIN(gslayered_app);
