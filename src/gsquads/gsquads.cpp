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
#include <sb7textoverlay.h>

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

class gsquads_app : public sb7::application
{
public:
    gsquads_app()
        : program_linesadjacency(0),
          program_fans(0),
          mode(0),
          paused(0),
          vid_offset(0)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Quad Rendering";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup(void)
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        load_shaders();

        overlay.init(80, 50);
    }

    void shutdown(void)
    {
        glDeleteProgram(program_linesadjacency);
        glDeleteProgram(program_fans);
        glDeleteVertexArrays(1, &vao);
    }

    void render(double currentTime)
    {
        static const GLfloat black[] = { 0.0f, 0.25f, 0.0f, 1.0f };
        glViewport(0, 0, info.windowWidth, info.windowHeight);

        static double last_time = 0.0;
        static double total_time = 0.0;

        if (!paused)
            total_time += (currentTime - last_time);
        last_time = currentTime;

        float t = (float)total_time;

        glClearBufferfv(GL_COLOR, 0, black);

        vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -2.0f) *
                                vmath::rotate((float)t * 5.0f, 0.0f, 0.0f, 1.0f) *
                                vmath::rotate((float)t * 30.0f, 1.0f, 0.0f, 0.0f);
        vmath::mat4 proj_matrix = vmath::perspective(50.0f, (float)info.windowWidth / (float)info.windowHeight, 0.1f, 1000.0f);
        vmath::mat4 mvp = proj_matrix * mv_matrix;
        
        overlay.clear();

        switch (mode)
        {
            case 0:
                overlay.drawText("Drawing quads using GL_TRIANGLE_FAN", 0, 0);
                glUseProgram(program_fans);
                glUniformMatrix4fv(mvp_loc_fans, 1, GL_FALSE, mvp);
                glUniform1i(vid_offset_loc_fans, vid_offset);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                break;
            case 1:
                overlay.drawText("Drawing quads using geometry shaders and GL_LINES_ADJACENCY", 0, 0);
                glUseProgram(program_linesadjacency);
                glUniformMatrix4fv(mvp_loc_linesadj, 1, GL_FALSE, mvp);
                glUniform1i(vid_offset_loc_linesadj, vid_offset);
                glDrawArrays(GL_LINES_ADJACENCY, 0, 4);
                break;
        }

        overlay.drawText("1, 2: Choose mode (M toggles)", 0, 1);
        overlay.drawText("P: Pause", 0, 2);
        overlay.drawText("Numpad +, -: Rotate quad vertices", 0, 3);
        overlay.draw();
    }

    void onKey(int key, int action)
    {
        if (!action)
            return;

        switch (key)
        {
            case '1':
            case '2':
                    mode = key - '1';
                break;
            case GLFW_KEY_KP_ADD:
                vid_offset++;
                break;
            case GLFW_KEY_KP_SUBTRACT:
                vid_offset--;
                break;
            case 'P': paused = !paused;
                break;
            case 'R':
                    load_shaders();
                break;
            case 'M':
                    mode = (mode + 1) % 2;
                break;
        }
    }

    void load_shaders()
    {
        GLuint vs;
        GLuint gs;
        GLuint fs;

        if (program_fans)
            glDeleteProgram(program_fans);

        program_fans = glCreateProgram();

        vs = sb7::shader::load("media/shaders/gsquads/quadsasfans.vs.glsl", GL_VERTEX_SHADER);
        fs = sb7::shader::load("media/shaders/gsquads/quadsasfans.fs.glsl", GL_FRAGMENT_SHADER);

        glAttachShader(program_fans, vs);
        glAttachShader(program_fans, fs);

        glLinkProgram(program_fans);

        mvp_loc_fans = glGetUniformLocation(program_fans, "mvp");
        vid_offset_loc_fans = glGetUniformLocation(program_fans, "vid_offset");

        glDeleteShader(vs);
        glDeleteShader(fs);

        vs = sb7::shader::load("media/shaders/gsquads/quadsaslinesadj.vs.glsl", GL_VERTEX_SHADER);
        gs = sb7::shader::load("media/shaders/gsquads/quadsaslinesadj.gs.glsl", GL_GEOMETRY_SHADER);
        fs = sb7::shader::load("media/shaders/gsquads/quadsaslinesadj.fs.glsl", GL_FRAGMENT_SHADER);

        if (program_linesadjacency)
            glDeleteProgram(program_linesadjacency);

        program_linesadjacency = glCreateProgram();

        glAttachShader(program_linesadjacency, vs);
        glAttachShader(program_linesadjacency, gs);
        glAttachShader(program_linesadjacency, fs);

        glLinkProgram(program_linesadjacency);

        mvp_loc_linesadj = glGetUniformLocation(program_linesadjacency, "mvp");
        vid_offset_loc_linesadj = glGetUniformLocation(program_fans, "vid_offset");

        glDeleteShader(vs);
        glDeleteShader(gs);
        glDeleteShader(fs);
    }

private:
    GLuint      program_fans;
    GLuint      program_linesadjacency;
    GLuint      vao;
    int         mode;
    int         mvp_loc_fans;
    int         mvp_loc_linesadj;
    int         vid_offset_loc_fans;
    int         vid_offset_loc_linesadj;
    int         vid_offset;
    bool        paused;
    sb7::text_overlay   overlay;
};

DECLARE_MAIN(gsquads_app);
