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
#include <sb7ktx.h>

#include <cstdio>

#define NUM_ELEMENTS 2048

static inline float random_float()
{
    static unsigned int seed = 0x13371337;

    float res;
    unsigned int tmp;

    seed *= 16807;

    tmp = seed ^ (seed >> 4) ^ (seed << 15);

    *((unsigned int *) &res) = (tmp >> 9) | 0x3F800000;

    return (res - 1.0f);
}

class prefixsum2d_app : public sb7::application
{
public:
    prefixsum2d_app()
        : prefix_sum_prog(0)
    {

    }

protected:
    void init()
    {
        static const char title[] = "OpenGL SuperBible - 2D Prefix Sum";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup();
    void render(double currentTime);
    void onKey(int key, int action);

    void prefix_sum(const float * input, float * output, int elements);

    GLuint images[3];

    void load_shaders();

    GLuint  prefix_sum_prog;
    GLuint  show_image_prog;
    GLuint  dummy_vao;
};

void prefixsum2d_app::startup()
{
    int i;

    glGenTextures(3, images);

    images[0] = sb7::ktx::file::load("media/textures/salad-gray.ktx");

    for (i = 1; i < 3; i++)
    {
        glBindTexture(GL_TEXTURE_2D, images[i]);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, NUM_ELEMENTS, NUM_ELEMENTS);
    }

    glGenVertexArrays(1, &dummy_vao);
    glBindVertexArray(dummy_vao);

    load_shaders();
}

void prefixsum2d_app::render(double currentTime)
{
    glUseProgram(prefix_sum_prog);

    glBindImageTexture(0, images[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glBindImageTexture(1, images[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    glDispatchCompute(NUM_ELEMENTS, 1, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindImageTexture(0, images[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glBindImageTexture(1, images[2], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    glDispatchCompute(NUM_ELEMENTS, 1, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindTexture(GL_TEXTURE_2D, images[2]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, images[2]);

    glUseProgram(show_image_prog);

    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glBindVertexArray(dummy_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void prefixsum2d_app::onKey(int key, int action)
{
    if (!action)
        return;

    switch (key)
    {
        case 'R':   load_shaders();
            break;
    }
}

void prefixsum2d_app::load_shaders()
{
    GLuint cs = sb7::shader::load("media/shaders/prefixsum2d/prefixsum2d.cs.glsl", GL_COMPUTE_SHADER);

    if (prefix_sum_prog)
        glDeleteProgram(prefix_sum_prog);

    prefix_sum_prog = sb7::program::link_from_shaders(&cs, 1, true);

    struct {
        GLuint  vs;
        GLuint  fs;
    } show_image_shaders;

    show_image_shaders.vs = sb7::shader::load("media/shaders/prefixsum2d/showimage.vs.glsl", GL_VERTEX_SHADER);
    show_image_shaders.fs = sb7::shader::load("media/shaders/prefixsum2d/showimage.fs.glsl", GL_FRAGMENT_SHADER);

    show_image_prog = sb7::program::link_from_shaders(&show_image_shaders.vs, 2, true);
}

void prefixsum2d_app::prefix_sum(const float * input, float * output, int elements)
{
    float f = 0.0f;
    int i;

    for (i = 0; i < elements; i++)
    {
        f += input[i];
        output[i] = f;
    }
}

DECLARE_MAIN(prefixsum2d_app);
