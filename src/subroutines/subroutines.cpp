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

class subroutines_app : public sb7::application
{
public:
    subroutines_app()
        : render_program(0)
    {

    }

    void startup();

    void render(double currentTime);

protected:
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Shader Subroutines";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void load_shaders();
    void onKey(int key, int action);

    GLuint      render_program;

    GLuint      vao;

    GLuint      subroutines[2];

    struct
    {
        GLint subroutine1;
    } uniforms;
};

void subroutines_app::startup()
{
    load_shaders();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
}

void subroutines_app::render(double currentTime)
{
    int i = int(currentTime);

    glUseProgram(render_program);

    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutines[i & 1]);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void subroutines_app::load_shaders()
{
    GLuint shaders[2];

    shaders[0] = sb7::shader::load("media/shaders/subroutines/subroutines.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/subroutines/subroutines.fs.glsl", GL_FRAGMENT_SHADER);

    if (render_program)
        glDeleteProgram(render_program);

    render_program = sb7::program::link_from_shaders(shaders, 2, true);

    subroutines[0] = glGetSubroutineIndex(render_program, GL_FRAGMENT_SHADER, "myFunction1");
    subroutines[1] = glGetSubroutineIndex(render_program, GL_FRAGMENT_SHADER, "myFunction2");

    uniforms.subroutine1 = glGetSubroutineUniformLocation(render_program, GL_FRAGMENT_SHADER, "mySubroutineUniform");
}

void subroutines_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'R':
                load_shaders();
                break;
        }
    }
}

DECLARE_MAIN(subroutines_app)
