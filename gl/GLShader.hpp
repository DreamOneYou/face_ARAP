#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace gl_face3d
{
class GLShader
{
public:
    virtual void draw() = 0;
protected:
    GLShader(const char* vertex, const char* fragment);

    int GLProgram;
};
}// namespace gl_face3d