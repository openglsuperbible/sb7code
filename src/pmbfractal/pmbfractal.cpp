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
#include <sb7textoverlay.h>
#include <sb7ktx.h>

#include <math.h>
#include <omp.h>

class pmbfractal_app : public sb7::application
{
public:
    pmbfractal_app()
    {

    }

    void init();
    void startup();
    void render(double currentTime);
    void onKey(int key, int action);
    void shutdown(void);

protected:
    sb7::text_overlay   overlay;
    void updateOverlay();

    void update_fractal();

    enum
    {
        // Use a super-low resolution fractal in debug builds to
        // make the application useable.
#ifdef _DEBUG
        FRACTAL_WIDTH   = 128,
        FRACTAL_HEIGHT  = 128,
#else
        FRACTAL_WIDTH   = 512,
        FRACTAL_HEIGHT  = 512,
#endif
        BUFFER_SIZE     = (FRACTAL_WIDTH * FRACTAL_HEIGHT)
    };

    GLuint              vao;
    GLuint              program;
    GLuint              buffer;
    GLuint              texture;
    unsigned char *     mapped_buffer;

    float               fps;

    struct
    {
        vmath::vec2 C;
        vmath::vec2 offset;
        float       zoom;
    } fractparams;
};

void pmbfractal_app::init()
{
    static const char title[] = "OpenGL SuperBible - Persistent Mapped Fractal";

    sb7::application::init();

    memcpy(info.title, title, sizeof(title));
}

void pmbfractal_app::startup()
{
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);

    glBufferStorage(GL_PIXEL_UNPACK_BUFFER,
                    BUFFER_SIZE,
                    nullptr,
                    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    mapped_buffer = (unsigned char*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,
                                   0,
                                   BUFFER_SIZE,
                                   GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    GLuint shaders[2] =
    {
        sb7::shader::load("media/shaders/fsq/fsq.vs.glsl", GL_VERTEX_SHADER),
        sb7::shader::load("media/shaders/fsq/fsq.fs.glsl", GL_FRAGMENT_SHADER)
    };

    program = sb7::program::link_from_shaders(shaders, 2, true);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, FRACTAL_WIDTH, FRACTAL_HEIGHT);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    overlay.init(128, 50);

    int maxThreads = omp_get_max_threads();
    omp_set_num_threads(maxThreads);
}

void pmbfractal_app::update_fractal()
{
    const vmath::vec2 C = fractparams.C; // (0.03f, -0.2f);
    const float thresh_squared = 256.0f;
    const float zoom = fractparams.zoom;
    const vmath::vec2 offset = fractparams.offset;

#pragma omp parallel for schedule (dynamic, 16)
    for (int y = 0; y < FRACTAL_HEIGHT; y++)
    {
        for (int x = 0; x < FRACTAL_WIDTH; x++)
        {
            vmath::vec2 Z;
            Z[0] = zoom * (float(x) / float(FRACTAL_WIDTH) - 0.5f) + offset[0];
            Z[1] = zoom * (float(y) / float(FRACTAL_HEIGHT) - 0.5f) + offset[1];
            unsigned char * ptr = mapped_buffer + y * FRACTAL_WIDTH + x;

            int it;
            for (it = 0; it < 256; it++)
            {
                vmath::vec2 Z_squared;

                Z_squared[0] = Z[0] * Z[0] - Z[1] * Z[1];
                Z_squared[1] = 2.0f * Z[0] * Z[1];
                Z = Z_squared + C;

                if ((Z[0] * Z[0] + Z[1] * Z[1]) > thresh_squared)
                    break;
            }
            *ptr = it;
        }
    }
}

void pmbfractal_app::render(double currentTime)
{
    static float lastTime = 0.0f;
    static int frames = 0;
    float nowTime = float(currentTime);

    fractparams.C = vmath::vec2(1.5f - cosf(nowTime * 0.4f) * 0.5f,
                                1.5f + cosf(nowTime * 0.5f) * 0.5f) * 0.3f;
    fractparams.offset = vmath::vec2(cosf(nowTime * 0.14f),
                                     cosf(nowTime * 0.25f)) * 0.25f;
    fractparams.zoom = (sinf(nowTime) + 1.3f) * 0.7f;

    update_fractal();

    glViewport(0, 0, info.windowWidth, info.windowHeight);

    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffer);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FRACTAL_WIDTH, FRACTAL_HEIGHT, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (nowTime > (lastTime + 0.25f))
    {
        fps = float(frames) / (nowTime - lastTime);
        frames = 0;
        lastTime = nowTime;
    }

    updateOverlay();

    frames++;
}

void pmbfractal_app::shutdown(void)
{ 
    glDeleteProgram(program);
    glDeleteBuffers(1, &buffer);
}

void pmbfractal_app::updateOverlay()
{
    char buffer[256];

    overlay.clear();
    sprintf(buffer, "%2.2fms / frame (%4.2f FPS)", 1000.0f / fps, fps);
    overlay.drawText(buffer, 0, 0);
    overlay.draw();
}

void pmbfractal_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'M':
                break;
        }
    }
}

DECLARE_MAIN(pmbfractal_app)
