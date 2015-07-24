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
#include <vmath.h>
#include <sb7ktx.h>

class cubicfilter_app : public sb7::application
{
public:
    cubicfilter_app()
        : draw_tex_program(0),
          dummy_vao(0),
          paused(false)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - High Quality Texture Filtering";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup();
    void render(double currentTime);

protected:
    GLuint      draw_tex_program;
    GLuint      dummy_vao;
    GLuint      source_tex;
    bool        paused;

    void load_shaders();
    void onKey(int key, int action);
};

void cubicfilter_app::startup()
{
    load_shaders();

    glGenVertexArrays(1, &dummy_vao);
    glBindVertexArray(dummy_vao);

    source_tex = sb7::ktx::file::load("media/textures/baboon.ktx");
}

void cubicfilter_app::render(double currentTime)
{
    static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
    static const GLfloat one = 1.0f;

    int i;
    static double last_time = 0.0;
    static double total_time = 0.0;

    if (!paused)
        total_time += (currentTime - last_time);
    last_time = currentTime;

    float t = (float)total_time;

    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glClearBufferfv(GL_COLOR, 0, gray);
    glClearBufferfv(GL_DEPTH, 0, &one);

    glUseProgram(draw_tex_program);

    Sleep(20);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source_tex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void cubicfilter_app::load_shaders()
{
    GLuint shaders[2];

    if (draw_tex_program)
        glDeleteProgram(draw_tex_program);

    shaders[0] = sb7::shader::load("media/shaders/hqfilter/hqfilter.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/hqfilter/hqfilter.fs.glsl", GL_FRAGMENT_SHADER);

    draw_tex_program = sb7::program::link_from_shaders(shaders, 2, true);
}

void cubicfilter_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'R':
                load_shaders();
                break;
            default:
                break;
        }
    }
}

DECLARE_MAIN(cubicfilter_app)
