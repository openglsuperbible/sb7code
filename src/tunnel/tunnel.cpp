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

class tunnel_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Tunnel";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup()
    {
        GLuint  vs, fs;

        static const char * vs_source[] =
        {
            "#version 420 core                                                      \n"
            "                                                                       \n"
            "out VS_OUT                                                             \n"
            "{                                                                      \n"
            "    vec2 tc;                                                           \n"
            "} vs_out;                                                              \n"
            "                                                                       \n"
            "uniform mat4 mvp;                                                      \n"
            "uniform float offset;                                                  \n"
            "                                                                       \n"
            "void main(void)                                                        \n"
            "{                                                                      \n"
            "    const vec2[4] position = vec2[4](vec2(-0.5, -0.5),                 \n"
            "                                     vec2( 0.5, -0.5),                 \n"
            "                                     vec2(-0.5,  0.5),                 \n"
            "                                     vec2( 0.5,  0.5));                \n"
            "    vs_out.tc = (position[gl_VertexID].xy + vec2(offset, 0.5)) * vec2(30.0, 1.0);                  \n"
            "    gl_Position = mvp * vec4(position[gl_VertexID], 0.0, 1.0);       \n"
            "}                                                                      \n"
        };

        static const char * fs_source[] =
        {
            "#version 420 core                                                      \n"
            "                                                                       \n"
            "layout (location = 0) out vec4 color;                                  \n"
            "                                                                       \n"
            "in VS_OUT                                                              \n"
            "{                                                                      \n"
            "    vec2 tc;                                                           \n"
            "} fs_in;                                                               \n"
            "                                                                       \n"
            "layout (binding = 0) uniform sampler2D tex;                            \n"
            "                                                                       \n"
            "void main(void)                                                        \n"
            "{                                                                      \n"
            "    color = texture(tex, fs_in.tc);                                    \n"
            "}                                                                      \n"
        };

        char buffer[1024];

        vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vs_source, NULL);
        glCompileShader(vs);

        glGetShaderInfoLog(vs, 1024, NULL, buffer);

        fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        glGetShaderInfoLog(vs, 1024, NULL, buffer);

        render_prog = glCreateProgram();
        glAttachShader(render_prog, vs);
        glAttachShader(render_prog, fs);
        glLinkProgram(render_prog);

        glDeleteShader(vs);
        glDeleteShader(fs);

        glGetProgramInfoLog(render_prog, 1024, NULL, buffer);

        uniforms.mvp = glGetUniformLocation(render_prog, "mvp");
        uniforms.offset = glGetUniformLocation(render_prog, "offset");

        glGenVertexArrays(1, &render_vao);
        glBindVertexArray(render_vao);

        tex_wall = sb7::ktx::file::load("media/textures/brick.ktx");
        tex_ceiling = sb7::ktx::file::load("media/textures/ceiling.ktx");
        tex_floor = sb7::ktx::file::load("media/textures/floor.ktx");

        int i;
        GLuint textures[] = { tex_floor, tex_wall, tex_ceiling };

        for (i = 0; i < 3; i++)
        {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        glBindVertexArray(render_vao);
    }

    void render(double currentTime)
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        float t = (float)currentTime;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);

        glUseProgram(render_prog);

        vmath::mat4 proj_matrix = vmath::perspective(60.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f, 100.0f);

        glUniform1f(uniforms.offset, t * 0.003f);

        int i;
        GLuint textures[] = { tex_wall, tex_floor, tex_wall, tex_ceiling };
        for (i = 0; i < 4; i++)
        {
            vmath::mat4 mv_matrix = vmath::rotate(90.0f * (float)i, vmath::vec3(0.0f, 0.0f, 1.0f)) *
                                    vmath::translate(-0.5f, 0.0f, -10.0f) *
                                    vmath::rotate(90.0f, 0.0f, 1.0f, 0.0f) *
                                    vmath::scale(30.0f, 1.0f, 1.0f);
            vmath::mat4 mvp = proj_matrix * mv_matrix;

            glUniformMatrix4fv(uniforms.mvp, 1, GL_FALSE, mvp);

            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

protected:
    GLuint          render_prog;
    GLuint          render_vao;
    struct
    {
        GLint       mvp;
        GLint       offset;
    } uniforms;

    GLuint          tex_wall;
    GLuint          tex_ceiling;
    GLuint          tex_floor;
};

DECLARE_MAIN(tunnel_app)
