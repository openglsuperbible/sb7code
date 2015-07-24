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
#include <shader.h>

#include <string>
static void print_shader_log(GLuint shader)
{
    std::string str;
    GLint len;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
    str.resize(len);
    glGetShaderInfoLog(shader, len, NULL, &str[0]);

#ifdef _WIN32
    OutputDebugStringA(str.c_str());
#endif
}

class hdrtonemap_app : public sb7::application
{
public:
    hdrtonemap_app()
        : exposure(1.0f),
          program_adaptive(0),
          program_exposure(0),
          program_naive(0),
          mode(0)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - HDR Tone Mapping";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup(void)
    {
        // Load texture from file
        tex_src = sb7::ktx::file::load("media/textures/treelights_2k.ktx");

        // Now bind it to the context using the GL_TEXTURE_2D binding point
        glBindTexture(GL_TEXTURE_2D, tex_src);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        load_shaders();

        static const GLfloat exposureLUT[20]   = { 11.0f, 6.0f, 3.2f, 2.8f, 2.2f, 1.90f, 1.80f, 1.80f, 1.70f, 1.70f,  1.60f, 1.60f, 1.50f, 1.50f, 1.40f, 1.40f, 1.30f, 1.20f, 1.10f, 1.00f };

        glGenTextures(1, &tex_lut);
        glBindTexture(GL_TEXTURE_1D, tex_lut);
        glTexStorage1D(GL_TEXTURE_1D, 1, GL_R32F, 20);
        glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 20, GL_RED, GL_FLOAT, exposureLUT);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    }

    void shutdown(void)
    {
        glDeleteProgram(program_adaptive);
        glDeleteProgram(program_exposure);
        glDeleteProgram(program_naive);
        glDeleteVertexArrays(1, &vao);
        glDeleteTextures(1, &tex_src);
        glDeleteTextures(1, &tex_lut);
    }

    void render(double t)
    {
        static const GLfloat black[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        glViewport(0, 0, info.windowWidth, info.windowHeight);

        glClearBufferfv(GL_COLOR, 0, black);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, tex_lut);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex_src);

        // glUseProgram(mode ? program_adaptive : program_naive);
        switch (mode)
        {
            case 0:
                glUseProgram(program_naive);
                break;
            case 1:
                glUseProgram(program_exposure);
                glUniform1f(uniforms.exposure.exposure, exposure);
                break;
            case 2:
                glUseProgram(program_adaptive);
                break;
        }
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void onKey(int key, int action)
    {
        if (!action)
            return;

        switch (key)
        {
            case '1':
            case '2':
            case '3':
                    mode = key - '1';
                break;
            case 'R':
                    load_shaders();
                break;
            case 'M':
                    mode = (mode + 1) % 3;
                break;
            case GLFW_KEY_KP_ADD:
                    exposure *= 1.1f;
                break;
            case GLFW_KEY_KP_SUBTRACT:
                    exposure /= 1.1f;
                break;
        }
    }

    void load_shaders()
    {
        GLuint vs;
        GLuint fs;

        if (program_naive)
            glDeleteProgram(program_naive);

        program_naive = glCreateProgram();

        vs = sb7::shader::load("media/shaders/hdrtonemap/tonemap.vs.glsl", GL_VERTEX_SHADER);
        fs = sb7::shader::load("media/shaders/hdrtonemap/tonemap_naive.fs.glsl", GL_FRAGMENT_SHADER);

        glAttachShader(program_naive, vs);
        glAttachShader(program_naive, fs);

        glLinkProgram(program_naive);

        glDeleteShader(fs);

        fs = sb7::shader::load("media/shaders/hdrtonemap/tonemap_adaptive.fs.glsl", GL_FRAGMENT_SHADER);

        if (program_adaptive)
            glDeleteProgram(program_adaptive);

        program_adaptive = glCreateProgram();

        glAttachShader(program_adaptive, vs);
        glAttachShader(program_adaptive, fs);

        glLinkProgram(program_adaptive);

        glDeleteShader(fs);

        fs = sb7::shader::load("media/shaders/hdrtonemap/tonemap_exposure.fs.glsl", GL_FRAGMENT_SHADER);

        if (program_exposure)
            glDeleteProgram(program_exposure);

        program_exposure = glCreateProgram();

        glAttachShader(program_exposure, vs);
        glAttachShader(program_exposure, fs);

        glLinkProgram(program_exposure);

        uniforms.exposure.exposure = glGetUniformLocation(program_exposure, "exposure");

        glDeleteShader(vs);
        glDeleteShader(fs);
    }

private:
    GLuint      tex_src;
    GLuint      tex_lut;

    GLuint      program_naive;
    GLuint      program_exposure;
    GLuint      program_adaptive;
    GLuint      vao;
    float       exposure;
    int         mode;

    struct
    {
        struct
        {
            int exposure;
        } exposure;
    } uniforms;
};

DECLARE_MAIN(hdrtonemap_app);
