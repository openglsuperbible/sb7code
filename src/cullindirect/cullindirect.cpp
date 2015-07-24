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
#include <sb7color.h>

#include <math.h>
#include <omp.h>

class cullindirect_app : public sb7::application
{
public:
    cullindirect_app()
    {
        programs.cull = 0;
        programs.draw = 0;
    }

    void init();
    void startup();
    void render(double currentTime);
    void onKey(int key, int action);
    void shutdown(void);

protected:
    sb7::text_overlay   overlay;
    void updateOverlay();

    void load_shaders();

    enum
    {
        CANDIDATE_COUNT     = 1024
    };

    struct
    {
        GLuint          cull;
        GLuint          draw;
    } programs;
    struct
    {
        GLuint          parameters;
        GLuint          drawCandidates;
        GLuint          drawCommands;
        GLuint          modelMatrices;
        GLuint          transforms;
    } buffers;
    unsigned char *     mapped_buffer;

    float               fps;
    sb7::object         object;
    GLuint              texture;
};

void cullindirect_app::init()
{
    static const char title[] = "OpenGL SuperBible - Indirect Culling";

    sb7::application::init();

    memcpy(info.title, title, sizeof(title));
}

struct CandidateDraw
{
    vmath::vec3 sphereCenter;
    float sphereRadius;
    unsigned int firstVertex;
    unsigned int vertexCount;
    unsigned int : 32;
    unsigned int : 32;
};

struct DrawArraysIndirectCommand
{
    GLuint vertexCount;
    GLuint instanceCount;
    GLuint firstVertex;
    GLuint baseInstance;
};

struct TransformBuffer
{
    vmath::mat4     view_matrix;
    vmath::mat4     proj_matrix;
    vmath::mat4     view_proj_matrix;
};

void cullindirect_app::startup()
{
    GLuint first, count;

    object.load("media/objects/asteroids.sbm");

    glGenBuffers(1, &buffers.parameters);
    glBindBuffer(GL_PARAMETER_BUFFER_ARB, buffers.parameters);
    glBufferStorage(GL_PARAMETER_BUFFER_ARB, 256, nullptr, 0);

    glGenBuffers(1, &buffers.drawCandidates);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers.drawCandidates);

    CandidateDraw* pDraws = new CandidateDraw[CANDIDATE_COUNT];

    int i;

    for (i = 0; i < CANDIDATE_COUNT; i++)
    {
        object.get_sub_object_info(i % object.get_sub_object_count(), first, count);
        pDraws[i].sphereCenter = vmath::vec3(0.0f);
        pDraws[i].sphereRadius = 4.0f;
        pDraws[i].firstVertex = first;
        pDraws[i].vertexCount = count;
    }

    glBufferStorage(GL_SHADER_STORAGE_BUFFER, CANDIDATE_COUNT * sizeof(CandidateDraw), pDraws, 0);

    delete [] pDraws;

    glGenBuffers(1, &buffers.drawCommands);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers.drawCommands);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, CANDIDATE_COUNT * sizeof(DrawArraysIndirectCommand), nullptr, GL_MAP_READ_BIT);

    glGenBuffers(1, &buffers.modelMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, buffers.modelMatrices);
    glBufferStorage(GL_UNIFORM_BUFFER, 1024 * sizeof(vmath::mat4), nullptr, GL_MAP_WRITE_BIT);

    glGenBuffers(1, &buffers.transforms);
    glBindBuffer(GL_UNIFORM_BUFFER, buffers.transforms);
    glBufferStorage(GL_UNIFORM_BUFFER, sizeof(TransformBuffer), nullptr, GL_MAP_WRITE_BIT);

    load_shaders();

    overlay.init(128, 50);

    texture = sb7::ktx::file::load("media/textures/rocks.ktx");
}

void cullindirect_app::load_shaders()
{
    GLuint shaders[2];

    shaders[0] = sb7::shader::load("media/shaders/cullindirect/cull.cs.glsl", GL_COMPUTE_SHADER);

    glDeleteProgram(programs.cull);
    programs.cull = sb7::program::link_from_shaders(shaders, 1, true);

    shaders[0] = sb7::shader::load("media/shaders/cullindirect/render.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/cullindirect/render.fs.glsl", GL_FRAGMENT_SHADER);

    glDeleteProgram(programs.draw);
    programs.draw = sb7::program::link_from_shaders(shaders, 2, true);
}

void cullindirect_app::render(double currentTime)
{
    static const GLfloat farplane[] = { 1.0f };
    static float lastTime = 0.0f;
    static int frames = 0;
    float nowTime = float(currentTime);
    int i;

    // Set viewport and clear
    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glClearBufferfv(GL_COLOR, 0, sb7::color::Black);
    glClearBufferfv(GL_DEPTH, 0, farplane);

    // Bind and clear atomic counter
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffers.parameters);
    glClearBufferSubData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, 0, sizeof(GLuint), GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);

    // Bind shader storage buffers
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffers.drawCandidates);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffers.drawCommands);

    // Bind model matrix UBO and fill with data
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffers.modelMatrices);
    vmath::mat4* pModelMatrix = (vmath::mat4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, 1024 * sizeof(vmath::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    
    for (i = 0; i < 1024; i++)
    {
        float f = float(i) / 127.0f + nowTime * 0.025f;
        float g = float(i) / 127.0f;
        const vmath::mat4 model_matrix = vmath::translate(70.0f * vmath::vec3(sinf(f * 3.0f), cosf(f * 5.0f), cosf(f * 9.0f))) *
                                         vmath::rotate(nowTime * 140.0f, vmath::normalize(vmath::vec3(sinf(g * 35.0f), cosf(g * 75.0f), cosf(g * 39.0f))));
        pModelMatrix[i] = model_matrix;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    // Bind view + projection matrix UBO and fill
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, buffers.transforms);
    TransformBuffer* pTransforms = (TransformBuffer*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(TransformBuffer), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    float t = nowTime * 0.1f;

    const vmath::mat4 view_matrix = vmath::lookat(vmath::vec3(150.0f * cosf(t), 0.0f, 150.0f * sinf(t)),
                                                  vmath::vec3(0.0f, 0.0f, 0.0f),
                                                  vmath::vec3(0.0f, 1.0f, 0.0f));
    const vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                       (float)info.windowWidth / (float)info.windowHeight,
                                                       1.0f,
                                                       2000.0f);

    pTransforms->view_matrix = view_matrix;
    pTransforms->proj_matrix = proj_matrix;
    pTransforms->view_proj_matrix = proj_matrix * view_matrix;

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    // Bind the culling compute shader and dispatch it
    glUseProgram(programs.cull);
    glDispatchCompute(CANDIDATE_COUNT / 16, 1, 1);

    // Barrier
    glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

    // Get ready to render
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glBindVertexArray(object.get_vao());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Bind indirect command buffer and parameter buffer
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffers.drawCommands);
    glBindBuffer(GL_PARAMETER_BUFFER_ARB, buffers.parameters);

    glUseProgram(programs.draw);

    // Draw
    glMultiDrawArraysIndirectCountARB(GL_TRIANGLES, 0, 0, CANDIDATE_COUNT, 0);

    // Update overlay
    if (nowTime > (lastTime + 0.25f))
    {
        fps = float(frames) / (nowTime - lastTime);
        frames = 0;
        lastTime = nowTime;
    }

    glDisable(GL_CULL_FACE);
    updateOverlay();

    frames++;
}

void cullindirect_app::shutdown(void)
{ 
    glDeleteProgram(programs.cull);
    glDeleteBuffers(1, &buffers.parameters);
}

void cullindirect_app::updateOverlay()
{
    char buffer[256];

    overlay.clear();
    sprintf(buffer, "%2.2fms / frame (%4.2f FPS)", 1000.0f / fps, fps);
    overlay.drawText(buffer, 0, 0);
    overlay.draw();
}

void cullindirect_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'R': load_shaders();
                break;
        }
    }
}

DECLARE_MAIN(cullindirect_app)
