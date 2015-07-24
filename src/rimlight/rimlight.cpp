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

class rimlight_app : public sb7::application
{
public:
    rimlight_app()
        : program(0),
          paused(false),
          rim_color(0.3f, 0.3f, 0.3f),
          rim_power(2.5f),
          rim_enable(true)
    {
        mat_rotation = vmath::mat4::identity();
    }

protected:
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Rim Lighting";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup()
    {
        loadShaders();

        object.load("media/objects/dragon.sbm");

        glEnable(GL_CULL_FACE);
        //glCullFace(GL_FRONT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    void render(double currentTime)
    {
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat one = 1.0f;
        static double last_time = 0.0;
        static double total_time = 0.0;

        if (!paused)
            total_time += (currentTime - last_time);
        last_time = currentTime;

        float f = (float)total_time;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(program);

        vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f,
                                                     1000.0f);
        glUniformMatrix4fv(uniforms.proj_matrix, 1, GL_FALSE, proj_matrix);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, -5.0f, -20.0f) *
                                vmath::rotate(f * 5.0f, 0.0f, 1.0f, 0.0f) *
                                vmath::mat4::identity();
        glUniformMatrix4fv(uniforms.mv_matrix, 1, GL_FALSE, mv_matrix);

        glUniform3fv(uniforms.rim_color, 1, rim_enable ? rim_color : vmath::vec3(0.0f));
        glUniform1f(uniforms.rim_power, rim_power);

        object.render();
    }

    virtual void shutdown()
    {
        object.free();
        glDeleteProgram(program);
    }

    void onKey(int key, int action)
    {
        if (action)
        {
            switch (key)
            {
                case 'Q':
                    rim_color[0] += 0.1f;
                    break;
                case 'W':
                    rim_color[1] += 0.1f;
                    break;
                case 'E':
                    rim_color[2] += 0.1f;
                    break;
                case 'R':
                    rim_power *= 1.5f;
                    break;
                case 'A':
                    rim_color[0] -= 0.1f;
                    break;
                case 'S':
                    rim_color[1] -= 0.1f;
                    break;
                case 'D':
                    rim_color[2] -= 0.1f;
                    break;
                case 'F':
                    rim_power /= 1.5f;
                    break;
                case 'Z':
                    rim_enable = !rim_enable;
                    break;

                case 'P':
                    paused = !paused;
                    break;
                case 'L':
                    loadShaders();
                    break;
            }
        }
    }

    void loadShaders()
    {
        GLuint vs;
        GLuint fs;

        vs = sb7::shader::load("media/shaders/rimlight/render.vs.glsl", GL_VERTEX_SHADER);
        fs = sb7::shader::load("media/shaders/rimlight/render.fs.glsl", GL_FRAGMENT_SHADER);

        if (program != 0)
            glDeleteProgram(program);

        program = glCreateProgram();

        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glLinkProgram(program);

        uniforms.mv_matrix = glGetUniformLocation(program, "mv_matrix");
        uniforms.proj_matrix = glGetUniformLocation(program, "proj_matrix");
        uniforms.rim_color = glGetUniformLocation(program, "rim_color");
        uniforms.rim_power = glGetUniformLocation(program, "rim_power");
    }

private:
    GLuint          program;
    struct
    {
        GLint           mv_matrix;
        GLint           proj_matrix;
        GLint           rim_color;
        GLint           rim_power;
    } uniforms;

    vmath::mat4         mat_rotation;

    sb7::object         object;
    bool                paused;
    vmath::vec3         rim_color;
    float               rim_power;
    bool                rim_enable;
};

DECLARE_MAIN(rimlight_app)
