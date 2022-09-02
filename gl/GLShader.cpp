#include "GLShader.hpp"

namespace gl_face3d
{
extern void glfw_error_callback(int error, const char* description);
GLShader::GLShader(const char* vertex, const char* fragment)
{
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        glfw_error_callback(-1, infoLog);
    }
    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        glfw_error_callback(-1, infoLog);
    }
    // link shaders
    GLProgram = glCreateProgram();
    glAttachShader(GLProgram, vertexShader);
    glAttachShader(GLProgram, fragmentShader);
    glLinkProgram(GLProgram);
    // check for linking errors
    glGetProgramiv(GLProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(GLProgram, 512, NULL, infoLog);
        glfw_error_callback(-1, infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
}