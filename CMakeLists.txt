cmake_minimum_required(VERSION 3.8.2)
project(face_profiling VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
message("cmake module path" ${CMAKE_MODULE_PATH})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# This sets the C++ standard to c++17 and required for all the following targets that we define.
# It has no effect on MSVC though - we thus define more specific requirements for each executable target respectively.
# Also it will not apply to the eos library target, since it is an INTERFACE_LIBRARY, and these properties do not apply to interface libraries.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # This makes CMake use -std=c++xx instead of -std=gnu++xx

# for debug mode
option(DEBUG_MODE "Debug mode, plot contour and occuding points." OFF)
message(STATUS "DEBUG_MODE: ${DEBUG_MODE}")

message(STATUS "Options:")
option(BUILD_EXAMPLES "Build the example applications." ON)
message(STATUS "BUILD_EXAMPLES: ${BUILD_EXAMPLES}")

set(3RDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty") # this is used by the CMakeLists.txt files in the subdirectories

# Use the Eigen included in the submodule by default, but allow the user to override EIGEN3_INCLUDE_DIR:
#if (NOT DEFINED EIGEN3_INCLUDE_DIR)
#    set(EIGEN3_INCLUDE_DIR ${3RDPARTY_DIR}/eigen)
#endif ()

find_package(Eigen3 REQUIRED)
message(STATUS "Eigen3 found: ${EIGEN3_FOUND}, version: ${EIGEN3_VERSION}")
message(STATUS "Eigen3 include dir found at ${EIGEN3_INCLUDE_DIR}")

# Set the include directories of the 3rd-party submodules that we use:
set(glm_INCLUDE_DIR "${3RDPARTY_DIR}/glm")

if (DEBUG_MOD)
    find_package(Boost 1.50.0 COMPONENTS system filesystem program_options REQUIRED)
    target_include_directories(eos INTERFACE ${Boost_INCLUDE_DIRS})
endif (DEBUG_MOD)


if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

set(LIBIGL_USE_STATIC_LIBRARY OFF CACHE BOOL "" FORCE)
set(LIBIGL_WITH_TRIANGLE ON CACHE BOOL "" FORCE)
set(LIBIGL_WITH_TETGEN ON CACHE BOOL "" FORCE)

find_package(LIBIGL REQUIRED)
find_package(Manis REQUIRED)
add_subdirectory(3rdparty/fmt)
add_subdirectory(gl)
