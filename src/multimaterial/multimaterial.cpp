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
#include <shader.h>

#include <cstdio>

#define NUM_PASSES 200
#define NUM_CUBES 256

class multimaterial_app : public sb7::application
{
public:
    multimaterial_app()
        : mode(BIG_UBO_INDIRECT),
          draw_triangles(true),
          paused(false)
    {

    }

    struct transform_t
    {
        vmath::mat4 mv_matrix;
        vmath::mat4 proj_matrix;
    };

    transform_t transforms[NUM_CUBES];

    virtual void startup()
    {
        load_shaders();

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        static const GLfloat vertex_positions[] =
        {
            -0.25f,  0.25f, -0.25f,
            -0.25f, -0.25f, -0.25f,
             0.25f, -0.25f, -0.25f,

             0.25f, -0.25f, -0.25f,
             0.25f,  0.25f, -0.25f,
            -0.25f,  0.25f, -0.25f,

             0.25f, -0.25f, -0.25f,
             0.25f, -0.25f,  0.25f,
             0.25f,  0.25f, -0.25f,

             0.25f, -0.25f,  0.25f,
             0.25f,  0.25f,  0.25f,
             0.25f,  0.25f, -0.25f,

             0.25f, -0.25f,  0.25f,
            -0.25f, -0.25f,  0.25f,
             0.25f,  0.25f,  0.25f,

            -0.25f, -0.25f,  0.25f,
            -0.25f,  0.25f,  0.25f,
             0.25f,  0.25f,  0.25f,

            -0.25f, -0.25f,  0.25f,
            -0.25f, -0.25f, -0.25f,
            -0.25f,  0.25f,  0.25f,

            -0.25f, -0.25f, -0.25f,
            -0.25f,  0.25f, -0.25f,
            -0.25f,  0.25f,  0.25f,

            -0.25f, -0.25f,  0.25f,
             0.25f, -0.25f,  0.25f,
             0.25f, -0.25f, -0.25f,

             0.25f, -0.25f, -0.25f,
            -0.25f, -0.25f, -0.25f,
            -0.25f, -0.25f,  0.25f,

            -0.25f,  0.25f, -0.25f,
             0.25f,  0.25f, -0.25f,
             0.25f,  0.25f,  0.25f,

             0.25f,  0.25f,  0.25f,
            -0.25f,  0.25f,  0.25f,
            -0.25f,  0.25f, -0.25f
        };

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vertex_positions),
                     vertex_positions,
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glGenBuffers(1, &transform_ubo);
        glBindBuffer(GL_UNIFORM_BUFFER, transform_ubo);
        glBufferData(GL_UNIFORM_BUFFER, 256 * sizeof(transform_t), NULL, GL_DYNAMIC_DRAW);

        struct draw_indirect_cmd_t
        {
            unsigned int count;
            unsigned int primCount;
            unsigned int first;
            unsigned int baseInstance;
        };

        glGenBuffers(1, &indirect_buffer);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, NUM_CUBES * sizeof(draw_indirect_cmd_t), NULL, GL_STATIC_DRAW);
        draw_indirect_cmd_t * ind = (draw_indirect_cmd_t *)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, NUM_CUBES * sizeof(draw_indirect_cmd_t), GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_WRITE_BIT);
        int i;
        for (i = 0; i < NUM_CUBES; i++)
        {
            ind[i].count = 36;
            ind[i].primCount = 1;
            ind[i].first = 0;
            ind[i].baseInstance = i;
        }
        glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
    }

    virtual void render(double currentTime)
    {
        static double last_time = 0.0;
        static double total_time = 0.0;
        static unsigned int q = 0;

        if (!paused)
            total_time += (currentTime - last_time);
        last_time = currentTime;

        switch (mode)
        {
            case SIMPLE_UNIFORM:
                render_simple_uniform(total_time);
                break;
            case BIG_UBO_WITH_UNIFORM:
                render_big_ubo_plus_uniform(total_time);
                break;
            case BIG_UBO_WITH_BASEVERTEX:
                render_big_ubo_base_vertex(total_time);
                break;
            case BIG_UBO_WITH_INSTANCING:
                render_big_ubo_with_instancing(total_time);
                break;
            case BIG_UBO_INDIRECT:
                render_big_ubo_indirect(total_time);
                break;
        };
    }

    void render_simple_uniform(double currentTime)
    {
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        float f = (float)currentTime * 0.3f;
        int i;
        int j;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, green);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(simple_uniform_program);

        for (i = 0; i < NUM_CUBES; i++)
        {
            float fi = 4.0f * (float)i / (float)NUM_CUBES;
            fi = 0.0f;
            transforms[i].proj_matrix = vmath::perspective(50.0f,
                                                           (float)info.windowWidth / (float)info.windowHeight,
                                                           0.1f,
                                                           1000.0f);
            transforms[i].mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f) *
                                      vmath::translate(sinf(5.1f * f + fi) * 1.0f,
                                                       cosf(7.7f * f + fi) * 1.0f,
                                                       sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f) *
                                      vmath::rotate(f * 45.0f + fi, 0.0f, 1.0f, 0.0f) *
                                      vmath::rotate(f * 81.0f + fi, 1.0f, 0.0f, 0.0f);
        }

        for (j = 0; j < NUM_PASSES; j++)
        {
            for (i = 0; i < NUM_CUBES; i++)
            {
                glUniformMatrix4fv(uniforms.simple_uniforms.proj_location, 1, GL_FALSE, transforms[i].proj_matrix);
                glUniformMatrix4fv(uniforms.simple_uniforms.mv_location, 1, GL_FALSE, transforms[i].mv_matrix);
                glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, 1, 0);
            }
        }
    }

    void render_big_ubo_plus_uniform(double currentTime)
    {
        static const GLfloat blue[] = { 0.0f, 0.0f, 0.25f, 1.0f };
        static const GLfloat one = 1.0f;
        float f = (float)currentTime * 0.3f;
        int i;
        int j;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, blue);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(ubo_plus_uniform_program);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);
        transform_t * transform = (transform_t *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, NUM_CUBES * sizeof(transform_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (i = 0; i < NUM_CUBES; i++)
        {
            float fi = 4.0f * (float)i / (float)NUM_CUBES;
            fi = 0.0f;
            transform[i].proj_matrix = vmath::perspective(50.0f,
                                                          (float)info.windowWidth / (float)info.windowHeight,
                                                          0.1f,
                                                          1000.0f);
            transform[i].mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f) *
                                     vmath::translate(sinf(5.1f * f + fi) * 1.0f,
                                                      cosf(7.7f * f + fi) * 1.0f,
                                                      sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f) *
                                     vmath::rotate(f * 45.0f + fi, 0.0f, 1.0f, 0.0f) *
                                     vmath::rotate(f * 81.0f + fi, 1.0f, 0.0f, 0.0f);
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        for (j = 0; j < NUM_PASSES; j++)
        {
            for (i = 0; i < NUM_CUBES; i++)
            {
                glUniform1i(uniforms.ubo_plus_uniform.transform_index, i);
                glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, 1, 0);
            }
        }
    }

    void render_big_ubo_base_vertex(double currentTime)
    {
        static const GLfloat red[] = { 0.25f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        int i;
        int j;
        float f = (float)currentTime * 0.3f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, red);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(ubo_plus_base_instance_program);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);
        transform_t * transform = (transform_t *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, NUM_CUBES * sizeof(transform_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (i = 0; i < NUM_CUBES; i++)
        {
            float fi = 4.0f * (float)i / (float)NUM_CUBES;
            fi = 0.0f;
            transform[i].proj_matrix = vmath::perspective(50.0f,
                                                          (float)info.windowWidth / (float)info.windowHeight,
                                                          0.1f,
                                                          1000.0f);
            transform[i].mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f) *
                                     vmath::translate(sinf(5.1f * f + fi) * 1.0f,
                                                      cosf(7.7f * f + fi) * 1.0f,
                                                      sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f) *
                                     vmath::rotate(f * 45.0f + fi, 0.0f, 1.0f, 0.0f) *
                                     vmath::rotate(f * 81.0f + fi, 1.0f, 0.0f, 0.0f);
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        for (j = 0; j < NUM_PASSES; j++)
        {
            for (i = 0; i < NUM_CUBES; i++)
            {
                glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, 1, i);
            }
        }
    }

    void render_big_ubo_with_instancing(double currentTime)
    {
        static const GLfloat yellow[] = { 0.25f, 0.25f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        int i;
        int j;
        float f = (float)currentTime * 0.3f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, yellow);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(ubo_plus_base_instance_program);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);
        transform_t * transform = (transform_t *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, NUM_CUBES * sizeof(transform_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (i = 0; i < NUM_CUBES; i++)
        {
            float fi = 4.0f * (float)i / (float)NUM_CUBES;
            fi = 0.0f;
            transform[i].proj_matrix = vmath::perspective(50.0f,
                                                          (float)info.windowWidth / (float)info.windowHeight,
                                                          0.1f,
                                                          1000.0f);
            transform[i].mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f) *
                                     vmath::translate(sinf(5.1f * f + fi) * 1.0f,
                                                      cosf(7.7f * f + fi) * 1.0f,
                                                      sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f) *
                                     vmath::rotate(f * 45.0f + fi, 0.0f, 1.0f, 0.0f) *
                                     vmath::rotate(f * 81.0f + fi, 1.0f, 0.0f, 0.0f);
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        for (j = 0; j < NUM_PASSES; j++)
        {
            glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, NUM_CUBES, 0);
        }
    }

    void render_big_ubo_indirect(double currentTime)
    {
        static const GLfloat purple[] = { 0.25f, 0.0f, 0.25f, 1.0f };
        static const GLfloat one = 1.0f;
        int i;
        int j;
        float f = (float)currentTime * 0.3f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, purple);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(ubo_plus_base_instance_program);

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, transform_ubo);
        transform_t * transform = (transform_t *)glMapBufferRange(GL_UNIFORM_BUFFER, 0, NUM_CUBES * sizeof(transform_t), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (i = 0; i < NUM_CUBES; i++)
        {
            float fi = 4.0f * (float)i / (float)NUM_CUBES;
            fi = 0.0f;
            transform[i].proj_matrix = vmath::perspective(50.0f,
                                                          (float)info.windowWidth / (float)info.windowHeight,
                                                          0.1f,
                                                          1000.0f);
            transform[i].mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f) *
                                     vmath::translate(sinf(5.1f * f + fi) * 1.0f,
                                                      cosf(7.7f * f + fi) * 1.0f,
                                                      sinf(6.3f * f + fi) * cosf(1.5f * f + fi) * 2.0f) *
                                     vmath::rotate(f * 45.0f + fi, 0.0f, 1.0f, 0.0f) *
                                     vmath::rotate(f * 81.0f + fi, 1.0f, 0.0f, 0.0f);
        }
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_buffer);

        for (j = 0; j < NUM_PASSES; j++)
        {
            glMultiDrawArraysIndirect(draw_triangles ? GL_TRIANGLES : GL_POINTS, NULL, NUM_CUBES, 0);
            /*
            for (i = 0; i < NUM_CUBES; i++)
            {
                glDrawArraysInstancedBaseInstance(draw_triangles ? GL_TRIANGLES : GL_POINTS, 0, 36, 1, i);
            }
            */
        }
    }

    virtual void shutdown()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(simple_uniform_program);
        glDeleteBuffers(1, &buffer);
    }

    virtual void onKey(int key, int action)
    {
        if (action == 1)
        {
            switch (key)
            {
                case 'P':
                    paused = !paused;
                    break;
                case 'T':
                    draw_triangles = !draw_triangles;
                    break;
                case 'M':
                    mode = (MODE)(mode + 1);
                    if (mode > BIG_UBO_INDIRECT)
                        mode = SIMPLE_UNIFORM;
                    break;
                default:
                    break;
            };
        }
    }

private:
    GLuint          simple_uniform_program;
    GLuint          ubo_plus_uniform_program;
    GLuint          ubo_plus_base_instance_program;
    GLuint          vao;
    GLuint          buffer;
    struct
    {
        struct
        {
            GLint           mv_location;
            GLint           proj_location;
        } simple_uniforms;
        struct
        {
            GLint           transform_index;
        } ubo_plus_uniform;
    } uniforms;

    GLuint          transform_ubo;
    GLuint          indirect_buffer;

    void load_shaders();

    enum MODE
    {
        SIMPLE_UNIFORM,
        BIG_UBO_WITH_UNIFORM,
        BIG_UBO_WITH_BASEVERTEX,
        BIG_UBO_WITH_INSTANCING,
        BIG_UBO_INDIRECT,
    } mode;
    bool            draw_triangles;
    bool            paused;
};

void multimaterial_app::load_shaders()
{
    GLuint shaders[4];

    shaders[0] = sb7::shader::load("media/shaders/multimaterial/simpleuniforms.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/multimaterial/simpleuniforms.fs.glsl", GL_FRAGMENT_SHADER);

    simple_uniform_program = sb7::program::link_from_shaders(shaders, 2, true);

    uniforms.simple_uniforms.mv_location = glGetUniformLocation(simple_uniform_program, "mv_matrix");
    uniforms.simple_uniforms.proj_location = glGetUniformLocation(simple_uniform_program, "proj_matrix");

    shaders[0] = sb7::shader::load("media/shaders/multimaterial/ubo-plus-uniform.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/multimaterial/ubo-plus-uniform.fs.glsl", GL_FRAGMENT_SHADER);

    ubo_plus_uniform_program = sb7::program::link_from_shaders(shaders, 2, true);

    uniforms.ubo_plus_uniform.transform_index = glGetUniformLocation(ubo_plus_uniform_program, "transform_index");

    shaders[0] = sb7::shader::load("media/shaders/multimaterial/ubo-plus-base-instance.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/multimaterial/ubo-plus-base-instance.fs.glsl", GL_FRAGMENT_SHADER);

    ubo_plus_base_instance_program = sb7::program::link_from_shaders(shaders, 2, true);
}

DECLARE_MAIN(multimaterial_app)
