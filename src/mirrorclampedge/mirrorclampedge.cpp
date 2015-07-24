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
#include <sb7ktx.h>
#include <sb7color.h>
#include <sb7textoverlay.h>

#define TEXTURE_WIDTH 512
#define TEXTURE_HEIGHT 512

class mirrorclamp_app : public sb7::application
{
public:
    mirrorclamp_app();
    void init();
    void startup();
    void render(double T);
    void onKey(int key, int action);

private:
    GLuint          render_program;
    GLuint          input_texture;
    GLuint          output_texture;
    GLuint          output_buffer;
    GLuint          output_buffer_texture;
    GLuint          dummy_vao;

    enum
    {
        CLAMP_TO_BORDER,
        MIRROR_CLAMP_TO_EDGE,
        MAX_MODE
    };

    int display_mode;

    void            load_shaders();

    sb7::text_overlay overlay;
};

mirrorclamp_app::mirrorclamp_app()
    : render_program(0),
      display_mode(CLAMP_TO_BORDER)
{
}

void mirrorclamp_app::init()
{
    static const char title[] = "OpenGL SuperBible - GL_MIRROR_CLAMP_TO_EDGE";

    sb7::application::init();

    memcpy(info.title, title, sizeof(title));
}

void mirrorclamp_app::startup()
{
    overlay.init(80, 50);

    input_texture = sb7::ktx::file::load("media/textures/flare.ktx");

    load_shaders();

    glGenVertexArrays(1, &dummy_vao);
}

void mirrorclamp_app::render(double T)
{
    glViewport(0, 0, info.windowWidth, info.windowHeight);

    glClearBufferfv(GL_COLOR, 0, sb7::color::Black);

    Sleep(1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input_texture);

    glBindVertexArray(dummy_vao);
    glUseProgram(render_program);

    switch (display_mode)
    {
        case CLAMP_TO_BORDER:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            break;
        case MIRROR_CLAMP_TO_EDGE:
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRROR_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRROR_CLAMP_TO_EDGE);
            break;
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    overlay.clear();
    overlay.drawText(display_mode == CLAMP_TO_BORDER ? "Mode = GL_CLAMP_TO_BORDER (M toggles)" : "Mode = GL_MIRROR_CLAMP_TO_EDGE (M toggles)", 0, 0);
    overlay.draw();
}


void mirrorclamp_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'M':
                display_mode = display_mode + 1;
                if (display_mode == MAX_MODE)
                    display_mode = CLAMP_TO_BORDER;
                break;
            case 'R':
                load_shaders();
                break;
        }
    }
}

void mirrorclamp_app::load_shaders()
{
    glDeleteProgram(render_program);

    GLuint shaders[2];

    shaders[0] = sb7::shader::load("media/shaders/mirrorclampedge/drawquad.fs.glsl", GL_FRAGMENT_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/mirrorclampedge/drawquad.vs.glsl", GL_VERTEX_SHADER);

    render_program = sb7::program::link_from_shaders(shaders, 2, true);
}

DECLARE_MAIN(mirrorclamp_app)
