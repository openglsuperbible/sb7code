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

static void print_program_log(GLuint program)
{
    std::string str;
    GLint len;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
    str.resize(len);
    glGetProgramInfoLog(program, len, NULL, &str[0]);

#ifdef _WIN32
    OutputDebugStringA(str.c_str());
#endif
}

enum
{
    WORKGROUP_SIZE  = 256,
    NUM_WORKGROUPS  = 64,
    FLOCK_SIZE      = (NUM_WORKGROUPS * WORKGROUP_SIZE)
};

class csflocking_app : public sb7::application
{
public:
    csflocking_app()
        : frame_index(0),
          flock_update_program(0),
          flock_render_program(0)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Compute Shader Flocking";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        // This is position and normal data for a paper airplane
        static const vmath::vec3 geometry[] =
        {
            // Positions
            vmath::vec3(-5.0f, 1.0f, 0.0f),
            vmath::vec3(-1.0f, 1.5f, 0.0f),
            vmath::vec3(-1.0f, 1.5f, 7.0f),
            vmath::vec3(0.0f, 0.0f, 0.0f),
            vmath::vec3(0.0f, 0.0f, 10.0f),
            vmath::vec3(1.0f, 1.5f, 0.0f),
            vmath::vec3(1.0f, 1.5f, 7.0f),
            vmath::vec3(5.0f, 1.0f, 0.0f),

            // Normals
            vmath::vec3(0.0f),
            vmath::vec3(0.0f),
            vmath::vec3(0.107f, -0.859f, 0.00f),
            vmath::vec3(0.832f, 0.554f, 0.00f),
            vmath::vec3(-0.59f, -0.395f, 0.00f),
            vmath::vec3(-0.832f, 0.554f, 0.00f),
            vmath::vec3(0.295f, -0.196f, 0.00f),
            vmath::vec3(0.124f, 0.992f, 0.00f),
        };

        load_shaders();

        glGenBuffers(2, flock_buffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, flock_buffer[0]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, flock_buffer[1]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, FLOCK_SIZE * sizeof(flock_member), NULL, GL_DYNAMIC_COPY);

        int i;

        glGenBuffers(1, &geometry_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), geometry, GL_STATIC_DRAW);

        glGenVertexArrays(2, flock_render_vao);

        for (i = 0; i < 2; i++)
        {
            glBindVertexArray(flock_render_vao[i]);
            glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)(8 * sizeof(vmath::vec3)));

            glBindBuffer(GL_ARRAY_BUFFER, flock_buffer[i]);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(flock_member), NULL);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(flock_member), (void *)sizeof(vmath::vec4));
            glVertexAttribDivisor(2, 1);
            glVertexAttribDivisor(3, 1);

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
        }

        glBindBuffer(GL_ARRAY_BUFFER, flock_buffer[0]);
        flock_member * ptr = reinterpret_cast<flock_member *>(glMapBufferRange(GL_ARRAY_BUFFER, 0, FLOCK_SIZE * sizeof(flock_member), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));

        for (i = 0; i < FLOCK_SIZE; i++)
        {
            ptr[i].position = (vmath::vec3::random() - vmath::vec3(0.5f)) * 300.0f;
            ptr[i].velocity = (vmath::vec3::random() - vmath::vec3(0.5f));
        }

        glUnmapBuffer(GL_ARRAY_BUFFER);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    void render(double T)
    {
        float t = (float)T;
        static const float black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const float one = 1.0f;

        glUseProgram(flock_update_program);

        vmath::vec3 goal = vmath::vec3(sinf(t * 0.34f),
                                       cosf(t * 0.29f),
                                       sinf(t * 0.12f) * cosf(t * 0.5f));

        goal = goal * vmath::vec3(35.0f, 25.0f, 60.0f);

        glUniform3fv(uniforms.update.goal, 1, goal);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, flock_buffer[frame_index]);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flock_buffer[frame_index ^ 1]);

        glDispatchCompute(NUM_WORKGROUPS, 1, 1);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(flock_render_program);

        vmath::mat4 mv_matrix = vmath::lookat(vmath::vec3(0.0f, 0.0f, -400.0f),
                                              vmath::vec3(0.0f, 0.0f, 0.0f),
                                              vmath::vec3(0.0f, 1.0f, 0.0f));
        vmath::mat4 proj_matrix = vmath::perspective(60.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f,
                                                     3000.0f);
        vmath::mat4 mvp = proj_matrix * mv_matrix;

        glUniformMatrix4fv(uniforms.render.mvp, 1, GL_FALSE, mvp);

        glBindVertexArray(flock_render_vao[frame_index]);

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 8, FLOCK_SIZE);

        frame_index ^= 1;
    }

    void load_shaders()
    {
        if (flock_update_program)
            glDeleteProgram(flock_update_program);

        if (flock_render_program)
            glDeleteProgram(flock_render_program);

        struct {
            GLuint vs;
            GLuint fs;
            GLuint cs;
        } shaders;

        shaders.cs = sb7::shader::load("media/shaders/flocking/flocking.cs.glsl", GL_COMPUTE_SHADER);

        flock_update_program = sb7::program::link_from_shaders(&shaders.cs, 1, true);

        uniforms.update.goal = glGetUniformLocation(flock_update_program, "goal");

        shaders.vs = sb7::shader::load("media/shaders/flocking/render.vs.glsl", GL_VERTEX_SHADER);
        shaders.fs = sb7::shader::load("media/shaders/flocking/render.fs.glsl", GL_FRAGMENT_SHADER);

        flock_render_program = sb7::program::link_from_shaders(&shaders.vs, 2, true);

        uniforms.render.mvp = glGetUniformLocation(flock_render_program, "mvp");
    }

    void onKey(int key, int action)
    {
        if (action)
        {
            switch (key)
            {
                case 'R': 
                    load_shaders();
                    break;
            }
        }
    }

private:
    GLuint      flock_update_program;
    GLuint      flock_render_program;

    GLuint      flock_buffer[2];

    GLuint      flock_render_vao[2];
    GLuint      geometry_buffer;

    struct flock_member
    {
        vmath::vec3 position;
        unsigned int : 32;
        vmath::vec3 velocity;
        unsigned int : 32;
    };

    struct
    {
        struct
        {
            GLint       goal;
        } update;
        struct
        {
            GLuint      mvp;
        } render;
    } uniforms;

    GLuint      frame_index;
};

DECLARE_MAIN(csflocking_app)
