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
#include <sb7ktx.h>
#include <vmath.h>

#include <cmath>

// Random number generator
static unsigned int seed = 0x13371337;

static inline float random_float()
{
    float res;
    unsigned int tmp;

    seed *= 16807;

    tmp = seed ^ (seed >> 4) ^ (seed << 15);

    *((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;

    return (res - 1.0f);
}

enum
{
    NUM_STARS           = 2000
};

class starfield_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Starfield";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup()
    {
        GLuint  vs, fs;

        static const char * fs_source[] =
        {
            "#version 410 core                                              \n"
            "                                                               \n"
            "layout (location = 0) out vec4 color;                          \n"
            "                                                               \n"
            "uniform sampler2D tex_star;                                    \n"
            "flat in vec4 starColor;                                        \n"
            "                                                               \n"
            "void main(void)                                                \n"
            "{                                                              \n"
            "    color = starColor * texture(tex_star, gl_PointCoord);      \n"
            "}                                                              \n"
        };

        static const char * vs_source[] =
        {
            "#version 410 core                                              \n"
            "                                                               \n"
            "layout (location = 0) in vec4 position;                        \n"
            "layout (location = 1) in vec4 color;                           \n"
            "                                                               \n"
            "uniform float time;                                            \n"
            "uniform mat4 proj_matrix;                                      \n"
            "                                                               \n"
            "flat out vec4 starColor;                                       \n"
            "                                                               \n"
            "void main(void)                                                \n"
            "{                                                              \n"
            "    vec4 newVertex = position;                                 \n"
            "                                                               \n"
            "    newVertex.z += time;                                       \n"
            "    newVertex.z = fract(newVertex.z);                          \n"
            "                                                               \n"
            "    float size = (20.0 * newVertex.z * newVertex.z);           \n"
            "                                                               \n"
            "    starColor = smoothstep(1.0, 7.0, size) * color;            \n"
            "                                                               \n"
            "    newVertex.z = (999.9 * newVertex.z) - 1000.0;              \n"
            "    gl_Position = proj_matrix * newVertex;                     \n"
            "    gl_PointSize = size;                                       \n"
            "}                                                              \n"
        };

        vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vs_source, NULL);
        glCompileShader(vs);

        fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        render_prog = glCreateProgram();
        glAttachShader(render_prog, vs);
        glAttachShader(render_prog, fs);
        glLinkProgram(render_prog);

        glDeleteShader(vs);
        glDeleteShader(fs);

        uniforms.time = glGetUniformLocation(render_prog, "time");
        uniforms.proj_matrix = glGetUniformLocation(render_prog, "proj_matrix");

        star_texture = sb7::ktx::file::load("media/textures/star.ktx");

        glGenVertexArrays(1, &star_vao);
        glBindVertexArray(star_vao);

        struct star_t
        {
            vmath::vec3     position;
            vmath::vec3     color;
        };

        glGenBuffers(1, &star_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, star_buffer);
        glBufferData(GL_ARRAY_BUFFER, NUM_STARS * sizeof(star_t), NULL, GL_STATIC_DRAW);

        star_t * star = (star_t *)glMapBufferRange(GL_ARRAY_BUFFER, 0, NUM_STARS * sizeof(star_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        int i;

        for (i = 0; i < 1000; i++)
        {
            star[i].position[0] = (random_float() * 2.0f - 1.0f) * 100.0f;
            star[i].position[1] = (random_float() * 2.0f - 1.0f) * 100.0f;
            star[i].position[2] = random_float();
            star[i].color[0] = 0.8f + random_float() * 0.2f;
            star[i].color[1] = 0.8f + random_float() * 0.2f;
            star[i].color[2] = 0.8f + random_float() * 0.2f;
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(star_t), NULL);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(star_t), (void *)sizeof(vmath::vec3));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
    }

    void render(double currentTime)
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat one[] = { 1.0f };
        float t = (float)currentTime;
        vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f,
                                                     1000.0f);

        t *= 0.1f;
        t -= floor(t);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, one);

        glUseProgram(render_prog);

        glUniform1f(uniforms.time, t);
        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        glBindVertexArray(star_vao);

        glEnable(GL_PROGRAM_POINT_SIZE);
        glDrawArrays(GL_POINTS, 0, NUM_STARS);
    }

protected:
    GLuint          render_prog;
    GLuint          star_texture;
    GLuint          star_vao;
    GLuint          star_buffer;

    struct
    {
        int         time;
        int         proj_matrix;
    } uniforms;
};

DECLARE_MAIN(starfield_app)
