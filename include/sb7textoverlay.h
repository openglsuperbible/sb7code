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

#ifndef __SB7TEXTOVERLAY_H__
#define __SB7TEXTOVERLAY_H__

#include <sb7.h>

namespace sb7
{

class text_overlay
{
public:
    text_overlay()
        : cursor_x(0),
          cursor_y(0)
    {

    }

    void init(int width, int height, const char* font = nullptr);
    void teardown();
    void draw();

    void drawText(const char* str, int x, int y);
    void print(const char* str);
    void scroll(int lines);
    void moveCursor(int x, int y);
    void clear();

private:
    GLuint      text_buffer;
    GLuint      font_texture;
    GLuint      vao;

    GLuint      text_program;
    char *      screen_buffer;
    int         buffer_width;
    int         buffer_height;
    bool        dirty;
    int         cursor_x;
    int         cursor_y;
};

} // namespace sb7

#endif /* __SB7TEXTOVERLAY_H__ */
