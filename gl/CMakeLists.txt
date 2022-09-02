file(GLOB GLSHOW_PUBLIC_HDRS ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
file(GLOB GLSHOW_SRC ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

set(GLSHOW_LIBRARY_SOURCE
    ${GLSHOW_PUBLIC_HDRS}
    ${GLSHOW_SRC})

add_library(glshow ${GLSHOW_LIBRARY_SOURCE})

target_include_directories(glshow PUBLIC ${CERES_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR})

# opengl
find_package(OpenGL REQUIRED)
target_include_directories(glshow PRIVATE ${OPENGL_INCLUDE_DIR})
target_link_libraries(glshow ${OPENGL_gl_LIBRARY})

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_USE_HYBRID_HPG OFF CACHE BOOL "" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glfw glfw)
target_link_libraries(glshow glfw)

# glad
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/glad glad)
target_link_libraries(glshow glad)

# imgui
add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD=1)
add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/imgui imgui)
target_link_libraries(glshow imgui)

# opencv
find_package(OpenCV REQUIRED)
target_include_directories(glshow PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(glshow ${OpenCV_LIBS})

# fmt
target_link_libraries(glshow fmt)
