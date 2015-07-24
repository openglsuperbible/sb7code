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

// Remove this to draw only a single cube!
#define MANY_CUBES

class tessellatedcube_app : public sb7::application
{
public:
    tessellatedcube_app()
        : wireframe_mode(false)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Tessellated Cube";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        static const char * vs_source[] =
        {
            "#version 420 core                                                  \n"
            "                                                                   \n"
            "in vec4 position;                                                  \n"
            "                                                                   \n"
            /* "out VS_OUT                                                         \n"
            "{                                                                  \n"
            "    vec4 color;                                                    \n"
            "} vs_out;                                                          \n"*/
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    gl_Position = position;              \n"
            // "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);      \n"
            "}                                                                  \n"
        };

#if 0
        static const char * tcs_source[] =
        {
            "#version 420 core                                                                 \n"
            "                                                                                  \n"
            "layout (vertices = 4) out;                                                        \n"
            "                                                                                  \n"
            /* "in VS_OUT                                                                         \n"
            "{                                                                                 \n"
            "    vec4 color;                                                                   \n"
            "} vs_in[];                                                                        \n"*/
            "                                                                                  \n"
            "void main(void)                                                                   \n"
            "{                                                                                 \n"
            "    if (gl_InvocationID == 0)                                                     \n"
            "    {                                                                             \n"
            "        gl_TessLevelInner[0] = 4.0;                                               \n"
            "        gl_TessLevelInner[1] = 4.0;                                               \n"
            "        gl_TessLevelOuter[0] = 4.0;                                               \n"
            "        gl_TessLevelOuter[1] = 4.0;                                               \n"
            "        gl_TessLevelOuter[2] = 4.0;                                               \n"
            "        gl_TessLevelOuter[3] = 4.0;                                               \n"
            "    }                                                                             \n"
            "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;     \n"
            "}                                                                                 \n"
        };
#endif

        static const char * tcs_source[] =
        {
            "#version 420 core                                                                 \n"
            "                                                                                  \n"
            "layout (vertices = 4) out;                                                        \n"
            "                                                                                  \n"
            /* "in VS_OUT                                                                         \n"
            "{                                                                                 \n"
            "    vec4 color;                                                                   \n"
            "} vs_in[];                                                                        \n"*/
            "                                                                                  \n"
            "uniform mat4 mv_matrix;                                                           \n"
            "uniform mat4 proj_matrix;                                                         \n"
            "                                                                                  \n"
            "void main(void)                                                                   \n"
            "{                                                                                 \n"
            "    vec4 pos[4];                                                                  \n"
            "    float tf[4];                                                                  \n"
            "    float t = 0.0;                                                                \n"
            "                                                                                  \n"
            "    int i;                                                                        \n"
            "                                                                                  \n"
            "    if (gl_InvocationID == 0)                                                     \n"
            "    {                                                                             \n"
            "        for (i = 0; i < 4; i++)                                                   \n"
            "        {                                                                         \n"
            "            pos[i] = proj_matrix * mv_matrix * gl_in[i].gl_Position;              \n"
            "        }                                                                         \n"
            "                                                                                  \n"
            "        tf[0] = max(2.0, distance(pos[0].xy / pos[0].w,                           \n"
            "                                  pos[1].xy / pos[1].w) * 16.0);                  \n"
            "        tf[1] = max(2.0, distance(pos[1].xy / pos[1].w,                           \n"
            "                                  pos[3].xy / pos[3].w) * 16.0);                  \n"
            "        tf[2] = max(2.0, distance(pos[2].xy / pos[2].w,                           \n"
            "                                  pos[3].xy / pos[3].w) * 16.0);                  \n"
            "        tf[3] = max(2.0, distance(pos[2].xy / pos[2].w,                           \n"
            "                                  pos[0].xy / pos[0].w) * 16.0);                  \n"
            "        for (i = 0; i < 4; i++)                                                   \n"
            "        {                                                                         \n"
            "            t = max(t, tf[i]);                                                           \n"
            "        }                                                                         \n"
            "                                                                                  \n"
            "        gl_TessLevelInner[0] = t;                             \n"
            "        gl_TessLevelInner[1] = t;                             \n"
            "        gl_TessLevelOuter[0] = tf[0];                                             \n"
            "        gl_TessLevelOuter[1] = tf[1];                                             \n"
            "        gl_TessLevelOuter[2] = tf[2];                                             \n"
            "        gl_TessLevelOuter[3] = tf[3];                                             \n"
            "    }                                                                             \n"
            "                                                                                  \n"
            "    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;     \n"
            "}                                                                                 \n"
        };

        static const char * tes_source[] =
        {
            "#version 420 core                                                                 \n"
            "                                                                                  \n"
            "layout (quads, fractional_odd_spacing, ccw) in;                                   \n"
            "                                                                                  \n"
            "uniform mat4 mv_matrix;                                                           \n"
            "uniform mat4 proj_matrix;                                                         \n"
            "                                                                                  \n"
            "out vec3 normal;                                                                  \n"
            "                                                                                  \n"
            "void main(void)                                                                   \n"
            "{                                                                                 \n"
            "    vec4 mid1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);  \n"
            "    vec4 mid2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);  \n"
            "    vec4 pos = mix(mid1, mid2, gl_TessCoord.y);                                   \n"
            "    pos.xyz = /* normalize*/(pos.xyz) * 0.25;                                          \n"
            "    normal = normalize(mat3(mv_matrix) * pos.xyz);                                \n"
            "    gl_Position = proj_matrix * mv_matrix * pos;                                  \n"
            "}                                                                                 \n"
        };

        static const char * fs_source[] =
        {
            "#version 420 core                                                  \n"
            "                                                                   \n"
            "out vec4 color;                                                    \n"
            "                                                                   \n"
            "in vec3 normal;                                                    \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    color = vec4(abs(normal), 1.0);                                \n"
            "}                                                                  \n"
        };

        program = glCreateProgram();
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vs_source, NULL);
        glCompileShader(vs);

        GLuint tcs = glCreateShader(GL_TESS_CONTROL_SHADER);
        glShaderSource(tcs, 1, tcs_source, NULL);
        glCompileShader(tcs);

        GLuint tes = glCreateShader(GL_TESS_EVALUATION_SHADER);
        glShaderSource(tes, 1, tes_source, NULL);
        glCompileShader(tes);

        glAttachShader(program, vs);
        glAttachShader(program, tcs);
        glAttachShader(program, tes);
        glAttachShader(program, fs);

        glLinkProgram(program);

        mv_location = glGetUniformLocation(program, "mv_matrix");
        proj_location = glGetUniformLocation(program, "proj_matrix");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        static const GLushort vertex_indices[] =
        {
            0, 1, 2, 3,
            2, 3, 4, 5,
            4, 5, 6, 7,
            6, 7, 0, 1,
            0, 2, 6, 4,
            1, 7, 3, 5
        };

        static const GLfloat vertex_positions[] =
        {
            -0.25f, -0.25f, -0.25f,
            -0.25f,  0.25f, -0.25f,
             0.25f, -0.25f, -0.25f,
             0.25f,  0.25f, -0.25f,
             0.25f, -0.25f,  0.25f,
             0.25f,  0.25f,  0.25f,
            -0.25f, -0.25f,  0.25f,
            -0.25f,  0.25f,  0.25f,
        };

        glGenBuffers(1, &position_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(vertex_positions),
                     vertex_positions,
                     GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(vertex_indices),
                     vertex_indices,
                     GL_STATIC_DRAW);

        glEnable(GL_CULL_FACE);
        // glFrontFace(GL_CW);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    virtual void render(double currentTime)
    {
        int i;
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, green);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(program);

        vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f,
                                                     1000.0f);
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);
        glPatchParameteri(GL_PATCH_VERTICES, 4);

        glPolygonMode(GL_FRONT_AND_BACK, wireframe_mode ? GL_LINE : GL_FILL);

#ifdef MANY_CUBES
        for (i = 0; i < 100; i++)
        {
            float f = (float)i + (float)currentTime * 0.03f;
            vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -10.0f) *
                                    vmath::translate(sinf(2.1f * f) * 4.0f,
                                                     cosf(1.7f * f) * 4.0f,
                                                     sinf(4.3f * f) * cosf(3.5f * f) * 30.0f) *
                                    vmath::rotate((float)currentTime * 3.0f, 1.0f, 0.0f, 0.0f) *
                                    vmath::rotate((float)currentTime * 5.0f, 0.0f, 1.0f, 0.0f);
            glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
            glDrawElements(GL_PATCHES, 24, GL_UNSIGNED_SHORT, 0);
        }
#else
        float f = (float)currentTime * 0.3f;
        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -3.0f) *
                                vmath::translate(0.0f, // sinf(2.1f * f) * 0.5f,
                                                 0.0f, // cosf(1.7f * f) * 0.5f,
                                                 sinf(1.3f * f) * cosf(1.5f * f) * 15.0f) *
                                // vmath::rotate((float)currentTime * 45.0f, 0.0f, 1.0f, 0.0f) *
                                vmath::rotate((float)currentTime * 81.0f, 1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);
        glDrawElements(GL_PATCHES, 24, GL_UNSIGNED_SHORT, 0);
#endif
    }

    virtual void shutdown()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
        glDeleteBuffers(1, &position_buffer);
    }

    virtual void onKey(int key, int action)
    {
        if (action == 1)
        {
            switch (key)
            {
                case 'W':   wireframe_mode = !wireframe_mode;
                    break;
            }
        }
    }

private:
    GLuint          program;
    GLuint          vao;
    GLuint          position_buffer;
    GLuint          index_buffer;
    GLint           mv_location;
    GLint           proj_location;

    bool            wireframe_mode;
};

DECLARE_MAIN(tessellatedcube_app)
