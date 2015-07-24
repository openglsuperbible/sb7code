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

#include <cmath>

class gstessellate_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Geometry Shader Tessellation";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        static const char * vs_source[] =
        {
            "// Vertex Shader                                                                          \n"
            "// OpenGL SuperBible                                                                      \n"
            "#version 410 core                                                                         \n"
            "                                                                                          \n"
            "// Incoming per vertex... position and normal                                             \n"
            "in vec4 vVertex;                                                                          \n"
            "                                                                                          \n"
            "void main(void)                                                                           \n"
            "{                                                                                         \n"
            "    gl_Position = vVertex;                                                                \n"
            "}                                                                                         \n"
        };

        static const char * gs_source[] =
        {
            "// Geometry Shader                                                            \n"
            "// Graham Sellers                                                             \n"
            "// OpenGL SuperBible                                                          \n"
            "#version 410 core                                                             \n"
            "                                                                              \n"
            "                                                                              \n"
            "layout (triangles) in;                                                        \n"
            "layout (triangle_strip, max_vertices = 12) out;                                \n"
            "                                                                              \n"
            "uniform float stretch = 0.7;                                                  \n"
            "                                                                              \n"
            "flat out vec4 color;                                                          \n"
            "                                                                              \n"
            "uniform mat4 mvpMatrix;                                                       \n"
            "uniform mat4 mvMatrix;                                                        \n"
            "                                                                              \n"
            "void make_face(vec3 a, vec3 b, vec3 c)                                        \n"
            "{                                                                             \n"
            "    vec3 face_normal = normalize(cross(c - a, c - b));                        \n"
            "    vec4 face_color = vec4(1.0, 0.4, 0.7, 1.0) * (mat3(mvMatrix) * face_normal).z;  \n"
            "    gl_Position = mvpMatrix * vec4(a, 1.0);                                   \n"
            "    color = face_color;                                                       \n"
            "    EmitVertex();                                                             \n"
            "                                                                              \n"
            "    gl_Position = mvpMatrix * vec4(b, 1.0);                                   \n"
            "    color = face_color;                                                       \n"
            "    EmitVertex();                                                             \n"
            "                                                                              \n"
            "    gl_Position = mvpMatrix * vec4(c, 1.0);                                   \n"
            "    color = face_color;                                                       \n"
            "    EmitVertex();                                                             \n"
            "                                                                              \n"
            "    EndPrimitive();                                                           \n"
            "}                                                                             \n"
            "                                                                              \n"
            "void main(void)                                                               \n"
            "{                                                                             \n"
            "    int n;                                                                    \n"
            "    vec3 a = gl_in[0].gl_Position.xyz;                                        \n"
            "    vec3 b = gl_in[1].gl_Position.xyz;                                        \n"
            "    vec3 c = gl_in[2].gl_Position.xyz;                                        \n"
            "                                                                              \n"
            "    vec3 d = (a + b) * stretch;                                               \n"
            "    vec3 e = (b + c) * stretch;                                               \n"
            "    vec3 f = (c + a) * stretch;                                               \n"
            "                                                                              \n"
            "    a *= (2.0 - stretch);                                                     \n"
            "    b *= (2.0 - stretch);                                                     \n"
            "    c *= (2.0 - stretch);                                                     \n"

            "    make_face(a, d, f);                                                       \n"
            "    make_face(d, b, e);                                                       \n"
            "    make_face(e, c, f);                                                       \n"
            "    make_face(d, e, f);                                                       \n"

            "    EndPrimitive();                                                           \n"
            "}                                                                             \n"
        };

        static const char * fs_source[] =
        {
            "// Fragment Shader                                                      \n"
            "// Graham Sellers                                                       \n"
            "// OpenGL SuperBible                                                    \n"
            "#version 410 core                                                       \n"
            "                                                                        \n"
            "flat in vec4 color;                                                     \n"
            "                                                                        \n"
            "out vec4 output_color;                                                  \n"
            "                                                                        \n"
            "void main(void)                                                         \n"
            "{                                                                       \n"
            "    output_color = color;                                               \n"
            "}                                                                       \n"
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

        mv_location = glGetUniformLocation(program, "mvMatrix");
        mvp_location = glGetUniformLocation(program, "mvpMatrix");
        stretch_location = glGetUniformLocation(program, "stretch");

        static const GLfloat tetrahedron_verts[] =
        {
             0.000f,  0.000f,  1.000f,
             0.943f,  0.000f, -0.333f,
            -0.471f,  0.816f, -0.333f,
            -0.471f, -0.816f, -0.333f
        };

        static const GLushort tetrahedron_indices[] =
        {
            0, 1, 2,
            0, 2, 3,
            0, 3, 1,
            3, 2, 1
        };

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetrahedron_verts) + sizeof(tetrahedron_indices), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(tetrahedron_indices), tetrahedron_indices);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetrahedron_indices), sizeof(tetrahedron_verts), tetrahedron_verts);

        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(tetrahedron_indices));
        glEnableVertexAttribArray(0);

        glEnable(GL_CULL_FACE);
        // glDisable(GL_CULL_FACE);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }

    virtual void render(double currentTime)
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        float f = (float)currentTime;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(program);

        vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f,
                                                     1000.0f);
        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -8.0f) *
                                vmath::rotate((float)currentTime * 71.0f, 0.0f, 1.0f, 0.0f) *
                                vmath::rotate((float)currentTime * 10.0f, 1.0f, 0.0f, 0.0f);

        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, proj_matrix * mv_matrix);

        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);

        glUniform1f(stretch_location, sinf(f * 4.0f) * 0.75f + 1.0f);

        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, NULL);
    }

    virtual void shutdown()
    {
        glDeleteProgram(program);
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &buffer);
    }

private:
    GLuint          program;
    GLint           mv_location;
    GLint           mvp_location;
    GLint           stretch_location;
    GLuint          vao;
    GLuint          buffer;
};

DECLARE_MAIN(gstessellate_app)
