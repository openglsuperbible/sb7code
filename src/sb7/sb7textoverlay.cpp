/*
 * Copyright © 2012-2015 Graham Sellers
 *
 * This code is part of the OpenGL SuperBible, 7th Edition.
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

#include <sb7textoverlay.h>

#include <sb7ktx.h>

namespace sb7
{

void text_overlay::init(int width, int height, const char* font)
{
    GLuint vs;
    GLuint fs;

    buffer_width = width;
    buffer_height = height;

    vs = glCreateShader(GL_VERTEX_SHADER);
    fs = glCreateShader(GL_FRAGMENT_SHADER);

    static const char * vs_source[] =
    {
        "#version 440 core\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = vec4(float((gl_VertexID >> 1) & 1) * 2.0 - 1.0,\n"
        "                       float((gl_VertexID & 1)) * 2.0 - 1.0,\n"
        "                       0.0, 1.0);\n"
        "}\n"
    };

    static const char * fs_source[] =
    {
        "#version 440 core\n"
        "layout (origin_upper_left) in vec4 gl_FragCoord;\n"
        "layout (location = 0) out vec4 o_color;\n"
        "layout (binding = 0) uniform isampler2D text_buffer;\n"
        "layout (binding = 1) uniform isampler2DArray font_texture;\n"
        "void main(void)\n"
        "{\n"
        "    ivec2 frag_coord = ivec2(gl_FragCoord.xy);\n"
        "    ivec2 char_size = textureSize(font_texture, 0).xy;\n"
        "    ivec2 char_location = frag_coord / char_size;\n"
        "    ivec2 texel_coord = frag_coord % char_size;\n"
        "    int character = texelFetch(text_buffer, char_location, 0).x;\n"
        "    float val = texelFetch(font_texture, ivec3(texel_coord, character), 0).x;\n"
        "    if (val == 0.0)\n"
        "        discard;\n"
        "    o_color = vec4(1.0);\n"
        "}\n"
    };

    glShaderSource(vs, 1, vs_source, nullptr);
    glCompileShader(vs);

    glShaderSource(fs, 1, fs_source, nullptr);
    glCompileShader(fs);

    text_program = glCreateProgram();
    glAttachShader(text_program, vs);
    glAttachShader(text_program, fs);
    glLinkProgram(text_program);

    glDeleteShader(fs);
    glDeleteShader(vs);

    // glCreateVertexArrays(1, &vao);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // glCreateTextures(GL_TEXTURE_2D, 1, &text_buffer);
    glGenTextures(1, &text_buffer);
    glBindTexture(GL_TEXTURE_2D, text_buffer);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_R8UI, width, height);

    font_texture = sb7::ktx::file::load(font ? font : "media/textures/cp437_9x16.ktx");

    screen_buffer = new char[width * height];
    memset(screen_buffer, 0, width * height);
}

void text_overlay::teardown()
{
    delete[] screen_buffer;
    glDeleteTextures(1, &font_texture);
    glDeleteTextures(1, &text_buffer);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(text_program);
}

void text_overlay::draw()
{
    glUseProgram(text_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, text_buffer);
    if (dirty)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer_width, buffer_height, GL_RED_INTEGER, GL_UNSIGNED_BYTE, screen_buffer);
        dirty = false;
    }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, font_texture);

    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void text_overlay::drawText(const char* str, int x, int y)
{
    char* dst = screen_buffer + y * buffer_width + x;
    strcpy(dst, str);
    dirty = true;
}

void text_overlay::scroll(int lines)
{
    const char* src = screen_buffer + lines * buffer_width;
    char * dst = screen_buffer;

    memmove(dst, src, (buffer_height - lines) * buffer_width);

    dirty = true;
}

void text_overlay::print(const char* str)
{
    const char* p = str;
    char c;
    char* dst = screen_buffer + cursor_y * buffer_width + cursor_x;

    while (*p != 0)
    {
        c = *p++;
        if (c == '\n')
        {
            cursor_y++;
            cursor_x = 0;
            if (cursor_y >= buffer_height)
            {
                cursor_y--;
                scroll(1);
            }
            dst = screen_buffer + cursor_y * buffer_width + cursor_x;
        }
        else
        {
            *dst++ = c;
            cursor_x++;
            if (cursor_x >= buffer_width)
            {
                cursor_y++;
                cursor_x = 0;
                if (cursor_y >= buffer_height)
                {
                    cursor_y--;
                    scroll(1);
                }
                dst = screen_buffer + cursor_y * buffer_width + cursor_x;
            }
        }
    }

    dirty = true;
}

void text_overlay::moveCursor(int x, int y)
{
    cursor_x = x;
    cursor_y = y;
}

void text_overlay::clear()
{
    memset(screen_buffer, 0, buffer_width * buffer_height);
    dirty = true;
    cursor_x = 0;
    cursor_y = 0;
}

}
