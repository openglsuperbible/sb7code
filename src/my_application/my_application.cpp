#include <sb7.h>

GLuint compileShaders() {
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;

    static const GLchar* vsSource[] = {
        "#version 440 core\n"
        "layout (location = 0) in vec4 offset;"
        "layout (location = 1) in vec4 color;"
        "out VS_OUT {"
        "vec4 color;"
        "} vs_out;"
        "void main() {\n"
        "const vec4 vertices[] = vec4[]("
        "vec4( 0.25,  -0.25, 0.5, 1.0),"
        "vec4( -0.25,  -0.25, 0.5, 1.0),"
        "vec4( 0.25,  0.25, 0.5, 1.0));"
        "gl_Position = vertices[gl_VertexID] + offset;\n"
        "vs_out.color.r = ((color * tan(gl_VertexID)) * 0.5 + 0.5).g;" 
        "vs_out.color.g = ((color * sin(gl_VertexID)) * 0.5 + 0.5).r;" 
        "vs_out.color.b = ((color * cos(gl_VertexID)) * 0.5 + 0.5).b;" 
        "}"
    };

    static const GLchar* fsSource[] = {
        "#version 440 core\n"
        "in VS_OUT {"
        "vec4 color;"
        "} fs_in;"
        "out vec4 color;\n"
        "void main() {\n"
        "color = fs_in.color;\n"
        "}"
    };

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, vsSource, NULL);
    glCompileShader(vertexShader);

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, fsSource, NULL);
    glCompileShader(fragmentShader);

    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}


class my_application : public sb7::application {
    public:
        void startup() {
            rendering_program = compileShaders();
            glGenVertexArrays(1, &vertex_array_object);
            glBindVertexArray(vertex_array_object);
            glPointSize(20.0f);
        }

        void shutdown() {
            glDeleteVertexArrays(1, &vertex_array_object);
            glDeleteProgram(rendering_program);
        }

        void render(double currentTime) {
            GLfloat clear_color[] = {
                0.1f,
                0.1f, 
                0.1f,
                1.0f
            };

            GLfloat color[] = {
                (GLfloat) cos(currentTime) * 0.5f + 0.5f,
                (GLfloat) sin(currentTime) * 0.5f + 0.5f,
                (GLfloat) sin(currentTime) * 0.5f + 0.5f,
                1.0f
            };

            GLfloat offset[] = {
                0.0f,
                0.0f,
                0.0f,
                0.0f
            };

            glClearBufferfv(GL_COLOR, 0, clear_color);

            glUseProgram(rendering_program);

            glVertexAttrib4fv(0, offset);
            glVertexAttrib4fv(1, color);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    private:
        GLuint rendering_program;
        GLuint vertex_array_object;

};

DECLARE_MAIN(my_application);
