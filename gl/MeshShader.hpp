#include <GLShader.hpp>

namespace gl_face3d
{
class MeshShader : public GLShader
{
public:
    MeshShader();

    void setMeshVertex(const float* v, const int size);

    void setMeshNormal(const float* v, const int size);

    void setMeshFace(const int* v, const int size);

    void setProjMatrix(const float* m);

    void setModelMatrix(const float* m);

    void setLineMode(bool m);

    void setTexture(const unsigned char* bgra, int w, int h);

    void setTexPosition(const float* xy, const int size);

    virtual void draw() override;

private:
    GLuint vao_, vbo_, ebo_, normal_vbo_, tex_, tex_vbo_;

    int face_size_ = 0;

    bool line_mode_ = false;

    float proj[16];

    float model[16];
};
}// namespace gl_face3d