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
// #define MANY_CUBES

class multiscissor_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Multiple Scissors";

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
            "out VS_OUT                                                         \n"
            "{                                                                  \n"
            "    vec4 color;                                                    \n"
            "} vs_out;                                                          \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    gl_Position = position;                                        \n"
            "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);      \n"
            "}                                                                  \n"
        };

        static const char * gs_source[] =
        {
            "#version 420 core                                                  \n"
            "                                                                   \n"
            "layout (triangles, invocations = 4) in;                            \n"
            "layout (triangle_strip, max_vertices = 3) out;                     \n"
            "                                                                   \n"
            "layout (std140, binding = 0) uniform transform_block               \n"
            "{                                                                  \n"
            "    mat4 mvp_matrix[4];                                            \n"
            "};                                                                 \n"
            "                                                                   \n"
            "in VS_OUT                                                          \n"
            "{                                                                  \n"
            "    vec4 color;                                                    \n"
            "} gs_in[];                                                         \n"
            "                                                                   \n"
            "out GS_OUT                                                         \n"
            "{                                                                  \n"
            "    vec4 color;                                                    \n"
            "} gs_out;                                                          \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    for (int i = 0; i < gl_in.length(); i++)                       \n"
            "    {                                                              \n"
            "        gs_out.color = gs_in[i].color;                             \n"
            "        gl_Position = mvp_matrix[gl_InvocationID] *                \n"
            "                      gl_in[i].gl_Position;                        \n"
            "        gl_ViewportIndex = gl_InvocationID;                        \n"
            "        EmitVertex();                                              \n"
            "    }                                                              \n"
            "    EndPrimitive();                                                \n"
            "}                                                                  \n"
        };

        static const char * fs_source[] =
        {
            "#version 420 core                                                  \n"
            "                                                                   \n"
            "out vec4 color;                                                    \n"
            "                                                                   \n"
            "in GS_OUT                                                          \n"
            "{                                                                  \n"
            "    vec4 color;                                                    \n"
            "} fs_in;                                                           \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    color = fs_in.color;                                           \n"
            "}                                                                  \n"
        };

        program = glCreateProgram();
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vs_source, NULL);
        glCompileShader(vs);

        GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gs, 1, gs_source, NULL);
        glCompileShader(gs);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        glAttachShader(program, vs);
        glAttachShader(program, gs);
        glAttachShader(program, fs);

        glLinkProgram(program);

        mv_location = glGetUniformLocation(program, "mv_matrix");
        proj_location = glGetUniformLocation(program, "proj_matrix");

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        static const GLushort vertex_indices[] =
        {
            0, 1, 2,
            2, 1, 3,
            2, 3, 4,
            4, 3, 5,
            4, 5, 6,
            6, 5, 7,
            6, 7, 0,
            0, 7, 1,
            6, 0, 2,
            2, 4, 6,
            7, 5, 3,
            7, 3, 1
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

        glGenBuffers(1, &uniform_buffer);
        glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
        glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(vmath::mat4), NULL, GL_DYNAMIC_DRAW);

        glEnable(GL_CULL_FACE);
        // glFrontFace(GL_CW);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    virtual void render(double currentTime)
    {
        int i;
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;

        glDisable(GL_SCISSOR_TEST);

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        // Turn on scissor testing
        glEnable(GL_SCISSOR_TEST);

        // Each rectangle will be 7/16 of the screen
        int scissor_width = (7 * info.windowWidth) / 16;
        int scissor_height = (7 * info.windowHeight) / 16;

        // Four rectangles - lower left first...
        glScissorIndexed(0,
                         0, 0,
                         scissor_width, scissor_height);

        // Lower right...
        glScissorIndexed(1,
                         info.windowWidth - scissor_width, 0,
                         scissor_width, scissor_height);

        // Upper left...
        glScissorIndexed(2,
                         0, info.windowHeight - scissor_height,
                         scissor_width, scissor_height);

        // Upper right...
        glScissorIndexed(3,
                         info.windowWidth - scissor_width,
                         info.windowHeight - scissor_height,
                         scissor_width, scissor_height);

        glUseProgram(program);

        vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f,
                                                     1000.0f);

        float f = (float)currentTime * 0.3f;

        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_buffer);
        vmath::mat4 * mv_matrix_array = (vmath::mat4 *)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                                        0,
                                                                        4 * sizeof(vmath::mat4),
                                                                        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (i = 0; i < 4; i++)
        {
            mv_matrix_array[i] = proj_matrix *
                                 vmath::translate(0.0f, 0.0f, -2.0f) *
                                 vmath::rotate((float)currentTime * 45.0f * (float)(i + 1), 0.0f, 1.0f, 0.0f) *
                                 vmath::rotate((float)currentTime * 81.0f * (float)(i + 1), 1.0f, 0.0f, 0.0f);
        }

        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
    }

    virtual void shutdown()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(program);
        glDeleteBuffers(1, &position_buffer);
    }

private:
    GLuint          program;
    GLuint          vao;
    GLuint          position_buffer;
    GLuint          index_buffer;
    GLuint          uniform_buffer;
    GLint           mv_location;
    GLint           proj_location;
};

DECLARE_MAIN(multiscissor_app)
