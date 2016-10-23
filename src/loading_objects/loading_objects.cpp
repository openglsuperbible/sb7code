#include <sb7.h>
#include <object.h>
#include <vmath.h>
#include <typeinfo>
#include "helpers.h"

class LoadingObjectsExample : public sb7::application {
    private:
        sb7::object     m_cube;
        GLuint          m_program;
        vmath::mat4     m_projectionMatrix;
        vmath::mat4     m_mvpMatrix;

    public:
        void startup() {
            m_program = compileShaders();

            printf("Starting %s\n", typeid(this).name());
            m_cube.load("../../bin/media/objects/cube.sbm");

            m_projectionMatrix = vmath::perspective(50.0f, 800.0f / 600.0f, 0.1f, 1000.0f);

            glEnable(GL_CULL_FACE);
        }

        void shutdown() {

        }

        void render(double time) {
            static const GLfloat clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };

            glClearBufferfv(GL_COLOR, 0, clearColor);

            m_mvpMatrix = m_projectionMatrix * vmath::translate(0.0f, 0.0f, -8.0f) * vmath::rotate((float) time * 45.0f, 0.0f, 1.0f, 0.0f) * vmath::rotate((float) time * 45.0f, 1.0f, 0.0f, 0.0f);

            glUseProgram(m_program);
            glUniformMatrix4fv(9, 1, GL_FALSE, m_mvpMatrix);
            m_cube.render();
        }
};

DECLARE_MAIN(LoadingObjectsExample);
