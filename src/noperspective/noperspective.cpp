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

class noperspective_app : public sb7::application
{
public:
    noperspective_app()
        : paused(false),
          use_perspective(true)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Perspective";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        static const char * vs_source[] =
        {
            "#version 410 core                                                                  \n"
            "                                                                                   \n"
            "out VS_OUT                                                                         \n"
            "{                                                                                  \n"
            "    vec2 tc;                                                                       \n"
            "    noperspective vec2 tc_np;                                                      \n"
            "} vs_out;                                                                          \n"
            "                                                                                   \n"
            "uniform mat4 mvp;                                                                  \n"
            "                                                                                   \n"
            "void main(void)                                                                    \n"
            "{                                                                                  \n"
            "    const vec4 vertices[] = vec4[](vec4(-0.5, -0.5, 0.0, 1.0),                     \n"
            "                                   vec4( 0.5, -0.5, 0.0, 1.0),                     \n"
            "                                   vec4(-0.5,  0.5, 0.0, 1.0),                     \n"
            "                                   vec4( 0.5,  0.5, 0.0, 1.0));                    \n"
            "                                                                                   \n"
            "    vec2 tc = (vertices[gl_VertexID].xy + vec2(0.5));                              \n"
            "    vs_out.tc = tc;                                                                \n"
            "    vs_out.tc_np = tc;                                                             \n"
            "    gl_Position = mvp * vertices[gl_VertexID];                                     \n"
            "}                                                                                  \n"
        };

        static const char * fs_source[] =
        {
            "#version 410 core                                                 \n"
            "                                                                  \n"
            "out vec4 color;                                                   \n"
            "                                                                  \n"
            "uniform sampler2D tex_checker;                                    \n"
            "                                                                  \n"
            "uniform bool use_perspective = true;                              \n"
            "                                                                  \n"
            "in VS_OUT                                                         \n"
            "{                                                                 \n"
            "    vec2 tc;                                                      \n"
            "    noperspective vec2 tc_np;                                     \n"
            "} fs_in;                                                          \n"
            "                                                                  \n"
            "void main(void)                                                   \n"
            "{                                                                 \n"
            "    vec2 tc = mix(fs_in.tc_np, fs_in.tc, bvec2(use_perspective));        \n"
            "    color = texture(tex_checker, tc).rrrr;                        \n"
            "}                                                                 \n"
        };

        char buffer[1024];

        program = glCreateProgram();
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vs_source, NULL);
        glCompileShader(vs);

        glGetShaderInfoLog(vs, 1024, NULL, buffer);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        glGetShaderInfoLog(fs, 1024, NULL, buffer);

        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glLinkProgram(program);

        uniforms.mvp = glGetUniformLocation(program, "mvp");
        uniforms.use_perspective = glGetUniformLocation(program, "use_perspective");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        static const unsigned char checker_data[] =
        {
            0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
            0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
            0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
            0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
            0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
            0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
            0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
            0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00,
        };

        glGenTextures(1, &tex_checker);
        glBindTexture(GL_TEXTURE_2D, tex_checker);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8, 8, 8);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_RED, GL_UNSIGNED_BYTE, checker_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    virtual void render(double currentTime)
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        static const GLfloat one = 1.0f;
        static double last_time = 0.0;
        static double total_time = 0.0;

        if (!paused)
            total_time += (currentTime - last_time);
        last_time = currentTime;

        float t = (float)total_time * 14.3f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -1.5f) *
                                vmath::rotate(t, 0.0f, 1.0f, 0.0f);
        vmath::mat4 proj_matrix = vmath::perspective(60.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f, 1000.0f);

        glUseProgram(program);

        glUniformMatrix4fv(uniforms.mvp, 1, GL_FALSE, proj_matrix * mv_matrix);
        glUniform1i(uniforms.use_perspective, use_perspective);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
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
                case 'M': use_perspective = !use_perspective;
                    break;
                case 'P': paused = !paused;
                    break;
                default:
                    break;
            };
        }
    }

private:
    GLuint          program;
    GLuint          vao;
    GLuint          tex_checker;
    bool            paused;
    bool            use_perspective;

    struct
    {
        GLint       mvp;
        GLint       use_perspective;
    } uniforms;
};

DECLARE_MAIN(noperspective_app)
