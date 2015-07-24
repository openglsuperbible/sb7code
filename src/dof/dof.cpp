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
#include <sb7textoverlay.h>

#define FBO_SIZE                2048
#define FRUSTUM_DEPTH           1000

class dof_app : public sb7::application
{
public:
    dof_app()
        : display_program(0),
          filter_program(0),
          view_program(0),
          paused(false),
          focal_distance(40.0f),
          focal_depth(50.0f)
    {
    }

protected:
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Depth Of Field";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup();
    void render(double currentTime);
    void render_scene(double currentTime);
    void onKey(int key, int action);

    void load_shaders();

    GLuint          view_program;
    GLuint          filter_program;
    GLuint          display_program;

    struct
    {
        struct
        {
            GLint   focal_distance;
            GLint   focal_depth;
        } dof;
        struct
        {
            GLint   mv_matrix;
            GLint   proj_matrix;
            GLint   full_shading;
            GLint   diffuse_albedo;
        } view;
    } uniforms;

    GLuint          depth_fbo;
    GLuint          depth_tex;
    GLuint          color_tex;
    GLuint          temp_tex;

    enum { OBJECT_COUNT = 5 };
    struct
    {
        sb7::object     obj;
        vmath::mat4     model_matrix;
        vmath::vec4     diffuse_albedo;
    } objects[OBJECT_COUNT];

    vmath::mat4     camera_view_matrix;
    vmath::mat4     camera_proj_matrix;

    GLuint          quad_vao;

    bool paused;

    float          focal_distance;
    float          focal_depth;

    sb7::text_overlay   overlay;
};

void dof_app::startup()
{
    load_shaders();

    int i;

    static const char * const object_names[] =
    {
        "media/objects/dragon.sbm",
        "media/objects/sphere.sbm",
        "media/objects/cube.sbm",
        "media/objects/cube.sbm",
        "media/objects/cube.sbm",
    };

    static const vmath::vec4 object_colors[] =
    {
        vmath::vec4(1.0f, 0.7f, 0.8f, 1.0f),
        vmath::vec4(0.7f, 0.8f, 1.0f, 1.0f),
        vmath::vec4(0.3f, 0.9f, 0.4f, 1.0f),
        vmath::vec4(0.6f, 0.4f, 0.9f, 1.0f),
        vmath::vec4(0.8f, 0.2f, 0.1f, 1.0f),
    };

    for (i = 0; i < OBJECT_COUNT; i++)
    {
        objects[i].obj.load(object_names[i]);
        objects[i].diffuse_albedo = object_colors[i];
    }

    glGenFramebuffers(1, &depth_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

    glGenTextures(1, &depth_tex);
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glTexStorage2D(GL_TEXTURE_2D, 11, GL_DEPTH_COMPONENT32F, FBO_SIZE, FBO_SIZE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &color_tex);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, FBO_SIZE, FBO_SIZE);

    glGenTextures(1, &temp_tex);
    glBindTexture(GL_TEXTURE_2D, temp_tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, FBO_SIZE, FBO_SIZE);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_tex, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_tex, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);

    glGenVertexArrays(1, &quad_vao);
    glBindVertexArray(quad_vao);

    overlay.init(80, 50);
    overlay.clear();
    overlay.drawText("Q: Increas focal distance", 0, 0);
    overlay.drawText("A: Decrease focal distance", 0, 1);
    overlay.drawText("W: Increase focal depth", 0, 2);
    overlay.drawText("S: Decrease focal depth", 0, 3);
    overlay.drawText("P: Pause", 0, 4);
}

void dof_app::render(double currentTime)
{
    static const GLfloat zeros[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    
    static double last_time = 0.0;
    static double total_time = 0.0;

    if (!paused)
        total_time += (currentTime - last_time);
    last_time = currentTime;

    const float f = (float)total_time + 30.0f;

    vmath::vec3 view_position = vmath::vec3(0.0f, 0.0f, 40.0f);

    camera_proj_matrix = vmath::perspective(50.0f,
                                            (float)info.windowWidth / (float)info.windowHeight,
                                            2.0f,
                                            300.0f);

    camera_view_matrix = vmath::lookat(view_position,
                                       vmath::vec3(0.0f),
                                       vmath::vec3(0.0f, 1.0f, 0.0f));

    objects[0].model_matrix = vmath::translate(5.0f, 0.0f, 20.0f) *
                              vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
                              vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f) *
                              vmath::translate(0.0f, -4.0f, 0.0f);

    objects[1].model_matrix = vmath::translate(-5.0f, 0.0f, 0.0f) *
                              vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
                              vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f) *
                              vmath::translate(0.0f, -4.0f, 0.0f);

    objects[2].model_matrix = vmath::translate(-15.0f, 0.0f, -20.0f) *
                              vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
                              vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f) *
                              vmath::translate(0.0f, -4.0f, 0.0f);

    objects[3].model_matrix = vmath::translate(-25.0f, 0.0f, -40.0f) *
                              vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
                              vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f) *
                              vmath::translate(0.0f, -4.0f, 0.0f);

    objects[4].model_matrix = vmath::translate(-35.0f, 0.0f, -60.0f) *
                              vmath::rotate(f * 14.5f, 0.0f, 1.0f, 0.0f) *
                              vmath::rotate(20.0f, 1.0f, 0.0f, 0.0f) *
                              vmath::translate(0.0f, -4.0f, 0.0f);

    glEnable(GL_DEPTH_TEST);
    render_scene(total_time);

    glUseProgram(filter_program);

    glBindImageTexture(0, color_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, temp_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute(info.windowHeight, 1, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindImageTexture(0, temp_tex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, color_tex, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute(info.windowWidth, 1, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, color_tex);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(display_program);
    glUniform1f(uniforms.dof.focal_distance, focal_distance);
    glUniform1f(uniforms.dof.focal_depth, focal_depth);
    glBindVertexArray(quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    overlay.draw();
}

void dof_app::render_scene(double currentTime)
{
    static const GLfloat ones[] = { 1.0f };
    static const GLfloat zero[] = { 0.0f };
    static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
    static const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
    static const vmath::mat4 scale_bias_matrix = vmath::mat4(vmath::vec4(0.5f, 0.0f, 0.0f, 0.0f),
                                                             vmath::vec4(0.0f, 0.5f, 0.0f, 0.0f),
                                                             vmath::vec4(0.0f, 0.0f, 0.5f, 0.0f),
                                                             vmath::vec4(0.5f, 0.5f, 0.5f, 1.0f));

    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

    glDrawBuffers(1, attachments);
    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glClearBufferfv(GL_COLOR, 0, gray);
    glClearBufferfv(GL_DEPTH, 0, ones);
    glUseProgram(view_program);
    glUniformMatrix4fv(uniforms.view.proj_matrix, 1, GL_FALSE, camera_proj_matrix);

    glClearBufferfv(GL_DEPTH, 0, ones);

    int i;
    for (i = 0; i < OBJECT_COUNT; i++)
    {
        vmath::mat4& model_matrix = objects[i].model_matrix;
        glUniformMatrix4fv(uniforms.view.mv_matrix, 1, GL_FALSE, camera_view_matrix * objects[i].model_matrix);
        glUniform3fv(uniforms.view.diffuse_albedo, 1, objects[i].diffuse_albedo);
        objects[0].obj.render();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void dof_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'Q':
                focal_distance *= 1.1f;
                break;
            case'A':
                focal_distance /= 1.1f;
                break;
            case 'W':
                focal_depth *= 1.1f;
                break;
            case 'S':
                focal_depth /= 1.1f;
                break;
            case 'R':
                load_shaders();
                break;
            case 'P':
                paused = !paused;
                break;
        }
    }
}

void dof_app::load_shaders()
{
    GLuint shaders[4];

    shaders[0] = sb7::shader::load("media/shaders/dof/render.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/dof/render.fs.glsl", GL_FRAGMENT_SHADER);

    if (view_program)
        glDeleteProgram(view_program);

    view_program = sb7::program::link_from_shaders(shaders, 2, true);

    uniforms.view.proj_matrix = glGetUniformLocation(view_program, "proj_matrix");
    uniforms.view.mv_matrix = glGetUniformLocation(view_program, "mv_matrix");
    uniforms.view.full_shading = glGetUniformLocation(view_program, "full_shading");
    uniforms.view.diffuse_albedo = glGetUniformLocation(view_program, "diffuse_albedo");

    shaders[0] = sb7::shader::load("media/shaders/dof/display.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/dof/display.fs.glsl", GL_FRAGMENT_SHADER);

    if (display_program)
        glDeleteProgram(display_program);

    display_program = sb7::program::link_from_shaders(shaders, 2, true);

    uniforms.dof.focal_distance = glGetUniformLocation(display_program, "focal_distance");
    uniforms.dof.focal_depth = glGetUniformLocation(display_program, "focal_depth");

    shaders[0] = sb7::shader::load("media/shaders/dof/gensat.cs.glsl", GL_COMPUTE_SHADER);

    if (filter_program)
        glDeleteProgram(filter_program);

    filter_program = sb7::program::link_from_shaders(shaders, 1, true);
}

DECLARE_MAIN(dof_app)
