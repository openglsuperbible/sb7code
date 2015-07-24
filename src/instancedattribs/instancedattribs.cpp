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

static const char * square_vs_source[] =
{
    "#version 410 core                                                               \n"
    "                                                                                \n"
    "layout (location = 0) in vec4 position;                                         \n"
    "layout (location = 1) in vec4 instance_color;                                   \n"
    "layout (location = 2) in vec4 instance_position;                                \n"
    "                                                                                \n"
    "out Fragment                                                                    \n"
    "{                                                                               \n"
    "    vec4 color;                                                                 \n"
    "} fragment;                                                                     \n"
    "                                                                                \n"
    "void main(void)                                                                 \n"
    "{                                                                               \n"
    "    gl_Position = (position + instance_position) * vec4(0.25, 0.25, 1.0, 1.0);    \n"
    "    fragment.color = instance_color;                                            \n"
    "}                                                                               \n"
};

static const char * square_fs_source[] =
{
    "#version 410 core                                                                \n"
    "precision highp float;                                                           \n"
    "                                                                                 \n"
    "in Fragment                                                                      \n"
    "{                                                                                \n"
    "    vec4 color;                                                                  \n"
    "} fragment;                                                                      \n"
    "                                                                                 \n"
    "out vec4 color;                                                                  \n"
    "                                                                                 \n"
    "void main(void)                                                                  \n"
    "{                                                                                \n"
    "    color = fragment.color;                                                      \n"
    "}                                                                                \n"
};

class instancing_app : public sb7::application
{
public:
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Instanced Attributes";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup(void)
    {
        static const GLfloat square_vertices[] =
        {
            -1.0f, -1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f,
             1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f,  1.0f, 0.0f, 1.0f
        };

        static const GLfloat instance_colors[] =
        {
            1.0f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 0.0f, 1.0f
        };

        static const GLfloat instance_positions[] =
        {
            -2.0f, -2.0f, 0.0f, 0.0f,
             2.0f, -2.0f, 0.0f, 0.0f,
             2.0f,  2.0f, 0.0f, 0.0f,
            -2.0f,  2.0f, 0.0f, 0.0f
        };

        GLuint offset = 0;

        glGenVertexArrays(1, &square_vao);
        glGenBuffers(1, &square_buffer);
        glBindVertexArray(square_vao);
        glBindBuffer(GL_ARRAY_BUFFER, square_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(square_vertices) + sizeof(instance_colors) + sizeof(instance_positions), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(square_vertices), square_vertices);
        offset += sizeof(square_vertices);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(instance_colors), instance_colors);
        offset += sizeof(instance_colors);
        glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(instance_positions), instance_positions);
        offset += sizeof(instance_positions);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(square_vertices));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(sizeof(square_vertices) + sizeof(instance_colors)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        glVertexAttribDivisor(1, 1);
        glVertexAttribDivisor(2, 1);

        square_program = glCreateProgram();

        GLuint square_vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(square_vs, 1, square_vs_source, NULL);
        glCompileShader(square_vs);
        glAttachShader(square_program, square_vs);
        GLuint square_fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(square_fs, 1, square_fs_source, NULL);
        glCompileShader(square_fs);
        glAttachShader(square_program, square_fs);

        glLinkProgram(square_program);
        glDeleteShader(square_vs);
        glDeleteShader(square_fs);
    }

    void shutdown(void)
    {
        glDeleteProgram(square_program);
        glDeleteBuffers(1, &square_buffer);
        glDeleteVertexArrays(1, &square_vao);
    }

    void render(double t)
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        glClearBufferfv(GL_COLOR, 0, black);

        glUseProgram(square_program);
        glBindVertexArray(square_vao);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 4);
    }

private:
    GLuint      square_buffer;
    GLuint      square_vao;

    GLuint      square_program;
};

DECLARE_MAIN(instancing_app);
