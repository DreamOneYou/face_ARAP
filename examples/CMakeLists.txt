set(TEST_DIR "${CMAKE_CURRENT_LIST_DIR}")

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR} # base.hpp
)

# Check installed version in order to include the correct OpenCV libraries:
# First call find_package without a version to find any OpenCV. OpenCV_VERSION_MAJOR is then defined.
find_package(OpenCV REQUIRED core)
if("${OpenCV_VERSION_MAJOR}$" EQUAL 2)
  message(STATUS "OpenCV 2.x detected")
  find_package(OpenCV 2.4.3 REQUIRED core imgproc highgui)
elseif("${OpenCV_VERSION_MAJOR}$" EQUAL 3)
  message(STATUS "OpenCV 3.x detected - including imgcodecs for compatibility")
  find_package(OpenCV 3 REQUIRED)
elseif("${OpenCV_VERSION_MAJOR}$" EQUAL 4)
  message(STATUS "OpenCV 4.x detected - including imgcodecs for compatibility")
  find_package(OpenCV 4 REQUIRED core imgproc highgui imgcodecs)
endif()

# 添加json库
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})
include_directories("${JSONCPP_INCLUDE_DIRS}")
add_library(jsoncpp SHARED IMPORTED)
set_target_properties(jsoncpp PROPERTIES
  IMPORTED_LOCATION "/usr/local/lib/libjsoncpp.dylib"
  INTERFACE_INCLUDE_DIRECTORIES "/usr/local/include/json"
)

# This allows us to compile in RelWithDebInfo. It'll use the Release-version of OpenCV:
set_target_properties(${OpenCV_LIBS} PROPERTIES MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE)

if(MSVC)
  # The standard find_package for boost on Win finds the dynamic libs, so for dynamic linking to boost we need to #define:
  add_definitions(-DBOOST_ALL_NO_LIB) # Don't use the automatic library linking by boost with VS (#pragma ...). Instead, we specify everything here in cmake.
  add_definitions(-DBOOST_ALL_DYN_LINK) # Link against the dynamic boost lib - needs to match with the version that find_package finds.
  add_definitions(-D_HAS_AUTO_PTR_ETC) # Boost 1.65.1 still does not work with VS C++17 mode, this is the workaround
endif()
find_package(Boost 1.50.0 COMPONENTS system filesystem program_options REQUIRED)
message(STATUS "Boost found at ${Boost_INCLUDE_DIRS}")

set(HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/FaceDetector.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FaceReconstructor.hpp
)

macro(app_project filename)
    project(${filename}_pro)
    add_executable(${filename}_bin ${filename}.cpp arap.cpp)
    set_target_properties(${filename}_bin PROPERTIES FOLDER "App")

    target_link_libraries(${filename}_bin ${OpenCV_LIBS})
    target_link_libraries(${filename}_bin ${Boost_LIBRARIES})
    target_link_libraries(${filename}_bin "$<$<CXX_COMPILER_ID:GNU>:-pthread>$<$<CXX_COMPILER_ID:Clang>:-pthreads>")
    target_link_libraries(${filename}_bin glshow)
		target_link_libraries(${filename}_bin igl::core igl::triangle igl::tetgen)
    target_link_libraries(${filename}_bin fmt)
    target_link_libraries(${filename}_bin ${JSONCPP_LIBRARIES})
    # target_link_libraries(${PROJECT_NAME} "${OpenCV_LIBS}")
		target_include_directories(${filename}_bin PUBLIC ${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

    if(MSVC)
        # libmtface
        include_directories(${CMAKE_SOURCE_DIR}/3rdparty/libmtface/Win_shared/v150/x64/include)
        target_link_libraries(${filename}_bin ${CMAKE_SOURCE_DIR}/3rdparty/libmtface/Win_shared/v150/x64/lib/mtface.lib)

        # libmanis
        include_directories(${CMAKE_SOURCE_DIR}/3rdparty/libManis/Win/x64/include)
        target_link_libraries(${filename}_bin ${CMAKE_SOURCE_DIR}/3rdparty/libManis/Win/x64/lib/Manis.lib)

        add_custom_command(TARGET ${filename}_bin
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/3rdparty/libManis/Win/x64/lib/libManis.dll          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/3rdparty/libManis/Win/x64/lib/glew32.dll          ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/3rdparty/libmtface/Win_shared/v150/x64/lib/libmtface.dll   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/
        )
    elseif(APPLE)
        find_library(
						Manis
						NAMES Manis
            HINTS ${CMAKE_SOURCE_DIR}/3rdparty/libManis/macOS
            REQUIRED)
        find_library(
            mtface
            NAMES mtface
            HINTS ${CMAKE_SOURCE_DIR}/3rdparty/libmtface/macos_shared
            REQUIRED)
        target_link_libraries(${filename}_bin ${Manis} ${mtface})
        message("mtface:" ${mtface})
    endif()
endmacro(app_project)

app_project(face-profiling)



