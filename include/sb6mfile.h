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
 *
 */

#ifndef __SB6MFILE_H__
#define __SB6MFILE_H__

#define SB6M_FOURCC(a,b,c,d)            ( ((unsigned int)(a) << 0) | ((unsigned int)(b) << 8) | ((unsigned int)(c) << 16) | ((unsigned int)(d) << 24) )
#define SB6M_MAGIC                      SB6M_FOURCC('S','B','6','M')

#ifdef _MSC_VER
#pragma pack (push, 1)
#endif

typedef enum SB6M_CHUNK_TYPE_t
{
    SB6M_CHUNK_TYPE_INDEX_DATA      = SB6M_FOURCC('I','N','D','X'),
    SB6M_CHUNK_TYPE_VERTEX_DATA     = SB6M_FOURCC('V','R','T','X'),
    SB6M_CHUNK_TYPE_VERTEX_ATTRIBS  = SB6M_FOURCC('A','T','R','B'),
    SB6M_CHUNK_TYPE_SUB_OBJECT_LIST = SB6M_FOURCC('O','L','S','T'),
    SB6M_CHUNK_TYPE_COMMENT         = SB6M_FOURCC('C','M','N','T'),
    SB6M_CHUNK_TYPE_DATA            = SB6M_FOURCC('D','A','T','A')
} SB6M_CHUNK_TYPE;

typedef struct SB6M_HEADER_t
{
    union
    {
        unsigned int    magic;
        char            magic_name[4];
    };
    unsigned int        size;
    unsigned int        num_chunks;
    unsigned int        flags;
} SB6M_HEADER;

typedef struct SB6M_CHUNK_HEADER_t
{
    union
    {
        unsigned int    chunk_type;
        char            chunk_name[4];
    };
    unsigned int        size;
} SB6M_CHUNK_HEADER;

typedef struct SB6M_CHUNK_INDEX_DATA_t
{
    SB6M_CHUNK_HEADER   header;
    unsigned int        index_type;
    unsigned int        index_count;
    unsigned int        index_data_offset;
} SB6M_CHUNK_INDEX_DATA;

typedef struct SB6M_CHUNK_VERTEX_DATA_t
{
    SB6M_CHUNK_HEADER   header;
    unsigned int        data_size;
    unsigned int        data_offset;
    unsigned int        total_vertices;
} SB6M_CHUNK_VERTEX_DATA;

typedef struct SB6M_VERTEX_ATTRIB_DECL_t
{
    char                name[64];
    unsigned int        size;
    unsigned int        type;
    unsigned int        stride;
    unsigned int        flags;
    unsigned int        data_offset;
} SB6M_VERTEX_ATTRIB_DECL;

#define SB6M_VERTEX_ATTRIB_FLAG_NORMALIZED      0x00000001
#define SB6M_VERTEX_ATTRIB_FLAG_INTEGER         0x00000002

typedef struct SB6M_VERTEX_ATTRIB_CHUNK_t
{
    SB6M_CHUNK_HEADER           header;
    unsigned int                attrib_count;
    SB6M_VERTEX_ATTRIB_DECL     attrib_data[1];
} SB6M_VERTEX_ATTRIB_CHUNK;

typedef enum SB6M_DATA_ENCODING_t
{
    SB6M_DATA_ENCODING_RAW              = 0
} SB6M_DATA_ENCODING;

typedef struct SB6M_DATA_CHUNK_t
{
    SB6M_CHUNK_HEADER           header;
    unsigned int                encoding;
    unsigned int                data_offset;
    unsigned int                data_length;
} SB6M_DATA_CHUNK;

typedef struct SB6M_SUB_OBJECT_DECL_t
{
    unsigned int                first;
    unsigned int                count;
} SB6M_SUB_OBJECT_DECL;

typedef struct SB6M_CHUNK_SUB_OBJECT_LIST_t
{
    SB6M_CHUNK_HEADER           header;
    unsigned int                count;
    SB6M_SUB_OBJECT_DECL        sub_object[1];
} SB6M_CHUNK_SUB_OBJECT_LIST;

typedef struct SB6M_CHUNK_COMMENT_t
{
    SB6M_CHUNK_HEADER           header;
    char                        comment[1];
} SB6M_CHUNK_COMMENT;

#ifdef _MSC_VER
#pragma pack (pop)
#endif

#endif /* __SB6MFILE_H__ */
