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
#include <sb7textoverlay.h>
#include <sb7color.h>

class tessmodes_app : public sb7::application
{
public:
    tessmodes_app()
        : program_index(0)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Tessellation Modes";

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
            "    const vec4 vertices[] = vec4[](vec4( 0.4, -0.4, 0.5, 1.0),    \n"
            "                                   vec4(-0.4, -0.4, 0.5, 1.0),    \n"
            "                                   vec4( 0.4,  0.4, 0.5, 1.0),    \n"
            "                                   vec4(-0.4,  0.4, 0.5, 1.0));   \n"
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
            "void main(void)                                                                   \n"
            "{                                                                                 \n"
            "    if (gl_InvocationID == 0)                                                     \n"
            "    {                                                                             \n"
            "        gl_TessLevelInner[0] = 5.0;                                               \n"
            "        gl_TessLevelOuter[0] = 8.0;                                               \n"
            "        gl_TessLevelOuter[1] = 8.0;                                               \n"
            "        gl_TessLevelOuter[2] = 8.0;                                               \n"
            "    }                                                                             \n"
            "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;     \n"
            "}                                                                                 \n"
        };

        static const char * tes_source_triangles[] =
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

        static const char * tes_source_triangles_as_points[] =
        {
            "#version 420 core                                                                 \n"
            "                                                                                  \n"
            "layout (triangles, point_mode) in;                                                \n"
            "                                                                                  \n"
            "void main(void)                                                                   \n"
            "{                                                                                 \n"
            "    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +                       \n"
            "                  (gl_TessCoord.y * gl_in[1].gl_Position) +                       \n"
            "                  (gl_TessCoord.z * gl_in[2].gl_Position);                        \n"
            "}                                                                                 \n"
        };

        static const char * tcs_source_quads[] =
        {
            "#version 420 core                                                                   \n"
            "                                                                                    \n"
            "layout (vertices = 4) out;                                                          \n"
            "                                                                                    \n"
            "void main(void)                                                                     \n"
            "{                                                                                   \n"
            "    if (gl_InvocationID == 0)                                                       \n"
            "    {                                                                               \n"
            "        gl_TessLevelInner[0] = 9.0;                                                 \n"
            "        gl_TessLevelInner[1] = 7.0;                                                 \n"
            "        gl_TessLevelOuter[0] = 3.0;                                                 \n"
            "        gl_TessLevelOuter[1] = 5.0;                                                 \n"
            "        gl_TessLevelOuter[2] = 3.0;                                                 \n"
            "        gl_TessLevelOuter[3] = 5.0;                                                 \n"
            "    }                                                                               \n"
            "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;       \n"
            "}                                                                                   \n"
        };

        static const char * tes_source_quads[] =
        {
            "#version 420 core                                                                    \n"
            "                                                                                     \n"
            "layout (quads) in;                                                                   \n"
            "                                                                                     \n"
            "void main(void)                                                                      \n"
            "{                                                                                    \n"
            "    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);       \n"
            "    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);       \n"
            "    gl_Position = mix(p1, p2, gl_TessCoord.y);                                       \n"
            "}                                                                                    \n"
        };

        static const char * tcs_source_isolines[] =
        {
            "#version 420 core                                                                   \n"
            "                                                                                    \n"
            "layout (vertices = 4) out;                                                          \n"
            "                                                                                    \n"
            "void main(void)                                                                     \n"
            "{                                                                                   \n"
            "    if (gl_InvocationID == 0)                                                       \n"
            "    {                                                                               \n"
            "        gl_TessLevelOuter[0] = 5.0;                                                 \n"
            "        gl_TessLevelOuter[1] = 5.0;                                                 \n"
            "    }                                                                               \n"
            "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;       \n"
            "}                                                                                   \n"
        };

        /*
        static const char * tes_source_isolines[] =
        {
            "#version 420 core                                                                    \n"
            "                                                                                     \n"
            "layout (isolines, equal_spacing, cw) in;                                             \n"
            "                                                                                     \n"
            "void main(void)                                                                      \n"
            "{                                                                                    \n"
            "    vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);       \n"
            "    vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);       \n"
            "    gl_Position = mix(p1, p2, gl_TessCoord.y);                                       \n"
            "}                                                                                    \n"
        };
        */

        static const char * tes_source_isolines[] =
        {
            "#version 420 core                                                                    \n"
            "                                                                                     \n"
            "layout (isolines) in;                                                                \n"
            "                                                                                     \n"
            "void main(void)                                                                      \n"
            "{                                                                                    \n"
            "    float r = (gl_TessCoord.y + gl_TessCoord.x / gl_TessLevelOuter[0]);              \n"
            "    float t = gl_TessCoord.x * 2.0 * 3.14159;                                        \n"
            "    gl_Position = vec4(sin(t) * r, cos(t) * r, 0.5, 1.0);                            \n"
            "}                                                                                    \n"
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
            vs_source, vs_source, vs_source, vs_source
        };

        static const char * const * tcs_sources[] =
        {
            tcs_source_quads, tcs_source_triangles, tcs_source_triangles, tcs_source_isolines
        };

        static const char * const * tes_sources[] =
        {
            tes_source_quads, tes_source_triangles, tes_source_triangles_as_points, tes_source_isolines
        };

        static const char * const * fs_sources[] =
        {
            fs_source, fs_source, fs_source, fs_source
        };

        overlay.init(80, 50);

        for (i = 0; i < 4; i++)
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
        glClearBufferfv(GL_COLOR, 0, sb7::color::Black);

        glUseProgram(program[program_index]);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_PATCHES, 0, 4);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        static const char * mode_names[] =
        {
            "QUADS", "TRIANGLES", "QUADS AS POINTS", "ISOLINES"
        };

        overlay.clear();
        overlay.print("Mode: ");
        overlay.print(mode_names[program_index]);
        overlay.print(" (M toggles)");

        overlay.draw();
    }

    virtual void shutdown()
    {
        int i;
        glDeleteVertexArrays(1, &vao);

        for (i = 0; i < 4; i++)
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
            case 'M': program_index = (program_index + 1) % 4;
                break;
        }
    }

private:
    GLuint          program[4];
    int             program_index;
    GLuint          vao;
    sb7::text_overlay   overlay;
};

DECLARE_MAIN(tessmodes_app)
