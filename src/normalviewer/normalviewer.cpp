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

#include <object.h>

class sb6mrender_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Normal Viewer";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        static const char * vs_source[] =
        {
            "#version 410 core                                                  \n"
            "                                                                   \n"
            "layout (location = 0) in vec4 position;                            \n"
            "layout (location = 1) in vec3 normal;                              \n"
            "                                                                   \n"
            "out VS_OUT                                                         \n"
            "{                                                                  \n"
            "    vec3 normal;                                                   \n"
            "    vec4 color;                                                    \n"
            "} vs_out;                                                          \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    gl_Position = position;                                        \n"
            "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);      \n"
            "    vs_out.normal = normalize(normal);                             \n"
            "}                                                                  \n"
        };

        static const char * gs_source[] =
        {
            "#version 410 core                                                      \n"
            "                                                                       \n"
            "layout (triangles) in;                                                 \n"
            "layout (line_strip, max_vertices = 4) out;                             \n"
            "                                                                       \n"
            "uniform mat4 mv_matrix;                                                \n"
            "uniform mat4 proj_matrix;                                              \n"
            "                                                                       \n"
            "in VS_OUT                                                              \n"
            "{                                                                      \n"
            "    vec3 normal;                                                       \n"
            "    vec4 color;                                                        \n"
            "} gs_in[];                                                             \n"
            "                                                                       \n"
            "out GS_OUT                                                             \n"
            "{                                                                      \n"
            "    vec3 normal;                                                       \n"
            "    vec4 color;                                                        \n"
            "} gs_out;                                                              \n"
            "                                                                       \n"
            "uniform float normal_length = 0.2;                                     \n"
            "                                                                       \n"
            "void main(void)                                                        \n"
            "{                                                                      \n"
            "    mat4 mvp = proj_matrix * mv_matrix;                                \n"
            "    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;     \n"
            "    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;     \n"
            "    vec3 face_normal = normalize(cross(ab, ac));                      \n"
            "                                                                       \n"
            "    vec4 tri_centroid = (gl_in[0].gl_Position +                        \n"
            "                         gl_in[1].gl_Position +                        \n"
            "                         gl_in[2].gl_Position) / 3.0;                  \n"
            "                                                                       \n"
            "    gl_Position = mvp * tri_centroid;                                  \n"
            "    gs_out.normal = gs_in[0].normal;                                   \n"
            "    gs_out.color = gs_in[0].color;                                     \n"
            "    EmitVertex();                                                      \n"
            "                                                                       \n"
            "    gl_Position = mvp * (tri_centroid +                                \n"
            "                         vec4(face_normal * normal_length, 0.0));      \n"
            "    gs_out.normal = gs_in[0].normal;                                   \n"
            "    gs_out.color = gs_in[0].color;                                     \n"
            "    EmitVertex();                                                      \n"
            "    EndPrimitive();                                                    \n"
            "                                                                       \n"
            "    gl_Position = mvp * gl_in[0].gl_Position;                          \n"
            "    gs_out.normal = gs_in[0].normal;                                   \n"
            "    gs_out.color = gs_in[0].color;                                     \n"
            "    EmitVertex();                                                      \n"
            "                                                                       \n"
            "    gl_Position = mvp * (gl_in[0].gl_Position +                        \n"
            "                         vec4(gs_in[0].normal * normal_length, 0.0));  \n"
            "    gs_out.normal = gs_in[0].normal;                                   \n"
            "    gs_out.color = gs_in[0].color;                                     \n"
            "    EmitVertex();                                                      \n"
            "    EndPrimitive();                                                    \n"
            "}                                                                      \n"
        };

        static const char * fs_source[] =
        {
            "#version 410 core                                                  \n"
            "                                                                   \n"
            "out vec4 color;                                                    \n"
            "                                                                   \n"
            "in GS_OUT                                                          \n"
            "{                                                                  \n"
            "    vec3 normal;                                                   \n"
            "    vec4 color;                                                    \n"
            "} fs_in;                                                           \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    color = vec4(1.0) * abs(normalize(fs_in.normal).z);            \n"
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
        normal_length_location = glGetUniformLocation(program, "normal_length");

        object.load("media/objects/torus.sbm");

        // glEnable(GL_CULL_FACE);
        //glCullFace(GL_FRONT);

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
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -3.0f) *
                                vmath::rotate((float)currentTime * 15.0f, 0.0f, 1.0f, 0.0f) *
                                vmath::rotate((float)currentTime * 21.0f, 1.0f, 0.0f, 0.0f);
        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);

        glUniform1f(normal_length_location, sinf((float)currentTime * 8.0f) * cosf((float)currentTime * 6.0f) * 0.03f + 0.05f);

        object.render();
    }

    virtual void shutdown()
    {
        object.free();
        glDeleteProgram(program);
    }

private:
    GLuint          program;
    GLint           mv_location;
    GLint           proj_location;
    GLint           normal_length_location;

    sb7::object     object;
};

DECLARE_MAIN(sb6mrender_app)
