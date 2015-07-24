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
#include <shader.h>
#include <vmath.h>
#include <sb7textoverlay.h>

class cubicbezier_app : public sb7::application
{
public:
    cubicbezier_app()
        : tess_program(0),
          draw_cp_program(0),
          show_points(false),
          show_cage(false),
          wireframe(false),
          paused(false)
    {

    }

    void init()
    {
        static const char title[] = "OpenGL SuperBible - Cubic Bezier Patch";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup();
    void render(double currentTime);

protected:
    GLuint      tess_program;
    GLuint      draw_cp_program;
    GLuint      patch_vao;
    GLuint      patch_buffer;
    GLuint      cage_indices;
    vmath::vec3 patch_data[16];

    bool        show_points;
    bool        show_cage;
    bool        wireframe;
    bool        paused;

    void load_shaders();
    void onKey(int key, int action);

    struct
    {
        struct
        {
            int     mv_matrix;
            int     proj_matrix;
            int     mvp;
        } patch;
        struct
        {
            int     draw_color;
            int     mvp;
        } control_point;
    } uniforms;

    sb7::text_overlay   overlay;
};

void cubicbezier_app::startup()
{
    load_shaders();

    glGenVertexArrays(1, &patch_vao);
    glBindVertexArray(patch_vao);

    glGenBuffers(1, &patch_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, patch_buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(patch_data), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    static const GLushort indices[] =
    {
        0, 1, 1, 2, 2, 3,
        4, 5, 5, 6, 6, 7,
        8, 9, 9, 10, 10, 11,
        12, 13, 13, 14, 14, 15,

        0, 4, 4, 8, 8, 12,
        1, 5, 5, 9, 9, 13,
        2, 6, 6, 10, 10, 14,
        3, 7, 7, 11, 11, 15
    };

    glGenBuffers(1, &cage_indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cage_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    overlay.init(80, 50);
    overlay.clear();
    overlay.drawText("W: Toggle wireframe", 0, 0);
    overlay.drawText("C: Toggle control cage", 0, 1);
    overlay.drawText("X: Toggle control points", 0, 2);
    overlay.drawText("P: Pause", 0, 3);
}

void cubicbezier_app::render(double currentTime)
{
    static const GLfloat gray[] = { 0.1f, 0.1f, 0.1f, 0.0f };
    static const GLfloat one = 1.0f;

    int i;
    static double last_time = 0.0;
    static double total_time = 0.0;

    if (!paused)
        total_time += (currentTime - last_time);
    last_time = currentTime;

    float t = (float)total_time;

    static const float patch_initializer[] =
    {
        -1.0f,  -1.0f,  0.0f,
        -0.33f, -1.0f,  0.0f,
         0.33f, -1.0f,  0.0f,
         1.0f,  -1.0f,  0.0f,

        -1.0f,  -0.33f, 0.0f,
        -0.33f, -0.33f, 0.0f,
         0.33f, -0.33f, 0.0f,
         1.0f,  -0.33f, 0.0f,

        -1.0f,   0.33f, 0.0f,
        -0.33f,  0.33f, 0.0f,
         0.33f,  0.33f, 0.0f,
         1.0f,   0.33f, 0.0f,

        -1.0f,   1.0f,  0.0f,
        -0.33f,  1.0f,  0.0f,
         0.33f,  1.0f,  0.0f,
         1.0f,   1.0f,  0.0f,
    };

    glViewport(0, 0, info.windowWidth, info.windowHeight);
    glClearBufferfv(GL_COLOR, 0, gray);
    glClearBufferfv(GL_DEPTH, 0, &one);

    glEnable(GL_DEPTH_TEST);

    vmath::vec3 * p = (vmath::vec3 *)glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(patch_data), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(p, patch_initializer, sizeof(patch_initializer));

    for (i = 0; i < 16; i++)
    {
        float fi = (float)i / 16.0f;
        p[i][2] = sinf(t * (0.2f + fi * 0.3f));
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindVertexArray(patch_vao);

    glUseProgram(tess_program);

    vmath::mat4 proj_matrix = vmath::perspective(50.0f,
                                                 (float)info.windowWidth / (float)info.windowHeight,
                                                 1.0f, 1000.0f);
    vmath::mat4 mv_matrix = vmath::translate(0.0f, 0.0f, -4.0f) *
                            vmath::rotate(t * 10.0f, 0.0f, 1.0f, 0.0f) *
                            vmath::rotate(t * 17.0f, 1.0f, 0.0f, 0.0f);
    
    glUniformMatrix4fv(uniforms.patch.mv_matrix, 1, GL_FALSE, mv_matrix);
    glUniformMatrix4fv(uniforms.patch.proj_matrix, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(uniforms.patch.mvp, 1, GL_FALSE, proj_matrix * mv_matrix);

    if (wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glPatchParameteri(GL_PATCH_VERTICES, 16);
    glDrawArrays(GL_PATCHES, 0, 16);

    glUseProgram(draw_cp_program);
    glUniformMatrix4fv(uniforms.control_point.mvp, 1, GL_FALSE, proj_matrix * mv_matrix);

    if (show_points)
    {
        glPointSize(9.0f);
        glUniform4fv(uniforms.control_point.draw_color, 1, vmath::vec4(0.2f, 0.7f, 0.9f, 1.0f));
        glDrawArrays(GL_POINTS, 0, 16);
    }

    if (show_cage)
    {
        glUniform4fv(uniforms.control_point.draw_color, 1, vmath::vec4(0.7f, 0.9f, 0.2f, 1.0f));
        glDrawElements(GL_LINES, 48, GL_UNSIGNED_SHORT, NULL);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    overlay.draw();
}

void cubicbezier_app::load_shaders()
{
    if (tess_program)
        glDeleteProgram(tess_program);

    GLuint shaders[4];

    shaders[0] = sb7::shader::load("media/shaders/cubicbezier/cubicbezier.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/cubicbezier/cubicbezier.tcs.glsl", GL_TESS_CONTROL_SHADER);
    shaders[2] = sb7::shader::load("media/shaders/cubicbezier/cubicbezier.tes.glsl", GL_TESS_EVALUATION_SHADER);
    shaders[3] = sb7::shader::load("media/shaders/cubicbezier/cubicbezier.fs.glsl", GL_FRAGMENT_SHADER);

    tess_program = sb7::program::link_from_shaders(shaders, 4, true);

    uniforms.patch.mv_matrix = glGetUniformLocation(tess_program, "mv_matrix");
    uniforms.patch.proj_matrix = glGetUniformLocation(tess_program, "proj_matrix");
    uniforms.patch.mvp = glGetUniformLocation(tess_program, "mvp");

    if (draw_cp_program)
        glDeleteProgram(draw_cp_program);

    shaders[0] = sb7::shader::load("media/shaders/cubicbezier/draw-control-points.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/cubicbezier/draw-control-points.fs.glsl", GL_FRAGMENT_SHADER);

    draw_cp_program = sb7::program::link_from_shaders(shaders, 2, true);

    uniforms.control_point.draw_color = glGetUniformLocation(draw_cp_program, "draw_color");
    uniforms.control_point.mvp = glGetUniformLocation(draw_cp_program, "mvp");
}

void cubicbezier_app::onKey(int key, int action)
{
    if (action)
    {
        switch (key)
        {
            case 'C': show_cage = !show_cage;
                break;
            case 'X': show_points = !show_points;
                break;
            case 'W': wireframe = !wireframe;
                break;
            case 'P': paused = !paused;
                break;
            case 'R':
                load_shaders();
                break;
            default:
                break;
        }
    }
}

DECLARE_MAIN(cubicbezier_app)
