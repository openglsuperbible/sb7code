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
#include <cmath>

class tesssubdivmodes_app : public sb7::application
{
public:
    tesssubdivmodes_app()
        : program_index(0)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Subdivision Modes";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        static const char * vs_source[] =
        {
            "#version 420 core                                                 \n"
            "                                                                  \n"
            "void main(void)                                                   \n"
            "{                                                                 \n"
            "    const vec4 vertices[] = vec4[](vec4( 0.8, -0.8, 0.5, 1.0),    \n"
            "                                   vec4(-0.8, -0.8, 0.5, 1.0),    \n"
            "                                   vec4( 0.8,  0.8, 0.5, 1.0),    \n"
            "                                   vec4(-0.8,  0.8, 0.5, 1.0));   \n"
            "                                                                  \n"
            "    gl_Position = vertices[gl_VertexID];                          \n"
            "}                                                                 \n"
        };

        static const char * tcs_source_triangles[] =
        {
            "#version 420 core                                                                 \n"
            "                                                                                  \n"
            "layout (vertices = 3) out;                                                        \n"
            "                                                                                  \n"
            "uniform float tess_level = 2.7;                                                   \n"
            "                                                                                  \n"
            "void main(void)                                                                   \n"
            "{                                                                                 \n"
            "    if (gl_InvocationID == 0)                                                     \n"
            "    {                                                                             \n"
            "        gl_TessLevelInner[0] = tess_level;                                        \n"
            "        gl_TessLevelOuter[0] = tess_level;                                        \n"
            "        gl_TessLevelOuter[1] = tess_level;                                        \n"
            "        gl_TessLevelOuter[2] = tess_level;                                        \n"
            "    }                                                                             \n"
            "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;     \n"
            "}                                                                                 \n"
        };

        static const char * tes_source_equal[] =
        {
            "#version 420 core                                                                 \n"
            "                                                                                  \n"
            "layout (triangles) in;                                                            \n"
            "                                                                                  \n"
            "void main(void)                                                                   \n"
            "{                                                                                 \n"
            "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +                       \n"
            "                  (gl_TessCoord.y * gl_in[1].gl_Position) +                       \n"
            "                  (gl_TessCoord.z * gl_in[2].gl_Position);                        \n"
            "}                                                                                 \n"
        };

        static const char * tes_source_fract_even[] =
        {
            "#version 420 core                                                                 \n"
            "                                                                                  \n"
            "layout (triangles, fractional_even_spacing) in;                                   \n"
            "                                                                                  \n"
            "void main(void)                                                                   \n"
            "{                                                                                 \n"
            "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +                       \n"
            "                  (gl_TessCoord.y * gl_in[1].gl_Position) +                       \n"
            "                  (gl_TessCoord.z * gl_in[2].gl_Position);                        \n"
            "}                                                                                 \n"
        };

        static const char * tes_source_fract_odd[] =
        {
            "#version 420 core                                                                 \n"
            "                                                                                  \n"
            "layout (triangles, fractional_odd_spacing) in;                                    \n"
            "                                                                                  \n"
            "void main(void)                                                                   \n"
            "{                                                                                 \n"
            "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +                       \n"
            "                  (gl_TessCoord.y * gl_in[1].gl_Position) +                       \n"
            "                  (gl_TessCoord.z * gl_in[2].gl_Position);                        \n"
            "}                                                                                 \n"
        };

        static const char * fs_source[] =
        {
            "#version 420 core                                                  \n"
            "                                                                   \n"
            "out vec4 color;                                                    \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    color = vec4(1.0);                                             \n"
            "}                                                                  \n"
        };

        int i;

        static const char * const * vs_sources[] =
        {
            vs_source, vs_source, vs_source
        };

        static const char * const * tcs_sources[] =
        {
            tcs_source_triangles, tcs_source_triangles, tcs_source_triangles
        };

        static const char * const * tes_sources[] =
        {
            tes_source_equal, tes_source_fract_even, tes_source_fract_odd
        };

        static const char * const * fs_sources[] =
        {
            fs_source, fs_source, fs_source
        };

        for (i = 0; i < 3; i++)
        {
            program[i] = glCreateProgram();
            GLuint vs = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vs, 1, vs_sources[i], NULL);
            glCompileShader(vs);

            GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
            glShaderSource(tcs, 1, tcs_sources[i], NULL);
            glCompileShader(tcs);

            GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
            glShaderSource(tes, 1, tes_sources[i], NULL);
            glCompileShader(tes);

            GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fs, 1, fs_sources[i], NULL);
            glCompileShader(fs);

            glAttachShader(program[i], vs);
            glAttachShader(program[i], tcs);
            glAttachShader(program[i], tes);
            glAttachShader(program[i], fs);
            glLinkProgram(program[i]);

            glDeleteShader(vs);
            glDeleteShader(tcs);
            glDeleteShader(tes);
            glDeleteShader(fs);
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glPatchParameteri(GL_PATCH_VERTICES, 4);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    virtual void render(double currentTime)
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glClearBufferfv(GL_COLOR, 0, black);

        glUseProgram(program[program_index]);
        // glUniform1f(0, sinf((float)currentTime) * 5.0f + 6.0f);
        glUniform1f(0, 5.3f);
        glDrawArrays(GL_PATCHES, 0, 4);
    }

    virtual void shutdown()
    {
        int i;
        glDeleteVertexArrays(1, &vao);

        for (i = 0; i < 3; i++)
        {
            glDeleteProgram(program[i]);
        }
    }

    void onKey(int key, int action)
    {
        if (!action)
            return;

        switch (key)
        {
            case 'M': program_index = (program_index + 1) % 3;
                break;
        }
    }

private:
    GLuint          program[3];
    int             program_index;
    GLuint          vao;
};

DECLARE_MAIN(tesssubdivmodes_app)
