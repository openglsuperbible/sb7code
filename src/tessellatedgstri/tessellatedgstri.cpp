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

class tessllatedgstri_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Tessellation and Geometry Shaders";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        static const char * vs_source[] =
        {
            "#version 410 core                                                                  \n"
            "                                                                                   \n"
            "void main(void)                                                                    \n"
            "{                                                                                  \n"
            "    const vec4 vertices[] = vec4[](vec4( 0.25, -0.25, 0.5, 1.0),                   \n"
            "                                   vec4(-0.25, -0.25, 0.5, 1.0),                   \n"
            "                                   vec4( 0.25,  0.25, 0.5, 1.0));                  \n"
            "                                                                                   \n"
            "    gl_Position = vertices[gl_VertexID];                                           \n"
            "}                                                                                  \n"
        };

        static const char * tcs_source[] =
        {
            "#version 410 core                                                                  \n"
            "                                                                                   \n"
            "layout (vertices = 3) out;                                                         \n"
            "                                                                                   \n"
            "void main(void)                                                                    \n"
            "{                                                                                  \n"
            "    if (gl_InvocationID == 0)                                                      \n"
            "    {                                                                              \n"
            "        gl_TessLevelInner[0] = 5.0;                                                \n"
            "        gl_TessLevelOuter[0] = 5.0;                                                \n"
            "        gl_TessLevelOuter[1] = 5.0;                                                \n"
            "        gl_TessLevelOuter[2] = 5.0;                                                \n"
            "    }                                                                              \n"
            "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;      \n"
            "}                                                                                  \n"
        };

        static const char * tes_source[] =
        {
            "#version 410 core                                                                  \n"
            "                                                                                   \n"
            "layout (triangles, equal_spacing, cw) in;                                          \n"
            "                                                                                   \n"
            "void main(void)                                                                    \n"
            "{                                                                                  \n"
            "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +                        \n"
            "                  (gl_TessCoord.y * gl_in[1].gl_Position) +                        \n"
            "                  (gl_TessCoord.z * gl_in[2].gl_Position);                         \n"
            "}                                                                                  \n"
        };

        static const char * gs_source[] =
        {
            "#version 410 core                                                                  \n"
            "                                                                                   \n"
            "layout (triangles) in;                                                             \n"
            "layout (points, max_vertices = 3) out;                                             \n"
            "                                                                                   \n"
            "void main(void)                                                                    \n"
            "{                                                                                  \n"
            "    int i;                                                                         \n"
            "                                                                                   \n"
            "    for (i = 0; i < gl_in.length(); i++)                                           \n"
            "    {                                                                              \n"
            "        gl_Position = gl_in[i].gl_Position;                                        \n"
            "        EmitVertex();                                                              \n"
            "    }                                                                              \n"
            "}                                                                                  \n"
        };

        static const char * fs_source[] =
        {
            "#version 410 core                                                 \n"
            "                                                                  \n"
            "out vec4 color;                                                   \n"
            "                                                                  \n"
            "void main(void)                                                   \n"
            "{                                                                 \n"
            "    color = vec4(0.0, 0.8, 1.0, 1.0);                             \n"
            "}                                                                 \n"
        };

        program = glCreateProgram();
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vs_source, NULL);
        glCompileShader(vs);

        GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tcs, 1, tcs_source, NULL);
        glCompileShader(tcs);

        GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tes, 1, tes_source, NULL);
        glCompileShader(tes);

        GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gs, 1, gs_source, NULL);
        glCompileShader(gs);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        glAttachShader(program, vs);
        glAttachShader(program, tcs);
        glAttachShader(program, tes);
        glAttachShader(program, gs);
        glAttachShader(program, fs);

        glLinkProgram(program);

        glDeleteShader(vs);
        glDeleteShader(tcs);
        glDeleteShader(tes);
        glDeleteShader(gs);
        glDeleteShader(fs);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    }

    virtual void render(double currentTime)
    {
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, green);

        glUseProgram(program);

        glPointSize(5.0f);

        glDrawArrays(GL_PATCHES, 0, 3);
    }

    virtual void shutdown()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
    }

private:
    GLuint          program;
    GLuint          vao;
};

DECLARE_MAIN(tessllatedgstri_app)
