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
#include <shader.h>
#include <object.h>
#include <vmath.h>

enum
{
    NUM_DRAWS           = 50000
};

struct DrawArraysIndirectCommand
{
    GLuint  count;
    GLuint  primCount;
    GLuint  first;
    GLuint  baseInstance;
};

class multidrawindirect_app : public sb7::application
{
public:
    multidrawindirect_app()
        : render_program(0),
          mode(MODE_MULTIDRAW),
          paused(false),
          vsync(false)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Asteroids";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup();

    void render(double currentTime);

protected:
    void load_shaders();
    void onKey(int key, int action);

    GLuint              render_program;

    sb7::object         object;

    GLuint              indirect_draw_buffer;
    GLuint              draw_index_buffer;

    struct
    {
        GLint           time;
        GLint           view_matrix;
        GLint           proj_matrix;
        GLint           viewproj_matrix;
    } uniforms;

    enum MODE
    {
        MODE_FIRST,
        MODE_MULTIDRAW = 0,
        MODE_SEPARATE_DRAWS,
        MODE_MAX = MODE_SEPARATE_DRAWS
    };

    MODE                mode;
    bool                paused;
    bool                vsync;
};

void multidrawindirect_app::startup()
{
    int i;

    load_shaders();

    object.load("media/objects/asteroids.sbm");

    glGenBuffers(1, &indirect_draw_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER,
                 NUM_DRAWS * sizeof(DrawArraysIndirectCommand),
                 NULL,
                 GL_STATIC_DRAW);

    DrawArraysIndirectCommand * cmd = (DrawArraysIndirectCommand *)
        glMapBufferRange(GL_DRAW_INDIRECT_BUFFER,
                         0,
                         NUM_DRAWS * sizeof(DrawArraysIndirectCommand),
                         GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    for (i = 0; i < NUM_DRAWS; i++)
    {
        object.get_sub_object_info(i % object.get_sub_object_count(),
                                   cmd[i].first,
                                   cmd[i].count);
        cmd[i].primCount = 1;
        cmd[i].baseInstance = i;
    }

    glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);

    glBindVertexArray(object.get_vao());

    glGenBuffers(1, &draw_index_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, draw_index_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 NUM_DRAWS * sizeof(GLuint),
                 NULL,
                 GL_STATIC_DRAW);

    GLuint * draw_index =
        (GLuint *)glMapBufferRange(GL_ARRAY_BUFFER,
                                   0,
                                   NUM_DRAWS * sizeof(GLuint),
                                   GL_MAP_WRITE_BIT |
                                   GL_MAP_INVALIDATE_BUFFER_BIT);

    for (i = 0; i < NUM_DRAWS; i++)
    {
        draw_index[i] = i;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);

    glVertexAttribIPointer(10, 1, GL_UNSIGNED_INT, 0, NULL);
    glVertexAttribDivisor(10, 1);
    glEnableVertexAttribArray(10);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
}

void multidrawindirect_app::render(double currentTime)
{
    int j;
    static const float one = 1.0f;
    static const float black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

        
    static double last_time = 0.0;
    static double total_time = 0.0;

    if (!paused)
        total_time += (currentTime - last_time);
    last_time = currentTime;

    float t = float(total_time);
    int i = int(total_time * 3.0f);

    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_DEPTH, 0, &one);

    const vmath::mat4 view_matrix = vmath::lookat(vmath::vec3(100.0f * cosf(t * 0.023f), 100.0f * cosf(t * 0.023f), 300.0f * sinf(t * 0.037f) - 600.0f),
                                                  vmath::vec3(0.0f, 0.0f, 260.0f),
                                                  vmath::normalize(vmath::vec3(0.1f - cosf(t * 0.1f) * 0.3f, 1.0f, 0.0f)));
    const vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                       (float)info.windowWidth / (float)info.windowHeight,
                                                       1.0f,
                                                       2000.0f);

    glUseProgram(render_program);

    glUniform1f(uniforms.time, t);
    glUniformMatrix4fv(uniforms.view_matrix, 1, GL_FALSE, view_matrix);
    glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(uniforms.viewproj_matrix, 1, GL_FALSE, proj_matrix * view_matrix);

    glBindVertexArray(object.get_vao());

    if (mode == MODE_MULTIDRAW)
    {
        glMultiDrawArraysIndirect(GL_TRIANGLES, NULL, NUM_DRAWS, 0);
    }
    else if (mode == MODE_SEPARATE_DRAWS)
    {
        for (j = 0; j < NUM_DRAWS; j++)
        {
            GLuint first, count;
            object.get_sub_object_info(j % object.get_sub_object_count(), first, count);
            glDrawArraysInstancedBaseInstance(GL_TRIANGLES,
                                              first,
                                              count,
                                              1, j);
        }
    }
}

void multidrawindirect_app::load_shaders()
{
    GLuint shaders[2];

    shaders[0] = sb7::shader::load("media/shaders/multidrawindirect/render.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/multidrawindirect/render.fs.glsl", GL_FRAGMENT_SHADER);

    if (render_program)
        glDeleteProgram(render_program);

    render_program = sb7::program::link_from_shaders(shaders, 2, true);

    uniforms.time            = glGetUniformLocation(render_program, "time");
    uniforms.view_matrix     = glGetUniformLocation(render_program, "view_matrix");
    uniforms.proj_matrix     = glGetUniformLocation(render_program, "proj_matrix");
    uniforms.viewproj_matrix = glGetUniformLocation(render_program, "viewproj_matrix");
}

void multidrawindirect_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'P':
                paused = !paused;
                break;
            case 'V':
                vsync = !vsync;
                setVsync(vsync);
                break;
            case 'D':
                mode = MODE(mode + 1);
                if (mode > MODE_MAX)
                    mode = MODE_FIRST;
                break;
            case 'R':
                load_shaders();
                break;
        }
    }
}

DECLARE_MAIN(multidrawindirect_app)
