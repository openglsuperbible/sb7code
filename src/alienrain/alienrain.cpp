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

class alienrain_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Alien Rain";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup()
    {
        GLuint  vs, fs;

        static const char * vs_source[] =
        {
            "#version 410 core                                                      \n"
            "                                                                       \n"
            "layout (location = 0) in int alien_index;                              \n"
            "                                                                       \n"
            "out VS_OUT                                                             \n"
            "{                                                                      \n"
            "    flat int alien;                                                    \n"
            "    vec2 tc;                                                           \n"
            "} vs_out;                                                              \n"
            "                                                                       \n"
            "struct droplet_t                                                       \n"
            "{                                                                      \n"
            "    float x_offset;                                                    \n"
            "    float y_offset;                                                    \n"
            "    float orientation;                                                 \n"
            "    float unused;                                                      \n"
            "};                                                                     \n"
            "                                                                       \n"
            "layout (std140) uniform droplets                                       \n"
            "{                                                                      \n"
            "    droplet_t droplet[256];                                            \n"
            "};                                                                     \n"
            "                                                                       \n"
            "void main(void)                                                        \n"
            "{                                                                      \n"
            "    const vec2[4] position = vec2[4](vec2(-0.5, -0.5),                 \n"
            "                                     vec2( 0.5, -0.5),                 \n"
            "                                     vec2(-0.5,  0.5),                 \n"
            "                                     vec2( 0.5,  0.5));                \n"
            "    vs_out.tc = position[gl_VertexID].xy + vec2(0.5);                  \n"
            "    float co = cos(droplet[alien_index].orientation);                  \n"
            "    float so = sin(droplet[alien_index].orientation);                  \n"
            "    mat2 rot = mat2(vec2(co, so),                                      \n"
            "                    vec2(-so, co));                                    \n"
            "    vec2 pos = 0.25 * rot * position[gl_VertexID];                     \n"
            "    gl_Position = vec4(pos.x + droplet[alien_index].x_offset,          \n"
            "                       pos.y + droplet[alien_index].y_offset,          \n"
            "                       0.5, 1.0);                                      \n"
            "    vs_out.alien = alien_index % 64;                                   \n"
            "}                                                                      \n"
        };

        static const char * fs_source[] =
        {
            "#version 410 core                                                      \n"
            "                                                                       \n"
            "layout (location = 0) out vec4 color;                                  \n"
            "                                                                       \n"
            "in VS_OUT                                                              \n"
            "{                                                                      \n"
            "    flat int alien;                                                    \n"
            "    vec2 tc;                                                           \n"
            "} fs_in;                                                               \n"
            "                                                                       \n"
            "uniform sampler2DArray tex_aliens;                                     \n"
            "                                                                       \n"
            "void main(void)                                                        \n"
            "{                                                                      \n"
            "    color = texture(tex_aliens, vec3(fs_in.tc, float(fs_in.alien)));   \n"
            "}                                                                      \n"
        };

        char buffer[1024];

        vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vs_source, NULL);
        glCompileShader(vs);

        glGetShaderInfoLog(vs, 1024, NULL, buffer);

        fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        glGetShaderInfoLog(vs, 1024, NULL, buffer);

        render_prog = glCreateProgram();
        glAttachShader(render_prog, vs);
        glAttachShader(render_prog, fs);
        glLinkProgram(render_prog);

        glDeleteShader(vs);
        glDeleteShader(fs);

        glGenVertexArrays(1, &render_vao);
        glBindVertexArray(render_vao);

        tex_alien_array = sb7::ktx::file::load("media/textures/aliens.ktx");
        glBindTexture(GL_TEXTURE_2D_ARRAY, tex_alien_array);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glGenBuffers(1, &rain_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, rain_buffer);
        glBufferData(GL_UNIFORM_BUFFER, 256 * sizeof(vmath::vec4), NULL, GL_DYNAMIC_DRAW);

        for (int i = 0; i < 256; i++)
        {
            droplet_x_offset[i] = random_float() * 2.0f - 1.0f;
            droplet_rot_speed[i] = (random_float() + 0.5f) * ((i & 1) ? -3.0f : 3.0f);
            droplet_fall_speed[i] = random_float() + 0.2f;
        }

        glBindVertexArray(render_vao);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void render(double currentTime)
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        float t = (float)currentTime;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);

        glUseProgram(render_prog);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, rain_buffer);
        vmath::vec4 * droplet = (vmath::vec4 *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, 256 * sizeof(vmath::vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (int i = 0; i < 256; i++)
        {
            droplet[i][0] = droplet_x_offset[i];
            droplet[i][1] = 2.0f - fmodf((t + float(i)) * droplet_fall_speed[i], 4.31f);
            droplet[i][2] = t * droplet_rot_speed[i];
            droplet[i][3] = 0.0f;
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        int alien_index;
        for (alien_index = 0; alien_index < 256; alien_index++)
        {
            glVertexAttribI1i(0, alien_index);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

protected:
    GLuint          render_prog;
    GLuint          render_vao;

    GLuint          tex_alien_array;
    GLuint          rain_buffer;

    float           droplet_x_offset[256];
    float           droplet_rot_speed[256];
    float           droplet_fall_speed[256];
};

DECLARE_MAIN(alienrain_app)

