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
#include <sb7ktx.h>

static const char * grass_vs_source[] =
{
    "// Vertex Shader                                                                                            \n"
    "// Graham Sellers                                                                                           \n"
    "// OpenGL SuperBible                                                                                        \n"
    "#version 420 core                                                                                           \n"
    "                                                                                                            \n"
    "// Incoming per vertex position                                                                             \n"
    "in vec4 vVertex;                                                                                            \n"
    "                                                                                                            \n"
    "// Output varyings                                                                                          \n"
    "out vec4 color;                                                                                             \n"
    "                                                                                                            \n"
    "uniform mat4 mvpMatrix;                                                                                     \n"
    "                                                                                                            \n"
    "layout (binding = 0) uniform sampler1D grasspalette_texture;                                                \n"
    "layout (binding = 1) uniform sampler2D length_texture;                                                      \n"
    "layout (binding = 2) uniform sampler2D orientation_texture;                                                 \n"
    "layout (binding = 3) uniform sampler2D grasscolor_texture;                                                  \n"
    "layout (binding = 4) uniform sampler2D bend_texture;                                                        \n"
    "                                                                                                            \n"
    "int random(int seed, int iterations)                                                                        \n"
    "{                                                                                                           \n"
    "    int value = seed;                                                                                       \n"
    "    int n;                                                                                                  \n"
    "                                                                                                            \n"
    "    for (n = 0; n < iterations; n++) {                                                                      \n"
    "        value = ((value >> 7) ^ (value << 9)) * 15485863;                                                   \n"
    "    }                                                                                                       \n"
    "                                                                                                            \n"
    "    return value;                                                                                           \n"
    "}                                                                                                           \n"
    "                                                                                                            \n"
    "vec4 random_vector(int seed)                                                                                \n"
    "{                                                                                                           \n"
    "    int r = random(gl_InstanceID, 4);                                                                       \n"
    "    int g = random(r, 2);                                                                                   \n"
    "    int b = random(g, 2);                                                                                   \n"
    "    int a = random(b, 2);                                                                                   \n"
    "                                                                                                            \n"
    "    return vec4(float(r & 0x3FF) / 1024.0,                                                                  \n"
    "                float(g & 0x3FF) / 1024.0,                                                                  \n"
    "                float(b & 0x3FF) / 1024.0,                                                                  \n"
    "                float(a & 0x3FF) / 1024.0);                                                                 \n"
    "}                                                                                                           \n"
    "                                                                                                            \n"
    "mat4 construct_rotation_matrix(float angle)                                                                 \n"
    "{                                                                                                           \n"
    "    float st = sin(angle);                                                                                  \n"
    "    float ct = cos(angle);                                                                                  \n"
    "                                                                                                            \n"
    "    return mat4(vec4(ct, 0.0, st, 0.0),                                                                     \n"
    "                vec4(0.0, 1.0, 0.0, 0.0),                                                                   \n"
    "                vec4(-st, 0.0, ct, 0.0),                                                                    \n"
    "                vec4(0.0, 0.0, 0.0, 1.0));                                                                  \n"
    "}                                                                                                           \n"
    "                                                                                                            \n"
    "void main(void)                                                                                             \n"
    "{                                                                                                           \n"
    "    vec4 offset = vec4(float(gl_InstanceID >> 10) - 512.0,                                                  \n"
    "                       0.0f,                                                                                \n"
    "                       float(gl_InstanceID & 0x3FF) - 512.0,                                                \n"
    "                       0.0f);                                                                               \n"
    "    int number1 = random(gl_InstanceID, 3);                                                                 \n"
    "    int number2 = random(number1, 2);                                                                       \n"
    "    offset += vec4(float(number1 & 0xFF) / 256.0,                                                           \n"
    "                   0.0f,                                                                                    \n"
    "                   float(number2 & 0xFF) / 256.0,                                                           \n"
    "                   0.0f);                                                                                   \n"
    "    // float angle = float(random(number2, 2) & 0x3FF) / 1024.0;                                            \n"
    "                                                                                                            \n"
    "    vec2 texcoord = offset.xz / 1024.0 + vec2(0.5);                                                         \n"
    "                                                                                                            \n"
    "    // float bend_factor = float(random(number2, 7) & 0x3FF) / 1024.0;                                      \n"
    "    float bend_factor = texture(bend_texture, texcoord).r * 2.0;                                            \n"
    "    float bend_amount = cos(vVertex.y);                                                                     \n"
    "                                                                                                            \n"
    "    float angle = texture(orientation_texture, texcoord).r * 2.0 * 3.141592;                                \n"
    "    mat4 rot = construct_rotation_matrix(angle);                                                            \n"
    "    vec4 position = (rot * (vVertex + vec4(0.0, 0.0, bend_amount * bend_factor, 0.0))) + offset;            \n"
    "                                                                                                            \n"
    "    position *= vec4(1.0, texture(length_texture, texcoord).r * 0.9 + 0.3, 1.0, 1.0);                       \n"
    "                                                                                                            \n"
    "    gl_Position = mvpMatrix * position; // (rot * position);                                                \n"
    "    // color = vec4(random_vector(gl_InstanceID).xyz * vec3(0.1, 0.5, 0.1) + vec3(0.1, 0.4, 0.1), 1.0);     \n"
    "    // color = texture(orientation_texture, texcoord);                                                      \n"
    "    color = texture(grasspalette_texture, texture(grasscolor_texture, texcoord).r) +                        \n"
    "            vec4(random_vector(gl_InstanceID).xyz * vec3(0.1, 0.5, 0.1), 1.0);                              \n"
    "}                                                                                                           \n"
};

static const char * grass_fs_source[] =
{
    "// Fragment Shader               \n"
    "// Graham Sellers                \n"
    "// OpenGL SuperBible             \n"
    "#version 420 core                \n"
    "                                 \n"
    "in vec4 color;                   \n"
    "                                 \n"
    "out vec4 output_color;           \n"
    "                                 \n"
    "void main(void)                  \n"
    "{                                \n"
    "    output_color = color;        \n"
    "}                                \n"
};

class grass_app : public sb7::application
{
public:
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Grass";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup(void)
    {
        static const GLfloat grass_blade[] =
        {
            -0.3f, 0.0f,
             0.3f, 0.0f,
            -0.20f, 1.0f,
             0.1f, 1.3f,
            -0.05f, 2.3f,
             0.0f, 3.3f
        };

        glGenBuffers(1, &grass_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, grass_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(grass_blade), grass_blade, GL_STATIC_DRAW);

        glGenVertexArrays(1, &grass_vao);
        glBindVertexArray(grass_vao);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        grass_program = glCreateProgram();
        GLuint grass_vs = glCreateShader(GL_VERTEX_SHADER);
        GLuint grass_fs = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(grass_vs, 1, grass_vs_source, NULL);
        glShaderSource(grass_fs, 1, grass_fs_source, NULL);

        glCompileShader(grass_vs);
        glCompileShader(grass_fs);

        glAttachShader(grass_program, grass_vs);
        glAttachShader(grass_program, grass_fs);

        glLinkProgram(grass_program);
        glDeleteShader(grass_fs);
        glDeleteShader(grass_vs);

        uniforms.mvpMatrix = glGetUniformLocation(grass_program, "mvpMatrix");

        glActiveTexture(GL_TEXTURE1);
        tex_grass_length = sb7::ktx::file::load("media/textures/grass_length.ktx");
        glActiveTexture(GL_TEXTURE2);
        tex_grass_orientation = sb7::ktx::file::load("media/textures/grass_orientation.ktx");
        glActiveTexture(GL_TEXTURE3);
        tex_grass_color = sb7::ktx::file::load("media/textures/grass_color.ktx");
        glActiveTexture(GL_TEXTURE4);
        tex_grass_bend = sb7::ktx::file::load("media/textures/grass_bend.ktx");
    }

    void shutdown(void)
    {
        glDeleteProgram(grass_program);
    }

    void render(double currentTime)
    {
        float t = (float)currentTime * 0.02f;
        float r = 550.0f;

        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        glClearBufferfv(GL_COLOR, 0, black);
        glClearBufferfv(GL_DEPTH, 0, &one);

        vmath::mat4 mv_matrix = vmath::lookat(vmath::vec3(sinf(t) * r, 25.0f, cosf(t) * r),
                                              vmath::vec3(0.0f, -50.0f, 0.0f),
                                              vmath::vec3(0.0, 1.0, 0.0));
        vmath::mat4 prj_matrix = vmath::perspective(45.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);

        glUseProgram(grass_program);
        glUniformMatrix4fv(uniforms.mvpMatrix, 1, GL_FALSE, (prj_matrix * mv_matrix));

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glBindVertexArray(grass_vao);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 6, 1024 * 1024);
    }

private:
    GLuint      grass_buffer;
    GLuint      grass_vao;

    GLuint      grass_program;

    GLuint      tex_grass_color;
    GLuint      tex_grass_length;
    GLuint      tex_grass_orientation;
    GLuint      tex_grass_bend;

    struct
    {
        GLint   mvpMatrix;
    } uniforms;
};

DECLARE_MAIN(grass_app);
