#pragma once
#include <list>
#include <memory>
#include <functional>
#include <opencv2/opencv.hpp>
#include <GLShader.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#ifdef _MSC_VER
#include <commdlg.h>
#endif

struct GLFWwindow;

namespace gl_face3d
{
class GLShow
{
public:
    GLShow();
    ~GLShow();

    GLShow(const GLShow&) = delete;
    GLShow(const GLShow&&) = delete;
    GLShow& operator=(const GLShow&) = delete;

public:
    void setWindowsSize(int W, int H);
    void preShowHook(std::function<void(void)>);
    void endShowHook(std::function<void(const cv::Mat&)>);
    void appendShader(GLShader*);
    void removeShader();
    bool show();
private:
    std::list<GLShader*> shader_list_;
    GLFWwindow* window_;
    std::function<void(void)> pre_show_;
    std::function<void(const cv::Mat&)> after_show_;
};
}// namespace gl_face3d
