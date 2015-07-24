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
#include <sb7ktx.h>
#include <shader.h>

class sdfdemo_app : public sb7::application
{
public:
    sdfdemo_app()
        : sdf_texture(0),
          render_mode(MODE_LOGO)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Distance Field (Text)";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup()
    {
        logo_texture = sb7::ktx::file::load("media/textures/gllogodistsmarray.ktx");
        glBindTexture(GL_TEXTURE_2D_ARRAY, sdf_texture);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        sdf_texture = sb7::ktx::file::load("media/textures/chars-df-array.ktx");
        glBindTexture(GL_TEXTURE_2D_ARRAY, sdf_texture);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glGenVertexArrays(1, &vao);
        load_shaders();
    }

    void shutdown()
    {
        glDeleteTextures(1, &sdf_texture);
    }

    void render(double currentTime)
    {
        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        // const GLfloat aspect = (float)info.windowHeight / (float)info.windowWidth;
        const GLfloat aspect = (float)info.windowWidth / (float)info.windowHeight;

        glBindVertexArray(vao);

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glClearBufferfv(GL_COLOR, 0, black);

        float scale = (float)(cos(currentTime * 0.2) * sin(currentTime * 0.15) * 6.0 + 3.001);
        float cos_t = (float)cos(currentTime) * scale;
        float sin_t = (float)sin(currentTime) * scale;

        float m[] =
        {
            cos_t, sin_t, 0.0f,
            -sin_t, cos_t, 0.0f,
            0.0f, 0.0f, 1.0f
        };

        float m2[] =
        {
            1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 1.0f
        };

        vmath::mat4 transform;

        int num_chars = 1;
        float tz = -10.0f;
        float rs = 1.0f;

        switch (render_mode)
        {
            case MODE_LOGO:
                num_chars = 1;
                tz = -4.0f;
                glBindTexture(GL_TEXTURE_2D_ARRAY, logo_texture);
                transform = vmath::translate(0.0f, 0.0f, -6.0f + cos_t * 0.25f) *
                            vmath::rotate(cos_t * 12.0f, vmath::vec3(1.0, 0.0, 0.0)) *
                            vmath::rotate(sin_t * 15.0f, vmath::vec3(0.0, 1.0, 0.0)) *
                            vmath::scale(6.0f, 6.0f, 1.0f);
                break;
            case MODE_TEXT:
                num_chars = 24;
                tz = -10.0f;
                rs = 4.0;
                glBindTexture(GL_TEXTURE_2D_ARRAY, sdf_texture);
                transform = vmath::translate(0.0f, 0.0f, tz + cos_t) *
                            vmath::rotate(cos_t * 4.0f * rs, vmath::vec3(1.0, 0.0, 0.0)) *
                            vmath::rotate(sin_t * 3.0f * rs, vmath::vec3(0.0, 0.0, 1.0)) *
                            vmath::translate(-float((num_chars - 1)) + sin_t, 0.0f, 0.0f);
                break;
        }

        vmath::mat4 projection;

        projection = vmath::frustum(-aspect, aspect, 1.0, -1.0, 1.0f, 100.0f);

        glUseProgram(sdf_program);
        glUniformMatrix3fv(0, 1, GL_FALSE, m2);
        glUniformMatrix4fv(1, 1, GL_FALSE, projection * transform);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, num_chars);
    }

    void load_shaders()
    {
        union
        {
            struct
            {
                GLuint      vs;
                GLuint      fs;
            };
            GLuint          shaders[2];
        } shader;

        shader.vs = sb7::shader::load("media/shaders/sdfdemo/sdf.vs.glsl", GL_VERTEX_SHADER);
        shader.fs = sb7::shader::load("media/shaders/sdfdemo/sdf.fs.glsl", GL_FRAGMENT_SHADER);

        sdf_program = sb7::program::link_from_shaders(shader.shaders, 2, true);

        GLint foo = glGetUniformLocation(sdf_program, "uv_transform");
    }

    void onKey(int key, int action);

private:
    GLuint      sdf_texture;
    GLuint      logo_texture;
    GLuint      sdf_program;

    GLuint      vao;

    enum RENDER_MODE
    {
        MODE_LOGO,
        MODE_TEXT
    };

    int         render_mode;
};

void sdfdemo_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case '1':
                render_mode = MODE_LOGO;
                setWindowTitle("OpenGL SuperBible - Distance Field (Logo)");
                break;
            case '2':
                render_mode = MODE_TEXT;
                setWindowTitle("OpenGL SuperBible - Distance Field (Text)");
                break;
            case 'R': 
                load_shaders();
                break;
        }
    }
}

DECLARE_MAIN(sdfdemo_app)
