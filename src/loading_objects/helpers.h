GLuint compileShaders() {
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint program;

    static const GLchar* vsSource[] = {
        "#version 450 core\n"
        "layout (location = 0) in vec4 vertex;"
        "layout (location = 1) in vec4 color;"
        "layout (location = 9) uniform mat4 mvpMatrix;\n"
        "out VS_OUT {"
        "vec4 color;"
        "} vs_out;"
        "void main() {\n"
        "gl_Position = mvpMatrix * vertex;\n"
        "vs_out.color = color;\n"
        "}"
    };

    static const GLchar* fsSource[] = {
        "#version 450 core\n"
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
