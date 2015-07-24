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
#include <sb7ktx.h>
#include <sb7textoverlay.h>
#include <sb7color.h>

class fontdemo_app : public sb7::application
{
    void init()
    {
        static const char title[] = "OpenGL SuperBible - Bitmap Font Rendering";

        sb7::application::init();

        memcpy(info.title, title, sizeof(title));
    }

    void startup()
    {

        static const char string[] = "This a test. Can you hear me?";
        static const char string2[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*();:<>,./?~`'\"";
        static const char string3[] = "The quick brown fox jumped over the lazy dog.";

        // _text_overlay.init(48, 32, "media/textures/font16x16.ktx");
        text_overlay.init(64, 32, "media/textures/cp437_9x16.ktx");
        text_overlay.clear();
        text_overlay.print("This is a demo of bitmap font rendering.\n\n"
                           "This was printed as one string with newlines.\n\n"
                           "If you have a really, really, really, really, really, really, really, really, really, really long string like this one, it will wrap.\n\n"
                           "The final text buffer is composited over whatever was drawn into the framebuffer below.\n\n\n\n\n"
                           "It's not all that pretty, but it's fast:");
        text_overlay.print("\n\n\n\n");
        text_overlay.print(string2);
        text_overlay.print("\n");
        text_overlay.print(string3);
    }
    
    void render(double currentTime)
    {
        static int frame = 0;

        char buffer[1024];
        frame++;
        if ((frame & 0x1FF) == 0x100)
        {
            sprintf(buffer, "%d frames in %lf secs = %3.3f fps", frame, currentTime, (float)frame / (float)currentTime);
            text_overlay.drawText(buffer, 0, 16);
        }

        glClearBufferfv(GL_COLOR, 0, sb7::color::Green);

        glViewport(0, 0, info.windowWidth, info.windowHeight);

        text_overlay.draw();
    }

    void shutdown()
    {
        text_overlay.teardown();
    }

protected:
    sb7::text_overlay    text_overlay;
};

DECLARE_MAIN(fontdemo_app)
