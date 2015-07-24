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
#include <sb7textoverlay.h>

#include <shader.h>

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

class prefixsum_app : public sb7::application
{
public:
    prefixsum_app()
        : prefix_sum_prog(0)
    {

    }

protected:
    void init()
    {
        static const char title[] = "OpenGL SuperBible - 1D Prefix Sum";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup();
    void render(double currentTime);
    void onKey(int key, int action);

    void prefix_sum(const float * input, float * output, int elements);

    GLuint  data_buffer[2];

    float input_data[NUM_ELEMENTS];
    float output_data[NUM_ELEMENTS];

    void load_shaders();

    GLuint  prefix_sum_prog;
    sb7::text_overlay overlay;
};

void prefixsum_app::startup()
{
    overlay.init(80, 50);

    glGenBuffers(2, data_buffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer[0]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_ELEMENTS * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, data_buffer[1]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_ELEMENTS * sizeof(float), NULL, GL_DYNAMIC_COPY);

    int i;

    for (i = 0; i < NUM_ELEMENTS; i++)
    {
        input_data[i] = random_float();
    }

    prefix_sum(input_data, output_data, NUM_ELEMENTS);

    load_shaders();
}

void prefixsum_app::render(double currentTime)
{
    float * ptr;

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, data_buffer[0], 0, sizeof(float) * NUM_ELEMENTS);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * NUM_ELEMENTS, input_data);

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, data_buffer[1], 0, sizeof(float) * NUM_ELEMENTS);

    glUseProgram(prefix_sum_prog);
    glDispatchCompute(1, 1, 1);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glFinish();

    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, data_buffer[1], 0, sizeof(float) * NUM_ELEMENTS);
    ptr = (float *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float) * NUM_ELEMENTS, GL_MAP_READ_BIT);

    char buffer[1024];
    sprintf(buffer, "SUM: %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f "
                    "%2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f %2.2f",
                    ptr[0], ptr[1], ptr[2], ptr[3], ptr[4], ptr[5], ptr[6], ptr[7],
                    ptr[8], ptr[9], ptr[10], ptr[11], ptr[12], ptr[13], ptr[14], ptr[15]);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    overlay.clear();
    overlay.drawText(buffer, 0, 0);
    overlay.draw();
}

void prefixsum_app::onKey(int key, int action)
{
    if (!action)
        return;

    switch (key)
    {
        case 'R':   load_shaders();
            break;
    }
}

void prefixsum_app::load_shaders()
{
    GLuint cs = sb7::shader::load("media/shaders/prefixsum/prefixsum.cs.glsl", GL_COMPUTE_SHADER);

    if (prefix_sum_prog)
        glDeleteProgram(prefix_sum_prog);

    prefix_sum_prog = sb7::program::link_from_shaders(&cs, 1, true);
    /*
    prefix_sum_prog = glCreateProgram();
    glAttachShader(prefix_sum_prog, cs);

    glLinkProgram(prefix_sum_prog);

    int n;
    glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &n);
    */

    glShaderStorageBlockBinding(prefix_sum_prog, 0, 0);
    glShaderStorageBlockBinding(prefix_sum_prog, 1, 1);
}

void prefixsum_app::prefix_sum(const float * input, float * output, int elements)
{
    float f = 0.0f;
    int i;

    for (i = 0; i < elements; i++)
    {
        f += input[i];
        output[i] = f;
    }
}

DECLARE_MAIN(prefixsum_app);
