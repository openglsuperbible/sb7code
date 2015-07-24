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

class dflandscape_app : public sb7::application
{
public:
    dflandscape_app()
        : map_texture(0),
          render_mode(MODE_LOGO)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Distance Field Lanscaping";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup()
    {
        map_texture = sb7::ktx::file::load("media/textures/psycho-map-df-sm.ktx");
        grass_texture = sb7::ktx::file::load("media/textures/mossygrass.ktx");
        rocks_texture = sb7::ktx::file::load("media/textures/rocks.ktx");

        glBindTexture(GL_TEXTURE_2D, map_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glBindTexture(GL_TEXTURE_2D, grass_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glBindTexture(GL_TEXTURE_2D, rocks_texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glGenVertexArrays(1, &vao);
        load_shaders();
    }

    void shutdown()
    {
        glDeleteTextures(1, &map_texture);
    }

    void render(double currentTime)
    {
        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        // const GLfloat aspect = (float)info.windowHeight / (float)info.windowWidth;
        const GLfloat aspect = (float)info.windowWidth / (float)info.windowHeight;

        glBindVertexArray(vao);

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glClearBufferfv(GL_COLOR, 0, black);

        float scale = (float)(cos(currentTime * 0.2) * sin(currentTime * 0.15) * 3.0 + 3.2);
        float cos_t = (float)cos(currentTime * 0.03) * 0.25f;
        float sin_t = (float)sin(currentTime * 0.02) * 0.25f;

        float m[] =
        {
            cos_t, sin_t, 0.0f,
            -sin_t, cos_t, 0.0f,
            0.0f, 0.0f, 1.0f
        };

        float m2[] =
        {
            cos_t * scale, sin_t * scale, 0.0f,
            -sin_t * scale, cos_t * scale, 0.0f,
            cos_t, sin_t, 1.0f
        };

        vmath::mat4 transform;
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, map_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grass_texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, rocks_texture);

        vmath::mat4 projection;

        transform = vmath::translate(0.0f, 0.0f, -1.0f) *
                    vmath::scale(10.0f, 10.0f, 1.0f);

        projection = vmath::frustum(-aspect, aspect, 1.0, -1.0, 1.0f, 100.0f);

        glUseProgram(sdf_program);
        glUniformMatrix3fv(0, 1, GL_FALSE, m2);
        glUniformMatrix4fv(1, 1, GL_FALSE, projection * transform);
        // glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 1);
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

        shader.vs = sb7::shader::load("media/shaders/dflandscape/dflandscape.vs.glsl", GL_VERTEX_SHADER);
        shader.fs = sb7::shader::load("media/shaders/dflandscape/dflandscape.fs.glsl", GL_FRAGMENT_SHADER);

        glDeleteProgram(sdf_program);

        sdf_program = sb7::program::link_from_shaders(shader.shaders, 2, true);

        GLint foo = glGetUniformLocation(sdf_program, "uv_transform");
    }

    void onKey(int key, int action);

private:
    GLuint      map_texture;
    GLuint      grass_texture;
    GLuint      rocks_texture;
    GLuint      sdf_program;

    GLuint      vao;

    enum RENDER_MODE
    {
        MODE_LOGO,
        MODE_TEXT
    };

    int         render_mode;
};

void dflandscape_app::onKey(int key, int action)
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

DECLARE_MAIN(dflandscape_app)
