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

#include <cstdio>
#include <cstring>

static const struct type_to_name_entry
{
    GLenum type;
    const char * name;
}
type_to_name_table[] =
{
    GL_FLOAT,                       "float",
    GL_FLOAT_VEC2,                  "vec2",
    GL_FLOAT_VEC3,                  "vec3",
    GL_FLOAT_VEC4,                  "vec4",
    GL_DOUBLE,                      "double",
    GL_DOUBLE_VEC2,                 "dvec2",
    GL_DOUBLE_VEC3,                 "dvec3",
    GL_DOUBLE_VEC4,                 "dvec4",
    GL_INT,                         "int",
    GL_INT_VEC2,                    "ivec2",
    GL_INT_VEC3,                    "ivec3",
    GL_INT_VEC4,                    "ivec4",
    GL_UNSIGNED_INT,                "uint",
    GL_UNSIGNED_INT_VEC2,           "uvec2",
    GL_UNSIGNED_INT_VEC3,           "uvec3",
    GL_UNSIGNED_INT_VEC4,           "uvec4",
    GL_BOOL,                        "bool",
    GL_BOOL_VEC2,                   "bvec2",
    GL_BOOL_VEC3,                   "bvec3",
    GL_BOOL_VEC4,                   "bvec4",
    GL_NONE,                        NULL
};

static const char * type_to_name(GLenum type)
{
    for (const type_to_name_entry * ptr = &type_to_name_table[0]; ptr->name != NULL; ptr++)
    {
        if (ptr->type == type)
            return ptr->name;
    }

    return NULL;
}

class programinfo_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Program Information";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup()
    {
        static const char * vs_source[] =
        {
            "#version 420 core                                                  \n"
            "                                                                   \n"
            "in vec4 position;                                                  \n"
            "                                                                   \n"
            "uniform mat4 mv_matrix;                                            \n"
            "uniform mat4 proj_matrix;                                          \n"
            "                                                                   \n"
            "out VS_OUT                                                         \n"
            "{                                                                  \n"
            "    vec2 tc;                                                       \n"
            "} vs_out;                                                          \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    gl_Position = proj_matrix * mv_matrix * position;              \n"
            "    vs_out.tc = position.xy;                     \n"
            "}                                                                  \n"
        };

        static const char * fs_source[] =
        {
            "#version 420 core                                                  \n"
            "                                                                   \n"
            "out vec4 color;                                                    \n"
            "layout (location = 2) out ivec2 data;                              \n"
            "out float extra;                                                   \n"
            "                                                                   \n"
            "in BLOCK0                                                          \n"
            "{                                                                  \n"
            "    vec2 tc;                                                       \n"
            "    vec4 color;                                                    \n"
            "    flat int foo;                                                  \n"
            "} fs_in0;                                                          \n"
            "                                                                   \n"
            "in BLOCK1                                                          \n"
            "{                                                                  \n"
            "    vec3 normal[4];                                                \n"
            "    flat ivec3 layers;                                             \n"
            "    double bar;                                                    \n"
            "} fs_in1;                                                          \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    float val = abs(fs_in0.tc.x + fs_in0.tc.y) * 20.0f;            \n"
            "    color = vec4(fract(val) >= 0.5 ? 1.0 : 0.25) + fs_in1.normal[3].xyzy;                  \n"
            "    data = ivec2(1, 2);                                            \n"
            "    extra = 9.0;                                                   \n"
            "}                                                                  \n"
        };

        overlay.init(80, 50);

        GLuint program = glCreateProgram();
        glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);

        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glAttachShader(program, fs);

        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        glLinkProgram(program);

        GLint outputs;

        glGetProgramInterfaceiv(program, GL_PROGRAM_OUTPUT, GL_ACTIVE_RESOURCES, &outputs);

        static const GLenum props[] = { GL_TYPE, GL_LOCATION, GL_ARRAY_SIZE };
        static const char * prop_name[] = { "type", "location", "array size" };
        GLint i;
        GLint params[4];
        GLchar name[64];
        const char * type_name;
        char buffer[1024];

        glGetProgramInfoLog(program, sizeof(buffer), NULL, buffer);

        overlay.print("Program linked\n");
        overlay.print(buffer);

        for (i = 0; i < outputs; i++)
        {
            glGetProgramResourceName(program, GL_PROGRAM_OUTPUT, i, sizeof(name), NULL, name);
            glGetProgramResourceiv(program, GL_PROGRAM_OUTPUT, i, 3, props, 3, NULL, params);
            type_name = type_to_name(params[0]);
            if (params[2] != 0)
            {
                sprintf(buffer, "Index %d: %s %s[%d] @ location %d.\n", i, type_name, name, params[2], params[1]);
            }
            else
            {
                sprintf(buffer, "Index %d: %s %s @ location %d.\n", i, type_name, name, params[1]);
            }
            overlay.print(buffer);
        }
    }

    virtual void render(double currentTime)
    {
        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);

        overlay.draw();
    }

    virtual void shutdown()
    {
    }

private:
    sb7::text_overlay overlay;
};

DECLARE_MAIN(programinfo_app)
