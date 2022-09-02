cmake_minimum_required(VERSION 3.1)

add_library(imgui
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_widgets.cpp
	examples/imgui_impl_glfw.cpp
	examples/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PRIVATE ./)
target_include_directories(imgui PUBLIC ./ ./examples)

target_link_libraries(imgui PRIVATE glad glfw)
