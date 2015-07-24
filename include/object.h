/*
 * Copyright © 2012-2013 Graham Sellers
 *
 * This code is part of the OpenGL SuperBible, 6th Edition.
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
 *
 */

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "sb6mfile.h"

#ifndef SB6M_FILETYPES_ONLY

#include <GL/glcorearb.h>

namespace sb7
{

class object
{
public:
    object();
    ~object();

    inline void render(unsigned int instance_count = 1,
                       unsigned int base_instance = 0)
    {
        render_sub_object(0, instance_count, base_instance);
    }

    void render_sub_object(unsigned int object_index,
                           unsigned int instance_count = 1,
                           unsigned int base_instance = 0);

    void get_sub_object_info(unsigned int index, GLuint &first, GLuint &count)
    {
        if (index >= num_sub_objects)
        {
            first = 0;
            count = 0;
        }
        else
        {
            first = sub_object[index].first;
            count = sub_object[index].count;
        }
    }

    unsigned int get_sub_object_count() const           { return num_sub_objects; }
    GLuint       get_vao() const                        { return vao; }
    void load(const char * filename);
    void free();

private:
    GLuint                  data_buffer;
    GLuint                  vao;
    GLuint                  index_type;
    GLuint                  index_offset;

    enum { MAX_SUB_OBJECTS = 256 };

    unsigned int            num_sub_objects;
    SB6M_SUB_OBJECT_DECL    sub_object[MAX_SUB_OBJECTS];
};

}

#endif /* SB6M_FILETYPES_ONLY */

#endif /* __OBJECT_H__ */

