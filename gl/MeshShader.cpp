#include "MeshShader.hpp"
#include <algorithm>

static const char *vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"out vec3 normal;\n"
"out vec2 texcoord;\n"
"uniform mat4 pose;\n"
"uniform mat4 proj;\n"
"void main()\n"
"{\n"
"   gl_Position = proj * pose * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"	normal = aNormal;\n"
"	texcoord = aTexCoord;\n"
"}\0";
static const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 normal;\n"
"in vec2 texcoord;\n"
"uniform sampler2D ourTexture;\n"
"void main()\n"
"{\n"
"   FragColor = texture(ourTexture, texcoord);\n"
"}\n\0";
namespace gl_face3d
{
MeshShader::MeshShader()
    :GLShader(vertexShaderSource, fragmentShaderSource)
{
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &normal_vbo_);
    glGenBuffers(1, &ebo_);
    glGenBuffers(1, &tex_vbo_);
    glGenTextures(1, &tex_);
}

void MeshShader::setMeshVertex(const float* v, const int size)
{
    glBindVertexArray(vao_);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), v, GL_STREAM_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
}

void MeshShader::setMeshNormal(const float* f, const int size)
{
    glBindVertexArray(vao_);
    {
        glBindBuffer(GL_ARRAY_BUFFER, normal_vbo_);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), f, GL_STREAM_DRAW);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
}

void MeshShader::setMeshFace(const int* v, const int size)
{
    glBindVertexArray(vao_);
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(int), v, GL_STATIC_DRAW);
    }
    glBindVertexArray(0);
    face_size_ = size;
}

void MeshShader::setProjMatrix(const float * m)
{
    std::copy(m, m + 16, proj);
    return;
}

void MeshShader::setModelMatrix(const float * m)
{
    std::copy(m, m + 16, model);
    return;
}

void MeshShader::setLineMode(bool m)
{
    line_mode_ = m;
}

void MeshShader::setTexture(const unsigned char * bgra, int w, int h)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, bgra); // GL_BGR_EXT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void MeshShader::setTexPosition(const float * xy, const int size)
{
    glBindVertexArray(vao_);
    {
        glBindBuffer(GL_ARRAY_BUFFER, tex_vbo_);
        glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), xy, GL_STATIC_DRAW);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);
}

void MeshShader::draw()
{
    glPolygonMode(GL_FRONT_AND_BACK, line_mode_ ? GL_LINE : GL_FILL);
    glUseProgram(GLProgram);

    int Loc = glGetUniformLocation(GLProgram, "proj");
    glUniformMatrix4fv(Loc, 1, GL_FALSE, proj);
    Loc = glGetUniformLocation(GLProgram, "pose");
    glUniformMatrix4fv(Loc, 1, GL_FALSE, model);

    glBindVertexArray(vao_);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, face_size_, GL_UNSIGNED_INT, 0);

    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(0);
}
}