#include <sb7.h>
#include "helpers.h"

class Buffers : public sb7::application {
    private:
        GLuint      m_VBO;
        GLuint      m_VAO;
        GLuint      m_IBO;
        GLuint      m_colorBuffer;
        GLuint      m_colorBuffer2;
        GLuint      m_program;

    public:
        void startup() {
            m_program = compileShaders();

            static const float data[] = {
                0.25f, -0.55f, 0.5f, 1.0f,
                -0.25f, -0.25f, 0.5f, 1.0f,
                0.25f, 0.25f, 0.5f, 1.0f,
            };

            static const float colors[] = {
                0.8f, 0.5f, 0.5f, 1.0f,
                0.2f, 0.2f, 0.6f, 1.0f,
                0.5f, 1.0f, 0.5f, 1.0f,
            };

            static const float colors2[] = {
                0.0f, 1.0f, 0.0f, 1.0f,
                0.0f, 0.0f, 0.6f, 1.0f,
                0.8f, 0.0f, 0.0f, 1.0f,
            };

            static const GLuint indices[] = { 0, 1, 2 };

            glCreateVertexArrays(1, &m_VAO);
            glBindVertexArray(m_VAO);

            glCreateBuffers(1, &m_VBO);
            printf("Size of data[]: %lu bytes\n", sizeof(data));
            glNamedBufferStorage(m_VBO, sizeof(data), data, GL_DYNAMIC_STORAGE_BIT);
            glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, 4 * sizeof(GLfloat));
            glVertexArrayAttribFormat(m_VAO, 0, 4, GL_FLOAT, GL_FALSE, 0);
            glEnableVertexAttribArray(0);
            
            glCreateBuffers(1, &m_colorBuffer);
            glNamedBufferStorage(m_colorBuffer, sizeof(colors), colors, GL_DYNAMIC_STORAGE_BIT);
            glVertexArrayVertexBuffer(m_VAO, 1, m_colorBuffer, 0, 4 * sizeof(GLfloat));
            glVertexArrayAttribFormat(m_VAO, 1, 4, GL_FLOAT, GL_FALSE, 0);
            glEnableVertexAttribArray(1);

            glCreateBuffers(1, &m_colorBuffer2);
            glNamedBufferStorage(m_colorBuffer2, sizeof(colors2), colors2, GL_DYNAMIC_STORAGE_BIT);
            glVertexArrayVertexBuffer(m_VAO, 2, m_colorBuffer2, 0, 4 * sizeof(GLfloat));
            glVertexArrayAttribFormat(m_VAO, 2, 4, GL_FLOAT, GL_FALSE, 0);

        }

        void render(double time) {
            static const GLfloat clear_color[] = {
                0.1f,
                0.1f, 
                0.1f,
                1.0f
            };

            glClearBufferfv(GL_COLOR, 0, clear_color);

            glUseProgram(m_program);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glVertexArrayAttribBinding(m_VAO, 0, 1);
            glVertexArrayAttribBinding(m_VAO, 1, 2);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glVertexArrayAttribBinding(m_VAO, 0, 2);
            glVertexArrayAttribBinding(m_VAO, 1, 0);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glVertexArrayAttribBinding(m_VAO, 0, 0);
            glVertexArrayAttribBinding(m_VAO, 1, 1);
        }

        void shutdown() {
            glDeleteVertexArrays(1, &m_VAO);
        }

};

DECLARE_MAIN(Buffers);
