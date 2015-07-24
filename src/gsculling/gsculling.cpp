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

class gsculling_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Geometry Shader Culling";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        static const char * vs_source[] =
        {
            "#version 410 core                                                                       \n"
            "                                                                                        \n"
            "// Incoming per vertex... position and normal                                           \n"
            "layout (location = 0) in vec4 vVertex;                                                  \n"
            "layout (location = 1) in vec3 vNormal;                                                  \n"
            "                                                                                        \n"
            "out Vertex                                                                              \n"
            "{                                                                                       \n"
            "    vec3 normal;                                                                        \n"
            "    vec4 color;                                                                         \n"
            "} vertex;                                                                               \n"
            "                                                                                        \n"
            "uniform vec3 vLightPosition = vec3(-10.0, 40.0, 200.0);                                 \n"
            "uniform mat4 mvMatrix;                                                                  \n"
            "                                                                                        \n"
            "void main(void)                                                                         \n"
            "{                                                                                       \n"
            "    // Get surface normal in eye coordinates                                            \n"
            "    vec3 vEyeNormal = mat3(mvMatrix) * normalize(vNormal);                              \n"
            "                                                                                        \n"
            "    // Get vertex position in eye coordinates                                           \n"
            "    vec4 vPosition4 = mvMatrix * vVertex;                                               \n"
            "    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;                                    \n"
            "                                                                                        \n"
            "    // Get vector to light source                                                       \n"
            "    vec3 vLightDir = normalize(vLightPosition - vPosition3);                            \n"
            "                                                                                        \n"
            "    // Dot product gives us diffuse intensity                                           \n"
            "    vertex.color = vec4(0.7, 0.6, 1.0, 1.0) * abs(dot(vEyeNormal, vLightDir));          \n"
            "                                                                                        \n"
            "    gl_Position = vVertex;                                                              \n"
            "    vertex.normal = vNormal;                                                            \n"
            "}                                                                                       \n"
        };

        static const char * gs_source[] =
        {
            "#version 410 core                                                       \n"
            "                                                                        \n"
            "layout (triangles) in;                                                  \n"
            "layout (triangle_strip, max_vertices = 3) out;                          \n"
            "                                                                        \n"
            "in Vertex                                                               \n"
            "{                                                                       \n"
            "    vec3 normal;                                                        \n"
            "    vec4 color;                                                         \n"
            "} vertex[];                                                             \n"
            "                                                                        \n"
            "out vec4 color;                                                         \n"
            "                                                                        \n"
            "uniform vec3 vLightPosition;                                            \n"
            "uniform mat4 mvpMatrix;                                                 \n"
            "uniform mat4 mvMatrix;                                                  \n"
            "                                                                        \n"
            "uniform vec3 viewpoint;                                                 \n"
            "                                                                        \n"
            "void main(void)                                                         \n"
            "{                                                                       \n"
            "    int n;                                                              \n"
            "                                                                        \n"
            "    vec3 ab = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;      \n"
            "    vec3 ac = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;      \n"
            "    vec3 normal = normalize(cross(ab, ac));                             \n"
            "    vec3 transformed_normal = (mat3(mvMatrix) * normal);                \n"
            "    vec4 worldspace = /* mvMatrix * */ gl_in[0].gl_Position;            \n"
            "    vec3 vt = normalize(viewpoint - worldspace.xyz);                    \n"
            "                                                                        \n"
            "    if (dot(normal, vt) > 0.0) {                                        \n"
            "        for (n = 0; n < 3; n++) {                                       \n"
            "            gl_Position = mvpMatrix * gl_in[n].gl_Position;             \n"
            "            color = vertex[n].color;                                    \n"
            "            EmitVertex();                                               \n"
            "        }                                                               \n"
            "        EndPrimitive();                                                 \n"
            "    }                                                                   \n"
            "}                                                                       \n"
        };

        static const char * fs_source[] =
        {
            "#version 410 core                                                       \n"
            "                                                                        \n"
            "in vec4 color;                                                          \n"
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
        viewpoint_location = glGetUniformLocation(program, "viewpoint");

        object.load("media/objects/dragon.sbm");

        glDisable(GL_CULL_FACE);

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
        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -20.0f) *
                                vmath::rotate((float)currentTime * 5.0f, 0.0f, 1.0f, 0.0f) *
                                vmath::rotate((float)currentTime * 100.0f, 1.0f, 0.0f, 0.0f);

        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, proj_matrix * mv_matrix);

        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);

        GLfloat vViewpoint[] = { sinf(f * 2.1f) * 70.0f, cosf(f * 1.4f) * 70.0f, sinf(f * 0.7f) * 70.0f };
        glUniform3fv(viewpoint_location, 1, vViewpoint);

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
    GLint           mvp_location;
    GLint           viewpoint_location;

    sb7::object     object;
};

DECLARE_MAIN(gsculling_app)
