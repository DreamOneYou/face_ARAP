#include <GLShow.hpp>
#include <fmt/format.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace gl_face3d
{
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

void glfw_error_callback(int error, const char* description)
{
    throw std::runtime_error(fmt::format("Glfw Error %d: %s\n", error, description));
}

GLShow::GLShow()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        throw std::runtime_error("");

    // Decide GL+GLSL versions
#if __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    glfwWindowHint(GLFW_SAMPLES, 16);

    GLFWwindow* window_ = glfwCreateWindow(1024, 768, "GLShow Window", NULL, NULL); \
        if (window_ == NULL)
            glfw_error_callback(-1, "glfwCreateWindow");
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        glfw_error_callback(-1, "Failed to initialize GLAD");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    this->window_ = window_;
}

GLShow::~GLShow()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window_);
    glfwTerminate();
}

void GLShow::setWindowsSize(int W, int H)
{
    glfwSetWindowSize(window_, W, H);
    return;
}

void GLShow::preShowHook(std::function<void(void)> f)
{
    pre_show_ = f;
}

void GLShow::endShowHook(std::function<void(const cv::Mat&)> f)
{
    after_show_ = f;
}

void GLShow::appendShader(GLShader *s)
{
    shader_list_.push_back(s);
    return;
}

void GLShow::removeShader()
{
    shader_list_.clear();
    return;
}

bool GLShow::show()
{
    if (glfwWindowShouldClose(window_))
        return false;

    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (pre_show_)
        pre_show_();

    ImGui::Render();

    int window_w, window_h;
    glfwMakeContextCurrent(window_);
    glfwGetFramebufferSize(window_, &window_w, &window_h);
    glViewport(0, 0, window_w, window_h);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Draw Object
    for (auto& shader : shader_list_)
        shader->draw();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwMakeContextCurrent(window_);
    glfwSwapBuffers(window_);

    if (after_show_)
    {
        cv::Mat frame(window_h, window_w, CV_8UC4);
        glPixelStorei(GL_PACK_ALIGNMENT, GLint((frame.step & 3) ? 1 : 4));
        glPixelStorei(GL_PACK_ROW_LENGTH, GLint(frame.step / frame.elemSize()));
        glReadPixels(0, 0, frame.cols, frame.rows, GL_BGRA, GL_UNSIGNED_BYTE, frame.data);
        cv::flip(frame, frame, 0);
        after_show_(frame);
    }

    return true;
}
}
