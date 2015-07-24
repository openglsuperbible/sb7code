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

#include <object.h>
#include <shader.h>
#include <sb7ktx.h>

class perpixelgloss_app : public sb7::application
{
public:
    perpixelgloss_app()
        : render_prog(0)
    {
    }

protected:
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Toon Shading";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        static const GLubyte toon_tex_data[] =
        {
            0x44, 0x00, 0x00, 0x00,
            0x88, 0x00, 0x00, 0x00,
            0xCC, 0x00, 0x00, 0x00,
            0xFF, 0x00, 0x00, 0x00
        };

        glGenTextures(1, &tex_toon);
        glBindTexture(GL_TEXTURE_1D, tex_toon);
        glTexStorage1D(GL_TEXTURE_1D, 1, GL_RGB8, sizeof(toon_tex_data) / 4);
        glTexSubImage1D(GL_TEXTURE_1D, 0,
                        0, sizeof(toon_tex_data) / 4,
                        GL_RGBA, GL_UNSIGNED_BYTE,
                        toon_tex_data);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        object.load("media/objects/torus_nrms_tc.sbm");

        load_shaders();

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    virtual void render(double currentTime)
    {
        static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        static const GLfloat ones[] = { 1.0f };

        glClearBufferfv(GL_COLOR, 0, gray);
        glClearBufferfv(GL_DEPTH, 0, ones);

        glBindTexture(GL_TEXTURE_1D, tex_toon);

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glUseProgram(render_prog);

        vmath::mat4 proj_matrix = vmath::perspective(60.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -3.0f) *
                                vmath::rotate((float)currentTime * 13.75f, 0.0f, 1.0f, 0.0f) *
                                vmath::rotate((float)currentTime * 7.75f, 0.0f, 0.0f, 1.0f) *
                                vmath::rotate((float)currentTime * 15.3f, 1.0f, 0.0f, 0.0f);

        glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);
        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);

        object.render();
    }

    virtual void shutdown()
    {
        glDeleteProgram(render_prog);
        glDeleteTextures(1, &tex_toon);
    }

    void load_shaders()
    {
        if (render_prog)
            glDeleteProgram(render_prog);

        GLuint vs, fs;

        vs = sb7::shader::load("media/shaders/toonshading/toonshading.vs.glsl", GL_VERTEX_SHADER);
        fs = sb7::shader::load("media/shaders/toonshading/toonshading.fs.glsl", GL_FRAGMENT_SHADER);

        render_prog = glCreateProgram();
        glAttachShader(render_prog, vs);
        glAttachShader(render_prog, fs);
        glLinkProgram(render_prog);

        glDeleteShader(vs);
        glDeleteShader(fs);

        uniforms.mv_matrix = glGetUniformLocation(render_prog, "mv_matrix");
        uniforms.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");
    }

    virtual void onKey(int key, int action)
    {
        if (action)
        {
            switch (key)
            {
                case 'R': load_shaders();
                    break;
            }
        }
    }

protected:
    GLuint          render_prog;

    GLuint          tex_toon;

    struct
    {
        GLint       mv_matrix;
        GLint       proj_matrix;
    } uniforms;

    sb7::object     object;
};

DECLARE_MAIN(perpixelgloss_app)
