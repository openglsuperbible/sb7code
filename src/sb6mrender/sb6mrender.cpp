/*
 * Copyright © 2012-2013 Graham Sellers
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

#include <sb6.h>
#include <vmath.h>

#include <object.h>
#include <sb6ktx.h>
#include <shader.h>
#include <arcball.h>

class sb6mrender_app : public sb6::application
{
public:
    sb6mrender_app()
        : mouseDown(false)
    {
        mat_rotation = vmath::mat4::identity();
    }

protected:
    void init()
    {
        static const char title[] = "OpenGL SuperBible - SB6 Model Rendering";

        sb6::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    virtual void startup()
    {
        /*
        static const char * vs_source[] =
        {
            "#version 420 core                                                  \n"
            "                                                                   \n"
            "layout (location = 0) in vec4 position;                            \n"
            "layout (location = 1) in vec3 normal;                              \n"
            "layout (location = 2) in vec3 tangent;                             \n"
            "layout (location = 4) in vec2 texcoord;                            \n"
            "                                                                   \n"
            "out VS_OUT                                                         \n"
            "{                                                                  \n"
            "    vec3 normal;                                                   \n"
            "    vec4 color;                                                    \n"
            "    vec2 texcoord;                                                 \n"
            "    vec3 tangent;                                                  \n"
            "} vs_out;                                                          \n"
            "                                                                   \n"
            "uniform mat4 mv_matrix;                                            \n"
            "uniform mat4 proj_matrix;                                          \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            "    gl_Position = proj_matrix * mv_matrix * position;              \n"
            "    vs_out.color = position * 2.0 + vec4(0.5, 0.5, 0.5, 0.0);      \n"
            "    vs_out.normal = mat3(mv_matrix) * normal;                      \n"
            "    vs_out.texcoord = texcoord;                                    \n"
            "    vs_out.tangent = tangent;                                      \n"
            "}                                                                  \n"
        };

        static const char * fs_source[] =
        {
            "#version 420 core                                                  \n"
            "                                                                   \n"
            "out vec4 color;                                                    \n"
            "                                                                   \n"
            "uniform sampler2D tex_color;                                       \n"
            "                                                                   \n"
            "in VS_OUT                                                          \n"
            "{                                                                  \n"
            "    vec3 normal;                                                   \n"
            "    vec4 color;                                                    \n"
            "    vec2 texcoord;                                                 \n"
            "    vec3 tangent;                                                  \n"
            "} fs_in;                                                           \n"
            "                                                                   \n"
            "void main(void)                                                    \n"
            "{                                                                  \n"
            // "    color = abs(fs_in.normal).z * vec4(fs_in.texcoord, 0.0, 1.0);        \n"
            "    color = texture(tex_color, fs_in.texcoord);                    \n"
            "}                                                                  \n"
        };

        program = glCreateProgram();
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, 1, fs_source, NULL);
        glCompileShader(fs);

        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, 1, vs_source, NULL);
        glCompileShader(vs);

        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glLinkProgram(program);
        */

        loadShaders();

        object.load("media/objects/ladybug.sbm");

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glGenTextures(1, &tex_color);
        glGenTextures(1, &tex_normal);
        glActiveTexture(GL_TEXTURE0);
        sb6::ktx::file::load("media/textures/ladybug_co.ktx", tex_color);
        glActiveTexture(GL_TEXTURE1);
        sb6::ktx::file::load("media/textures/ladybug_nm.ktx", tex_normal);
    }

    virtual void render(double currentTime)
    {
        static const GLfloat green[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        static const GLfloat one = 1.0f;
        float f = (float)currentTime;

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, green);
        glClearBufferfv(GL_DEPTH, 0, &one);

        glUseProgram(program);

        vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                     (float)info.windowWidth / (float)info.windowHeight,
                                                     0.1f,
                                                     1000.0f);
        glUniformMatrix4fv(proj_location, 1, GL_FALSE, proj_matrix);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, -0.5f, -7.0f) *
                                vmath::rotate((float)currentTime * 5.0f, 0.0f, 1.0f, 0.0f) *
                                vmath::mat4::identity();
        glUniformMatrix4fv(mv_location, 1, GL_FALSE, mv_matrix);

        object.render();
    }

    virtual void shutdown()
    {
        object.free();
        glDeleteProgram(program);
    }

    void onMouseButton(int button, int action)
    {
        int x, y;

        if (action == 1)
        {
            getMousePosition(x, y);
            arcball.onMouseDown(float(x), float(y));
            mouseDown = true;
        }
        else
        {
            mouseDown = false;
        }
    }

    void onMouseMove(int x, int y)
    {
        if (mouseDown)
        {
            arcball.onMouseMove(float(x), float(y));
            // mat_rotation = arcball.getRotation().asMatrix();
            mat_rotation = arcball.getRotationMatrix();
        }
    }

    void onResize(int w, int h)
    {
        sb6::application::onResize(w, h);

        arcball.setDimensions(float(w), float(h));
    }

    void onKey(int key, int action)
    {
        if (action)
        {
            switch (key)
            {
                case 'R': 
                    loadShaders();
                    break;
            }
        }
    }

    void loadShaders()
    {
        GLuint vs;
        GLuint fs;

        vs = sb6::shader::load("media/shaders/sb6mrender/render.vs.glsl", GL_VERTEX_SHADER);
        fs = sb6::shader::load("media/shaders/sb6mrender/render.fs.glsl", GL_FRAGMENT_SHADER);

        if (program != 0)
            glDeleteProgram(program);

        program = glCreateProgram();

        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glLinkProgram(program);

        mv_location = glGetUniformLocation(program, "mv_matrix");
        proj_location = glGetUniformLocation(program, "proj_matrix");

    }

private:
    GLuint          program;
    GLint           mv_location;
    GLint           proj_location;

    vmath::mat4         mat_rotation;

    GLuint              tex_color;
    GLuint              tex_normal;
    sb6::object         object;
    sb6::utils::arcball arcball;
    bool                mouseDown;
};

DECLARE_MAIN(sb6mrender_app)
