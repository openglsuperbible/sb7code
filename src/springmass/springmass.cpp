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
#include <shader.h>

enum BUFFER_TYPE_t
{
    POSITION_A,
    POSITION_B,
    VELOCITY_A,
    VELOCITY_B,
    CONNECTION
};

enum
{
    POINTS_X            = 50,
    POINTS_Y            = 50,
    POINTS_TOTAL        = (POINTS_X * POINTS_Y),
    CONNECTIONS_TOTAL   = (POINTS_X - 1) * POINTS_Y + (POINTS_Y - 1) * POINTS_X
};

class springmass_app : public sb7::application
{
public:
    springmass_app()
        : m_iteration_index(0),
          m_update_program(0),
          m_render_program(0),
          draw_points(true),
          draw_lines(true),
          iterations_per_frame(16)
    {
    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Spring-Mass Simulator";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup(void)
    {
        int i, j;

        load_shaders();

        vmath::vec4 * initial_positions = new vmath::vec4 [POINTS_TOTAL];
        vmath::vec3 * initial_velocities = new vmath::vec3 [POINTS_TOTAL];
        vmath::ivec4 * connection_vectors = new vmath::ivec4 [POINTS_TOTAL];

        int n = 0;

        for (j = 0; j < POINTS_Y; j++) {
            float fj = (float)j / (float)POINTS_Y;
            for (i = 0; i < POINTS_X; i++) {
                float fi = (float)i / (float)POINTS_X;

                initial_positions[n] = vmath::vec4((fi - 0.5f) * (float)POINTS_X,
                                                   (fj - 0.5f) * (float)POINTS_Y,
                                                   0.6f * sinf(fi) * cosf(fj),
                                                   1.0f);
                initial_velocities[n] = vmath::vec3(0.0f);

                connection_vectors[n] = vmath::ivec4(-1);

                if (j != (POINTS_Y - 1))
                {
                    if (i != 0)
                        connection_vectors[n][0] = n - 1;

                    if (j != 0)
                        connection_vectors[n][1] = n - POINTS_X;

                    if (i != (POINTS_X - 1))
                        connection_vectors[n][2] = n + 1;

                    if (j != (POINTS_Y - 1))
                        connection_vectors[n][3] = n + POINTS_X;
                }
                n++;
            }
        }

        glGenVertexArrays(2, m_vao);
        glGenBuffers(5, m_vbo);

        for (i = 0; i < 2; i++) {
            glBindVertexArray(m_vao[i]);

            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[POSITION_A + i]);
            glBufferData(GL_ARRAY_BUFFER, POINTS_TOTAL * sizeof(vmath::vec4), initial_positions, GL_DYNAMIC_COPY);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(0);

            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[VELOCITY_A + i]);
            glBufferData(GL_ARRAY_BUFFER, POINTS_TOTAL * sizeof(vmath::vec3), initial_velocities, GL_DYNAMIC_COPY);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glEnableVertexAttribArray(1);

            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[CONNECTION]);
            glBufferData(GL_ARRAY_BUFFER, POINTS_TOTAL * sizeof(vmath::ivec4), connection_vectors, GL_STATIC_DRAW);
            glVertexAttribIPointer(2, 4, GL_INT, 0, NULL);
            glEnableVertexAttribArray(2);
        }

        delete [] connection_vectors;
        delete [] initial_velocities;
        delete [] initial_positions;

        glGenTextures(2, m_pos_tbo);
        glBindTexture(GL_TEXTURE_BUFFER, m_pos_tbo[0]);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_vbo[POSITION_A]);
        glBindTexture(GL_TEXTURE_BUFFER, m_pos_tbo[1]);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, m_vbo[POSITION_B]);

        int lines = (POINTS_X - 1) * POINTS_Y + (POINTS_Y - 1) * POINTS_X;

        glGenBuffers(1, &m_index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, lines * 2 * sizeof(int), NULL, GL_STATIC_DRAW);

        int * e = (int *)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, lines * 2 * sizeof(int), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

        for (j = 0; j < POINTS_Y; j++)  
        {
            for (i = 0; i < POINTS_X - 1; i++)
            {
                *e++ = i + j * POINTS_X;
                *e++ = 1 + i + j * POINTS_X;
            }
        }

        for (i = 0; i < POINTS_X; i++)
        {
            for (j = 0; j < POINTS_Y - 1; j++)
            {
                *e++ = i + j * POINTS_X;
                *e++ = POINTS_X + i + j * POINTS_X;
            }
        }

        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    }

    void shutdown(void)
    {
        glDeleteProgram(m_update_program);
        glDeleteBuffers(5, m_vbo);
        glDeleteVertexArrays(2, m_vao);
    }

    void render(double t)
    {
        int i;
        glUseProgram(m_update_program);

        glEnable(GL_RASTERIZER_DISCARD);

        for (i = iterations_per_frame; i != 0; --i)
        {
            glBindVertexArray(m_vao[m_iteration_index & 1]);
            glBindTexture(GL_TEXTURE_BUFFER, m_pos_tbo[m_iteration_index & 1]);
            m_iteration_index++;
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_vbo[POSITION_A + (m_iteration_index & 1)]);
            glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, m_vbo[VELOCITY_A + (m_iteration_index & 1)]);
            glBeginTransformFeedback(GL_POINTS);
            glDrawArrays(GL_POINTS, 0, POINTS_TOTAL);
            glEndTransformFeedback();
        }

        glDisable(GL_RASTERIZER_DISCARD);

        static const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 0.0f };

        glViewport(0, 0, info.windowWidth, info.windowHeight);
        glClearBufferfv(GL_COLOR, 0, black);

        glUseProgram(m_render_program);

        if (draw_points)
        {
            glPointSize(4.0f);
            glDrawArrays(GL_POINTS, 0, POINTS_TOTAL);
        }

        if (draw_lines)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
            glDrawElements(GL_LINES, CONNECTIONS_TOTAL * 2, GL_UNSIGNED_INT, NULL);
        }
    }

private:
    void onKey(int key, int action)
    {
        if (action)
        {
            switch (key)
            {
                case 'R': load_shaders();
                    break;
                case 'L': draw_lines = !draw_lines;
                    break;
                case 'P': draw_points = !draw_points;
                    break;
                case GLFW_KEY_KP_ADD: iterations_per_frame++;
                    break;
                case GLFW_KEY_KP_SUBTRACT: iterations_per_frame--;
                    break;
            }
        }
    }

    void load_shaders()
    {
        GLuint vs;
        GLuint fs;
        char buffer[1024];

        vs = sb7::shader::load("media/shaders/springmass/update.vs.glsl", GL_VERTEX_SHADER);

        if (m_update_program)
            glDeleteProgram(m_update_program);
        m_update_program = glCreateProgram();
        glAttachShader(m_update_program, vs);

        static const char * tf_varyings[] = 
        {
            "tf_position_mass",
            "tf_velocity"
        };

        glTransformFeedbackVaryings(m_update_program, 2, tf_varyings, GL_SEPARATE_ATTRIBS);

        glLinkProgram(m_update_program);

        glGetShaderInfoLog(vs, 1024, NULL, buffer);
        glGetProgramInfoLog(m_update_program, 1024, NULL, buffer);

        glDeleteShader(vs);

        vs = sb7::shader::load("media/shaders/springmass/render.vs.glsl", GL_VERTEX_SHADER);
        fs = sb7::shader::load("media/shaders/springmass/render.fs.glsl", GL_FRAGMENT_SHADER);

        if (m_render_program)
            glDeleteProgram(m_render_program);
        m_render_program = glCreateProgram();
        glAttachShader(m_render_program, vs);
        glAttachShader(m_render_program, fs);
        
        glLinkProgram(m_render_program);
    }

    GLuint          m_vao[2];
    GLuint          m_vbo[5];
    GLuint          m_index_buffer;
    GLuint          m_pos_tbo[2];
    GLuint          m_update_program;
    GLuint          m_render_program;
    GLuint          m_C_loc;
    GLuint          m_iteration_index;

    bool            draw_points;
    bool            draw_lines;
    int             iterations_per_frame;
};

DECLARE_MAIN(springmass_app);
