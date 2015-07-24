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
#include <sb7ktx.h>
#include <shader.h>

class fragmentlist_app : public sb7::application
{
public:
    fragmentlist_app()
        : clear_program(0),
          append_program(0),
          resolve_program(0)
    {
    }

protected:
    void init();
    void startup();
    void render(double currentTime);
    void onKey(int key, int action);

    void load_shaders();

    GLuint          clear_program;
    GLuint          append_program;
    GLuint          resolve_program;

    struct
    {
        GLuint      color;
        GLuint      normals;
    } textures;

    struct uniforms_block
    {
        vmath::mat4     mv_matrix;
        vmath::mat4     view_matrix;
        vmath::mat4     proj_matrix;
    };

    GLuint          uniforms_buffer;

    struct
    {
        GLint           mvp;
    } uniforms;

    sb7::object     object;

    GLuint          fragment_buffer;
    GLuint          head_pointer_image;
    GLuint          atomic_counter_buffer;
    GLuint          dummy_vao;
};

void fragmentlist_app::init()
{
    static const char title[] = "OpenGL SuperBible - Fragment List";

    sb7::application::init();

    memcpy(info.title, title, sizeof(title));
}

void fragmentlist_app::startup()
{
    load_shaders();

    glGenBuffers(1, &uniforms_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, uniforms_buffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(uniforms_block), NULL, GL_DYNAMIC_DRAW);

    object.load("media/objects/dragon.sbm");

    glGenBuffers(1, &fragment_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, fragment_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1024 * 1024 * 16, NULL, GL_DYNAMIC_COPY);

    glGenBuffers(1, &atomic_counter_buffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, atomic_counter_buffer);
    glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, NULL, GL_DYNAMIC_COPY);

    glGenTextures(1, &head_pointer_image);
    glBindTexture(GL_TEXTURE_2D, head_pointer_image);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32UI, 1024, 1024);

    glGenVertexArrays(1, &dummy_vao);
    glBindVertexArray(dummy_vao);
}

void fragmentlist_app::render(double currentTime)
{
    static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
    static const GLfloat ones[] = { 1.0f };
    const float f = (float)currentTime;

    glViewport(0, 0, info.windowWidth, info.windowHeight);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(clear_program);
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glUseProgram(append_program);

    vmath::mat4 model_matrix = vmath::scale(7.0f);
    vmath::vec3 view_position = vmath::vec3(cosf(f * 0.35f) * 120.0f, cosf(f * 0.4f) * 30.0f, sinf(f * 0.35f) * 120.0f);
    vmath::mat4 view_matrix = vmath::lookat(view_position,
                                            vmath::vec3(0.0f, 30.0f, 0.0f),
                                            vmath::vec3(0.0f, 1.0f, 0.0f));

    vmath::mat4 mv_matrix = view_matrix * model_matrix;
    vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                 (float)info.windowWidth / (float)info.windowHeight,
                                                 0.1f,
                                                 1000.0f);

    glUniformMatrix4fv(uniforms.mvp, 1, GL_FALSE, proj_matrix * mv_matrix);

    static const unsigned int zero = 0;
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomic_counter_buffer);
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(zero), &zero);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, fragment_buffer);

    glBindImageTexture(0, head_pointer_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    object.render();

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(resolve_program);

    glBindVertexArray(dummy_vao);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void fragmentlist_app::onKey(int key, int action)
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

void fragmentlist_app::load_shaders()
{
    GLuint  shaders[2];

    shaders[0] = sb7::shader::load("media/shaders/fragmentlist/clear.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/fragmentlist/clear.fs.glsl", GL_FRAGMENT_SHADER);

    if (clear_program)
        glDeleteProgram(clear_program);

    clear_program = sb7::program::link_from_shaders(shaders, 2, true);

    shaders[0] = sb7::shader::load("media/shaders/fragmentlist/append.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/fragmentlist/append.fs.glsl", GL_FRAGMENT_SHADER);

    if (append_program)
        glDeleteProgram(append_program);

    append_program = sb7::program::link_from_shaders(shaders, 2, true);

    uniforms.mvp = glGetUniformLocation(append_program, "mvp");

    shaders[0] = sb7::shader::load("media/shaders/fragmentlist/resolve.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/fragmentlist/resolve.fs.glsl", GL_FRAGMENT_SHADER);

    if (resolve_program)
        glDeleteProgram(resolve_program);

    resolve_program = sb7::program::link_from_shaders(shaders, 2, true);
}

DECLARE_MAIN(fragmentlist_app)
