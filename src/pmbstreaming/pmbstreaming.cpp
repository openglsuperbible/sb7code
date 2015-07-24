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

class pmbstreaming_app : public sb7::application
{
public:
    pmbstreaming_app()
        : mode(NO_SYNC),
          sync_index(0),
          stalled(false)
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

    enum
    {
        CHUNK_COUNT     = 4,
        CHUNK_SIZE      = 3 * sizeof(vmath::mat4),
        BUFFER_SIZE     = (CHUNK_SIZE * CHUNK_COUNT)
    };

    enum MODE
    {
        NO_SYNC         = 0,
        FINISH          = 1,
        ONE_SYNC        = 2,
        RINGED_SYNC     = 3,
        NUM_MODES
    } mode;

    struct MATRICES
    {
        vmath::mat4     modelview;
        vmath::mat4     projection;
    };

    sb7::object         object;
    GLuint              program;
    GLuint              buffer;
    GLuint              texture;
    GLsync              fence[CHUNK_COUNT];
    MATRICES*           vs_uniforms;
    int                 sync_index;
    bool                stalled;

    float               fps;
};

void pmbstreaming_app::init()
{
    static const char title[] = "OpenGL SuperBible - Constant Streaming";

    sb7::application::init();

    memcpy(info.title, title, sizeof(title));
}

void pmbstreaming_app::startup()
{
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);

    glBufferStorage(GL_UNIFORM_BUFFER,
                    BUFFER_SIZE,
                    nullptr,
                    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    vs_uniforms = (MATRICES*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                   0,
                                   BUFFER_SIZE,
                                   GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

    GLuint shaders[2] =
    {
        sb7::shader::load("media/shaders/pmbstreaming/pmbstreaming.vs.glsl", GL_VERTEX_SHADER),
        sb7::shader::load("media/shaders/pmbstreaming/pmbstreaming.fs.glsl", GL_FRAGMENT_SHADER)
    };

    program = sb7::program::link_from_shaders(shaders, 2, true);

    object.load("media/objects/torus_nrms_tc.sbm");

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer);

    overlay.init(128, 50);

    glActiveTexture(GL_TEXTURE0);
    texture = sb7::ktx::file::load("media/textures/pattern1.ktx");

    for (int i = 0; i < CHUNK_COUNT; i++)
    {
        fence[i] = 0;
    }
}

void pmbstreaming_app::render(double currentTime)
{
    static float lastTime = 0.0f;
    static int frames = 0;
    float nowTime = float(currentTime);
    int isSignaled;

    static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    static const GLfloat one[] = { 1.0f };

    vmath::mat4 proj_matrix = vmath::perspective(60.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1800.0f);
    vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -3.0f) *
                            vmath::rotate((float)currentTime * 43.75f, 0.0f, 1.0f, 0.0f) *
                            vmath::rotate((float)currentTime * 17.75f, 0.0f, 0.0f, 1.0f) *
                            vmath::rotate((float)currentTime * 35.3f, 1.0f, 0.0f, 0.0f);

    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glClearBufferfv(GL_COLOR, 0, black);
    glClearBufferfv(GL_DEPTH, 0, one);

    glUseProgram(program);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    if (mode == ONE_SYNC)
    {
        if (fence[0] != 0)
        {
            glGetSynciv(fence[0], GL_SYNC_STATUS, sizeof(int), nullptr, &isSignaled);
            stalled = isSignaled == GL_UNSIGNALED;
            glClientWaitSync(fence[0], 0, GL_TIMEOUT_IGNORED);
            glDeleteSync(fence[0]);
        }
    }
    else if (mode == RINGED_SYNC)
    {
        if (fence[sync_index] != 0)
        {
            glGetSynciv(fence[sync_index], GL_SYNC_STATUS, sizeof(int), nullptr, &isSignaled);
            stalled = isSignaled == GL_UNSIGNALED;
            glClientWaitSync(fence[sync_index], 0, GL_TIMEOUT_IGNORED);
            glDeleteSync(fence[sync_index]);
        }
    }

    vs_uniforms[sync_index].modelview = mv_matrix;
    vs_uniforms[sync_index].projection = proj_matrix;

    if (mode == RINGED_SYNC)
    {
        object.render(1, sync_index);
    }
    else
    {
        object.render(1, 0);
    }

    if (nowTime > (lastTime + 0.25f))
    {
        fps = float(frames) / (nowTime - lastTime);
        frames = 0;
        lastTime = nowTime;
    }

    updateOverlay();

    if (mode == FINISH)
    {
        glFinish();
        stalled = true;
    }
    else if (mode == ONE_SYNC)
    {
        fence[0] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
    else if (mode == RINGED_SYNC)
    {
        fence[sync_index] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }

    sync_index = (sync_index + 1) % CHUNK_COUNT;

    frames++;
}

void pmbstreaming_app::shutdown(void)
{
    glDeleteProgram(program);
    glDeleteBuffers(1, &buffer);
}

void pmbstreaming_app::updateOverlay()
{
    char buffer[256];
    static const char * modenames[] =
    {
        "NO_SYNC",
        "FINISH",
        "ONE_SYNC",
        "RINGED_SYNC"
    };

    overlay.clear();
    sprintf(buffer, "%2.2fms / frame (%4.2f FPS)", 1000.0f / fps, fps);
    overlay.drawText(buffer, 0, 0);
    sprintf(buffer, "MODE: %s", modenames[mode]);
    overlay.drawText(buffer, 0, 1);
    if (stalled)
    {
        overlay.drawText("STALLED", 0, 2);
    }

    overlay.draw();
}

void pmbstreaming_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'M': mode = (MODE)((mode + 1) % NUM_MODES);
                break;
        }
    }
}

DECLARE_MAIN(pmbstreaming_app)
