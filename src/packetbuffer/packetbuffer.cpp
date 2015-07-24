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
#include <object.h>
#include <vmath.h>
#include <sb7textoverlay.h>

#define ATOMIC_PACKET_BUFFER

#ifdef ATOMIC_PACKET_BUFFER
#include <atomic>
#endif

namespace packet
{

struct base;

typedef void (APIENTRYP PFN_EXECUTE)(const base* __restrict pParams);

struct base
{
    PFN_EXECUTE     pfnExecute;
};

struct BIND_PROGRAM : public base
{
    GLuint program;

    static void APIENTRY execute(const BIND_PROGRAM* __restrict pParams)
    {
        glUseProgram(pParams->program);
    }
};

struct BIND_VERTEX_ARRAY : public base
{
    GLuint vao;

    static void APIENTRY execute(const BIND_VERTEX_ARRAY* __restrict pParams)
    {
        glBindVertexArray(pParams->vao);
    }
};

struct BIND_BUFFER_RANGE : public base
{
    GLenum target;
    GLuint index;
    GLuint buffer;
    GLintptr offset;
    GLsizeiptr size;

    static void APIENTRY execute(const BIND_BUFFER_RANGE* __restrict pParams)
    {
        glBindBufferRange(pParams->target, pParams->index, pParams->buffer, pParams->offset, pParams->size);
    }
};

struct DRAW_ELEMENTS : public base
{
    GLenum mode;
    GLsizei count;
    GLenum type;
    GLvoid *indices;
    GLsizei primcount;
    GLint basevertex;
    GLuint baseinstance;

    static void APIENTRY execute(const DRAW_ELEMENTS* __restrict pParams)
    {
        glDrawElementsInstancedBaseVertexBaseInstance(pParams->mode, pParams->count, pParams->type, pParams->indices, pParams->primcount, pParams->basevertex, pParams->baseinstance);
    }
};

struct DRAW_ARRAYS : public base
{
    GLenum mode;
    GLint first;
    GLsizei count;
    GLsizei primcount;
    GLuint baseinstance;

    static void APIENTRY execute(const DRAW_ARRAYS* __restrict pParams)
    {
        glDrawArraysInstancedBaseInstance(pParams->mode, pParams->first, pParams->count, pParams->primcount, pParams->baseinstance);
    }
};

struct ENABLE_DISABLE : public base
{
    GLenum cap;

    static void APIENTRY execute_enable(const ENABLE_DISABLE* __restrict pParams)
    {
        glEnable(pParams->cap);
    }

    static void APIENTRY execute_disable(const ENABLE_DISABLE* __restrict pParams)
    {
        glDisable(pParams->cap);
    }
};

union ALL_PACKETS
{
public:
    PFN_EXECUTE         execute;
private:
    base                Base;
    BIND_PROGRAM        BindProgram;
    BIND_VERTEX_ARRAY   BindVertexArray;
    DRAW_ELEMENTS       DrawElements;
    DRAW_ARRAYS         DrawArrays;
    ENABLE_DISABLE      EnableDisable;
};

}

class packet_stream
{
public:
    packet_stream()
        : max_packets(0),
          m_packets(nullptr)
    {
        num_packets.store(0);
    }

    enum FINIALIZE_MODE
    {
        FINILIZE_TERMINATE,
        FINALIZE_RETURN_TO_DEFAULTS
    };

    enum RESET_MODE
    {
        RESET_INHERIT,
        RESET_RETURN_TO_DEFAULTS
    };

    void init(int max_packets_);
    void teardown();
    void clear();
    void reset(RESET_MODE mode = RESET_INHERIT, packet_stream* pInherit = nullptr);
    void sync(bool force);
    void finalize(FINIALIZE_MODE mode = FINILIZE_TERMINATE);
    void execute();

    inline void BindProgram(GLuint program);
    inline void BindVertexArray(GLuint vao);
    inline void BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    inline void DrawElements(GLenum mode, GLsizei count, GLenum type, GLuint start, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
    inline void DrawArrays(GLenum mode, GLint first, GLsizei count, GLsizei primcount, GLuint baseinstance);
    inline void EnableDisable(GLenum cap, GLboolean enable);

private:
    unsigned int            max_packets;
    packet::ALL_PACKETS*    m_packets;
#ifdef ATOMIC_PACKET_BUFFER
    std::atomic_uint        num_packets;
#else
    unsigned int            num_packets;
#endif

    struct
    {
        union
        {
            struct
            {
                unsigned int        cull_face : 1;
                unsigned int        rasterizer_discard : 1;
                unsigned int        depth_test : 1;
                unsigned int        stencil_test : 1;
                unsigned int        depth_clamp : 1;
            };
            unsigned int            all_bits;
        } enables;

        union
        {
            struct
            {
                unsigned int        cull_face : 1;
                unsigned int        rasterizer_discard : 1;
                unsigned int        depth_test : 1;
                unsigned int        stencil_test : 1;
                unsigned int        depth_clamp : 1;
            };
            unsigned int            all_bits;
        } valid;
    } state;

#ifdef ATOMIC_PACKET_BUFFER
    template <typename T>
    T* NextPacket() { return reinterpret_cast<T*>(&m_packets[num_packets.fetch_add(1)]); }
#else
    template <typename T>
    T* NextPacket() { return reinterpret_cast<T*>(&m_packets[num_packets++]); }
#endif
};

void packet_stream::init(int max_packets_)
{
    max_packets = max_packets_;
    num_packets = 0;
    m_packets = new packet::ALL_PACKETS[max_packets];
    memset(m_packets, 0, max_packets * sizeof(packet::ALL_PACKETS));
}

void packet_stream::teardown()
{
    delete [] m_packets;
    m_packets = nullptr;
    max_packets = 0;
}

void packet_stream::clear()
{
    num_packets = 0;
}

void packet_stream::reset(packet_stream::RESET_MODE mode, packet_stream* pInherited)
{
    switch (mode)
    {
        case RESET_INHERIT:
            break;
        case RESET_RETURN_TO_DEFAULTS:
            break;
    }
}

void packet_stream::execute(void)
{
    const packet::ALL_PACKETS* __restrict pPacket;

    if (!num_packets)
        return;

    for (pPacket = m_packets; pPacket->execute != nullptr; pPacket++)
    {
        pPacket->execute((packet::base*)pPacket);
    }
}

void packet_stream::BindProgram(GLuint program)
{
    packet::BIND_PROGRAM* __restrict pPacket = NextPacket<packet::BIND_PROGRAM>();

    pPacket->pfnExecute = packet::PFN_EXECUTE(packet::BIND_PROGRAM::execute);
    pPacket->program = program;
}

void packet_stream::BindVertexArray(GLuint vao)
{
    packet::BIND_VERTEX_ARRAY* __restrict pPacket = NextPacket<packet::BIND_VERTEX_ARRAY>();

    pPacket->pfnExecute = packet::PFN_EXECUTE(packet::BIND_VERTEX_ARRAY::execute);
    pPacket->vao = vao;
}

void packet_stream::BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    packet::BIND_BUFFER_RANGE* __restrict pPacket = NextPacket<packet::BIND_BUFFER_RANGE>();

    pPacket->pfnExecute = packet::PFN_EXECUTE(packet::BIND_BUFFER_RANGE::execute);
    pPacket->target = target;
    pPacket->index = index;
    pPacket->buffer = buffer;
    pPacket->offset = offset;
    pPacket->size = size;
}

void packet_stream::DrawElements(GLenum mode, GLsizei count, GLenum type, GLuint start, GLsizei instancecount, GLint basevertex, GLuint baseinstance)
{
    packet::DRAW_ELEMENTS* __restrict pPacket = NextPacket<packet::DRAW_ELEMENTS>();
    
    pPacket->pfnExecute = packet::PFN_EXECUTE(packet::DRAW_ELEMENTS::execute);
    pPacket->mode = mode;
    pPacket->count = count;
    pPacket->indices = (GLvoid*)mode;
    pPacket->primcount = instancecount;
    pPacket->basevertex = basevertex;
    pPacket->baseinstance = baseinstance;
}

void packet_stream::DrawArrays(GLenum mode, GLint first, GLsizei count, GLsizei primcount, GLuint baseinstance)
{
    packet::DRAW_ARRAYS* __restrict pPacket = NextPacket<packet::DRAW_ARRAYS>();

    pPacket->pfnExecute = packet::PFN_EXECUTE(packet::DRAW_ARRAYS::execute);
    pPacket->mode = mode;
    pPacket->first = first;
    pPacket->count = count;
    pPacket->primcount = primcount;
    pPacket->baseinstance = baseinstance;
}

void packet_stream::EnableDisable(GLenum cap, GLboolean enable)
{
    switch (cap)
    {
        case GL_CULL_FACE:
            if (state.valid.cull_face == 1 &&
                state.enables.cull_face == enable)
                return;
            state.enables.cull_face = enable;
            state.valid.cull_face = 1;
            break;
        case GL_RASTERIZER_DISCARD:
            if (state.valid.rasterizer_discard == 1 &&
                state.enables.rasterizer_discard == enable)
                return;
            state.enables.rasterizer_discard = enable;
            state.valid.rasterizer_discard = 1;
            break;
        case GL_DEPTH_TEST:
            if (state.valid.depth_test == 1 &&
                state.enables.depth_test == enable)
                return;
            state.enables.depth_test = enable;
            state.valid.depth_test = 1;
            break;
        case GL_STENCIL_TEST:
            if (state.valid.stencil_test == 1 &&
                state.enables.stencil_test == enable)
                return;
            state.enables.stencil_test = enable;
            state.valid.stencil_test = 1;
            break;
        case GL_DEPTH_CLAMP:
            if (state.valid.depth_clamp == 1 &&
                state.enables.depth_clamp == enable)
                return;
            state.enables.depth_clamp = enable;
            state.valid.depth_clamp = 1;
            break;
        default:
            break;
    }

    packet::ENABLE_DISABLE* __restrict pPacket =
        NextPacket<packet::ENABLE_DISABLE>();

    if (enable)
    {
        pPacket->pfnExecute =
            packet::PFN_EXECUTE(packet::ENABLE_DISABLE::execute_enable);
    }
    else
    {
        pPacket->pfnExecute =
            packet::PFN_EXECUTE(packet::ENABLE_DISABLE::execute_disable);
    }
}

class packetrender_app : public sb7::application
{
public:
    packetrender_app()
    {

    }

    void startup();
    void render(double currentTime);
    void shutdown(void);

protected:
    packet_stream       stream;
    sb7::text_overlay   overlay;
};

void packetrender_app::startup()
{
    stream.init(256);

    const char* vs_source =
        "#version 440 core\n"
        "layout (binding = 0) uniform BLOCK\n"
        "{\n"
        "    vec4 vtx_color[4];\n"
        "};"
        "out vec4 vs_fs_color;\n"
        "void main(void)\n"
        "{\n"
        "    vs_fs_color = vtx_color[gl_VertexID & 3];\n"
        "    gl_Position = vec4((gl_VertexID & 2) - 1.0, (gl_VertexID & 1) * 2.0 - 1.0, 0.5, 1.0);\n"
        "}\n";

    const char * fs_source =
        "#version 440 core\n"
        "layout (location = 0) out vec4 color;\n"
        "in vec4 vs_fs_color;\n"
        "void main(void)\n"
        "{\n"
        "    color = vs_fs_color;\n"
        "}\n";

    GLuint vao;
    glGenVertexArrays(1, &vao);

    GLuint shaders[2] =
    {
        sb7::shader::from_string(vs_source, GL_VERTEX_SHADER),
        sb7::shader::from_string(fs_source, GL_FRAGMENT_SHADER)
    };

    GLuint program = sb7::program::link_from_shaders(shaders, 2, true);

    GLuint buffer;

    static const GLfloat colors[] =
    {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
    };

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferStorage(GL_UNIFORM_BUFFER, sizeof(colors), colors, 0);

    stream.BindProgram(program);
    stream.BindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, 0, sizeof(colors));
    stream.BindVertexArray(vao);
    stream.DrawArrays(GL_TRIANGLE_STRIP, 0, 4, 1, 0);

    sb7::object object;
    object.load("media/objects/sphere.sbm");

    shaders[0] = sb7::shader::load("media/shaders/blinnphong/blinnphong.vs.glsl", GL_VERTEX_SHADER);
    shaders[1] = sb7::shader::load("media/shaders/blinnphong/blinnphong.fs.glsl", GL_FRAGMENT_SHADER);

    program = sb7::program::link_from_shaders(shaders, 2, true);

    GLuint first;
    GLuint count;
    object.get_vao();
    object.get_sub_object_info(0, first, count);

    struct
    {
        vmath::mat4 mv_matrix;
        vmath::mat4 view_matrix;
        vmath::mat4 proj_matrix;
    } matrices;

    matrices.view_matrix = vmath::translate(0.0f, 0.0f, -1.0f);
    matrices.mv_matrix = matrices.view_matrix * vmath::rotate(30.0f, 0.0f, 1.0f, 0.0f);
    matrices.proj_matrix = vmath::frustum(-1.0f, 1.0f, 1.0f, -1.0f, 0.1f, 1000.0f);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_UNIFORM_BUFFER, buffer);
    glBufferStorage(GL_UNIFORM_BUFFER, sizeof(matrices), &matrices, 0);

    stream.BindProgram(program);
    stream.BindVertexArray(object.get_vao());
    stream.BindBufferRange(GL_UNIFORM_BUFFER, 0, buffer, 0, sizeof(matrices));
    stream.DrawArrays(GL_TRIANGLES, first, count, 1, 0);

    overlay.init(80, 40, nullptr);
}

void packetrender_app::render(double currentTime)
{
    static double lastTime = 0.0f;
    static unsigned int frames = 0;

    glViewport(0, 0, info.windowWidth, info.windowHeight);
    stream.execute();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_MAX);

    if ((currentTime - lastTime) > 1.0)
    {
        char buffer[128];
        sprintf(buffer, "%u frames in %1.3f seconds is %4.2f fps", frames, float(currentTime - lastTime), float(frames) / float(currentTime - lastTime));
        overlay.drawText(buffer, 0, 0);
        lastTime = currentTime;
        frames = 0;
    }
    overlay.draw();

    glDisable(GL_BLEND);
    frames++;
}

void packetrender_app::shutdown(void)
{
    stream.teardown();
    overlay.teardown();
}

DECLARE_MAIN(packetrender_app)
