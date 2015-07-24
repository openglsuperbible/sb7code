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
#include <sb7color.h>
#include <sb7textoverlay.h>

struct DrawArraysIndirectCommand
{
    GLuint  count;
    GLuint  primCount;
    GLuint  first;
    GLuint  baseInstance;
};

struct MaterialProperties
{
    vmath::vec4     ambient;
    vmath::vec4     diffuse;
    vmath::vec3     specular;
    float           specular_power;
};

struct FrameUniforms
{
    vmath::mat4 view_matrix;
    vmath::mat4 proj_matrix;
    vmath::mat4 viewproj_matrix;
};

class indirectmaterial_app : public sb7::application
{
public:
    indirectmaterial_app()
        : render_program(0),
          frame_count(0),
          draws_per_frame(NUM_DRAWS)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Indirect Materials";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup();

    void render(double currentTime);

protected:
    void load_shaders();
    void onKey(int key, int action);
    void updateOverlay(double currentTime);

    GLuint              render_program;

    sb7::object         object;
    sb7::text_overlay   overlay;

    GLuint              frame_uniforms_buffer;
    GLuint              transform_buffer;
    GLuint              indirect_draw_buffer;
    GLuint              material_buffer;

    struct
    {
        GLint           time;
        GLint           view_matrix;
        GLint           proj_matrix;
        GLint           viewproj_matrix;
    } uniforms;

    enum
    {
        NUM_MATERIALS       = 100,
        NUM_DRAWS           = 16384
    };

    unsigned int        frame_count;
    unsigned int        draws_per_frame;
};

void indirectmaterial_app::startup()
{
    int i;

    load_shaders();

    object.load("media/objects/asteroids.sbm");

    glGenBuffers(1, &indirect_draw_buffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_draw_buffer);
    glBufferStorage(GL_DRAW_INDIRECT_BUFFER,
                    NUM_DRAWS * sizeof(DrawArraysIndirectCommand),
                    nullptr,
                    GL_MAP_WRITE_BIT);

    DrawArraysIndirectCommand * cmd = (DrawArraysIndirectCommand *)
        glMapBufferRange(GL_DRAW_INDIRECT_BUFFER,
                         0,
                         NUM_DRAWS * sizeof(DrawArraysIndirectCommand),
                         GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    int num_objects = object.get_sub_object_count();
    for (i = 0; i < NUM_DRAWS; i++)
    {
        object.get_sub_object_info(i % num_objects,
                                   cmd[i].first,
                                   cmd[i].count);
        cmd[i].primCount = 1;
        cmd[i].baseInstance = i % NUM_MATERIALS;
    }

    glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);

    glGenBuffers(1, &transform_buffer);
    // glBindBuffer(GL_UNIFORM_BUFFER, transform_buffer);
    // glBufferStorage(GL_UNIFORM_BUFFER, NUM_DRAWS * sizeof(vmath::mat4), nullptr, GL_MAP_WRITE_BIT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, transform_buffer);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, NUM_DRAWS * sizeof(vmath::mat4), nullptr, GL_MAP_WRITE_BIT);
    
    glGenBuffers(1, &frame_uniforms_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, frame_uniforms_buffer);
    glBufferStorage(GL_UNIFORM_BUFFER, sizeof(FrameUniforms), nullptr, GL_MAP_WRITE_BIT);

    glGenBuffers(1, &material_buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, material_buffer);
    glBufferStorage(GL_UNIFORM_BUFFER, NUM_MATERIALS * sizeof(MaterialProperties), nullptr, GL_MAP_WRITE_BIT);

    MaterialProperties* pMaterial = (MaterialProperties*)
        glMapBufferRange(GL_UNIFORM_BUFFER,
                         0,
                         NUM_MATERIALS * sizeof(MaterialProperties),
                         GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    for (i = 0; i < NUM_MATERIALS; i++)
    {
        float f = float(i) / float(NUM_MATERIALS);
        pMaterial[i].ambient = (vmath::vec4(sinf(f * 3.7f), sinf(f * 5.7f + 3.0f), sinf(f * 4.3f + 2.0f), 1.0f) + vmath::vec4(1.0f, 1.0f, 2.0f, 0.0f)) * 0.1f;
        pMaterial[i].diffuse = (vmath::vec4(sinf(f * 9.9f + 6.0f), sinf(f * 3.1f + 2.5f), sinf(f * 7.2f + 9.0f), 1.0f) + vmath::vec4(1.0f, 2.0f, 2.0f, 0.0f)) * 0.4f;
        pMaterial[i].specular = (vmath::vec3(sinf(f * 1.6f + 4.0f), sinf(f * 0.8f + 2.7f), sinf(f * 5.2f + 8.0f)) + vmath::vec3(19.0f, 19.0f, 19.0f)) * 0.6f;
        pMaterial[i].specular_power = 200.0f + sinf(f) * 50.0f;
    }

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    glBindVertexArray(object.get_vao());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);

    overlay.init(80, 50);
}

void indirectmaterial_app::render(double currentTime)
{
    float t = float(currentTime);
    int i;

    const vmath::mat4 view_matrix = vmath::lookat(vmath::vec3(30.0f * cosf(t * 0.023f), 30.0f * cosf(t * 0.023f), 30.0f * sinf(t * 0.037f) - 200.0f),
                                                  vmath::vec3(0.0f, 0.0f, 0.0f),
                                                  vmath::normalize(vmath::vec3(0.1f - cosf(t * 0.1f) * 0.3f, 1.0f, 0.0f)));
    const vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                       (float)info.windowWidth / (float)info.windowHeight,
                                                       1.0f,
                                                       2000.0f);

    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glClearBufferfv(GL_COLOR, 0, sb7::color::Black);
    glClearBufferfv(GL_DEPTH, 0, sb7::color::White);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, frame_uniforms_buffer);
    FrameUniforms* pUniforms = (FrameUniforms*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(FrameUniforms), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    pUniforms->proj_matrix = proj_matrix;
    pUniforms->view_matrix = view_matrix;
    pUniforms->viewproj_matrix = view_matrix * proj_matrix;
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    vmath::mat4* pModelMatrices = (vmath::mat4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_DRAWS * sizeof(vmath::mat4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, transform_buffer);

    float f = t * 0.1f;
    for (i = 0; i < draws_per_frame; i++)
    {
        vmath::mat4 m = vmath::translate(sinf(f * 7.3f) * 70.0f, sinf(f * 3.7f + 2.0f) * 70.0f, sinf(f * 2.9f + 8.0f) * 70.0f) *
                        vmath::rotate(f * 330.0f, f * 490.0f, f * 250.0f);
        pModelMatrices[i] = m;
        f += 3.1f;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glBindBufferBase(GL_UNIFORM_BUFFER, 2, material_buffer);

    glUseProgram(render_program);
    glBindVertexArray(object.get_vao());
    glMultiDrawArraysIndirect(GL_TRIANGLES, nullptr, draws_per_frame, 0);

    frame_count++;

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    updateOverlay(currentTime);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void indirectmaterial_app::updateOverlay(double currentTime)
{
    static unsigned int frames = 1;
    static float deltaTime = 0.1f;
    static float lastTime = 0.0f;
    static unsigned int last_frames;
    float floatTime = float(currentTime);
    char buffer[1024];

    overlay.clear();

    sprintf(buffer, "Time = %2.2f", floatTime);
    overlay.drawText(buffer, 0, 0);
    sprintf(buffer, "Frames rendered = %u", frame_count);
    overlay.drawText(buffer, 0, 1);

    if (floatTime >= (lastTime + 1.0f))
    {
        frames = frame_count - last_frames;
        deltaTime = floatTime - lastTime;
        last_frames = frame_count;
        lastTime = floatTime;
    }

    sprintf(buffer, "Frame time = %2.2fms (%2.2ffps)", 1000.0f * deltaTime / float(frames), float(frames) / deltaTime);
    overlay.drawText(buffer, 0, 2);
    sprintf(buffer, "%u draws / frame (%2.2f draws / second)", draws_per_frame, float(draws_per_frame * frames) / deltaTime);
    overlay.drawText(buffer, 0, 3);

    overlay.draw();
}

void indirectmaterial_app::load_shaders()
{
    GLuint shaders[2];

    shaders[0] = sb7::shader::load("media/shaders/indirectmaterial/render.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/indirectmaterial/render.fs.glsl", GL_FRAGMENT_SHADER);

    if (render_program)
        glDeleteProgram(render_program);

    render_program = sb7::program::link_from_shaders(shaders, 2, true);
}

void indirectmaterial_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'A':
                draws_per_frame += 512;
                if (draws_per_frame >= NUM_DRAWS)
                {
                    draws_per_frame = NUM_DRAWS;
                }
                break;
            case 'Z':
                draws_per_frame -= 512;
                if (draws_per_frame >= NUM_DRAWS)
                {
                    draws_per_frame = 0;
                }
                break;
            case 'R':
                load_shaders();
                break;
        }
    }
}

DECLARE_MAIN(indirectmaterial_app)
