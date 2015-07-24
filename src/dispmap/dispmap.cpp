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
#include <sb7ktx.h>
#include <vmath.h>
#include <shader.h>

class dispmap_app : public sb7::application
{
public:
    dispmap_app()
        : program(0),
          enable_displacement(true),
          wireframe(false),
          enable_fog(true),
          paused(false)
    {

    }

    void load_shaders();

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Displacement Mapping";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        load_shaders();

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glPatchParameteri(GL_PATCH_VERTICES, 4);

        glEnable(GL_CULL_FACE);

        tex_displacement = sb7::ktx::file::load("media/textures/terragen1.ktx");
        glActiveTexture(GL_TEXTURE1);
        tex_color = sb7::ktx::file::load("media/textures/terragen_color.ktx");
    }

    virtual void render(double currentTime)
    {
        static const GLfloat black[] = { 0.85f, 0.95f, 1.0f, 1.0f };
        static const GLfloat one = 1.0f;
        static double last_time = 0.0;
        static double total_time = 0.0;

        if (!paused)
            total_time += (currentTime - last_time);
        last_time = currentTime;

        float t = (float)total_time * 0.03f;
        float r = sinf(t * 5.37f) * 15.0f + 16.0f;
        float h = cosf(t * 4.79f) * 2.0f + 3.2f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        vmath::mat4 mv_matrix = /* vmath::translate(0.0f, 0.0f, -1.4f) *
                                vmath::translate(0.0f, -0.4f, 0.0f) * */
                                // vmath::rotate((float)currentTime * 6.0f, 0.0f, 1.0f, 0.0f) *
                                vmath::lookat(vmath::vec3(sinf(t) * r, h, cosf(t) * r), vmath::vec3(0.0f), vmath::vec3(0.0f, 1.0f, 0.0f));
        vmath::mat4 proj_matrix = vmath::perspective(60.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f, 1000.0f);

        glUseProgram(program);

        glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);
        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);
        glUniformMatrix4fv(uniforms.mvp_matrix, 1, GL_FALSE, proj_matrix * mv_matrix);
        glUniform1f(uniforms.dmap_depth, enable_displacement ? dmap_depth : 0.0f);
        glUniform1i(uniforms.enable_fog, enable_fog ? 1 : 0);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        if (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawArraysInstanced(GL_PATCHES, 0, 4, 64 * 64);
    }

    virtual void shutdown()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
    }

    void onKey(int key, int action)
    {
        if (action == 1)
        {
            switch (key)
            {
                case GLFW_KEY_KP_ADD: dmap_depth += 0.1f;
                    break;
                case GLFW_KEY_KP_SUBTRACT: dmap_depth -= 0.1f;
                    break;
                case 'F': enable_fog = !enable_fog;
                    break;
                case 'D': enable_displacement = !enable_displacement;
                    break;
                case 'W': wireframe = !wireframe;
                    break;
                case 'P': paused = !paused;
                    break;
                case 'R':
                        load_shaders();
                    break;
                default:
                    break;
            };
        }
    }

private:
    GLuint          program;
    GLuint          vao;
    GLuint          tex_displacement;
    GLuint          tex_color;
    float           dmap_depth;
    bool            enable_displacement;
    bool            wireframe;
    bool            enable_fog;
    bool            paused;

    struct
    {
        GLint       mvp_matrix;
        GLint       mv_matrix;
        GLint       proj_matrix;
        GLint       dmap_depth;
        GLint       enable_fog;
    } uniforms;
};

void dispmap_app::load_shaders()
{
    if (program)
        glDeleteProgram(program);

    GLuint vs = sb7::shader::load("media/shaders/dispmap/dispmap.vs.glsl", GL_VERTEX_SHADER);
    GLuint tcs = sb7::shader::load("media/shaders/dispmap/dispmap.tcs.glsl", GL_TESS_CONTROL_SHADER);
    GLuint tes = sb7::shader::load("media/shaders/dispmap/dispmap.tes.glsl", GL_TESS_EVALUATION_SHADER);
    GLuint fs = sb7::shader::load("media/shaders/dispmap/dispmap.fs.glsl", GL_FRAGMENT_SHADER);

    program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, tcs);
    glAttachShader(program, tes);
    glAttachShader(program, fs);

    glLinkProgram(program);

    uniforms.mv_matrix = glGetUniformLocation(program, "mv_matrix");
    uniforms.mvp_matrix = glGetUniformLocation(program, "mvp_matrix");
    uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
    uniforms.dmap_depth = glGetUniformLocation(program, "dmap_depth");
    uniforms.enable_fog = glGetUniformLocation(program, "enable_fog");
    dmap_depth = 6.0f;
}

DECLARE_MAIN(dispmap_app)

