#include <GLShader.hpp>

namespace gl_face3d
{
class BackgroundShader : public GLShader
{
public:
    BackgroundShader();

    void setImage(const unsigned char* bgra, int w, int h);

    virtual void draw() override;
private:
    GLuint vao_, vbo_, ebo_;
    GLuint tex_;
};
}// namespace gl_face3d