#
# OpenCV_INCLUDE_DIRS
# OpenCV_LIBS
#

set(_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/libopencv")
set(_NAME libopencv)

if(MSVC)
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "4") 
        set(libopencv_ROOT "${_ROOT}/win/x86")
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        set(libopencv_ROOT "${_ROOT}/win/x64")
    endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "4") 

    # debug properties
    set(OpenCV_INCLUDE_DIRS "${libopencv_ROOT}/include" "${libopencv_ROOT}/include/opencv")	
    foreach(opencv_module zlib opencv_video opencv_calib3d opencv_features2d opencv_imgproc opencv_core)
        add_library(${opencv_module} STATIC IMPORTED)

        set_property(TARGET ${opencv_module} APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(${opencv_module} PROPERTIES
            IMPORTED_LOCATION_DEBUG "${libopencv_ROOT}/lib/${opencv_module}d.lib"
        )
        set_property(TARGET ${opencv_module} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(${opencv_module} PROPERTIES
            IMPORTED_LOCATION_RELEASE "${libopencv_ROOT}/lib/${opencv_module}.lib"
        )
    endforeach(opencv_module)
    set(OpenCV_LIBS zlib opencv_video opencv_calib3d opencv_features2d opencv_imgproc opencv_core)

else() # *nix platform

    if(ANDROID)
        set(libopencv_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/${_NAME}/android/${BUILD_ARCH}/")
        message(STATUS "opencn_root ${libopencv_ROOT}")
        foreach(opencv_module opencv_video opencv_calib3d opencv_features2d opencv_imgproc opencv_core)
            add_library(${opencv_module} STATIC IMPORTED)
            set_target_properties(${opencv_module} PROPERTIES
                IMPORTED_LOCATION "${libopencv_ROOT}/lib/lib${opencv_module}.a"
            )
            message(STATUS "cv path: ${libopencv_ROOT}/lib/lib${opencv_module}.a")
        endforeach(opencv_module)
        # find_package(ZLIB REQUIRED)
        set(OpenCV_LIBS opencv_video opencv_calib3d opencv_features2d opencv_imgproc opencv_core ${ZLIB_LIBRARIES})
        set(OpenCV_INCLUDE_DIRS "${libopencv_ROOT}/include" "${libopencv_ROOT}/include/opencv" ${ZLIB_INCLUDE_DIRS})

    elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        set(libopencv_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/${_NAME}/wasm")
        message(STATUS "opencv_root ${libopencv_ROOT}")
        foreach(opencv_module opencv_video opencv_calib3d opencv_features2d opencv_imgproc opencv_core)
            add_library(${opencv_module} STATIC IMPORTED)
            set_target_properties(${opencv_module} PROPERTIES
                IMPORTED_LOCATION "${libopencv_ROOT}/lib/lib${opencv_module}.a"
            )
            message(STATUS "cv path: ${libopencv_ROOT}/lib/lib${opencv_module}.a")
        endforeach(opencv_module)
        set(OpenCV_LIBS opencv_video opencv_calib3d opencv_features2d opencv_imgproc opencv_core ${ZLIB_LIBRARIES})
        set(OpenCV_INCLUDE_DIRS "${libopencv_ROOT}/include" "${libopencv_ROOT}/include/opencv" ${ZLIB_INCLUDE_DIRS})
        
    elseif(APPLE)
        if(IOS_DEPLOYMENT_TARGET) # IOS 
            set(libopencv_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/${_NAME}/iOS/")
        else()              # macOS
            set(libopencv_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/${_NAME}/macos/")
        endif()
        find_package(ZLIB REQUIRED)
        # set(OpenCV_INCLUDE_DIRS "${libopencv_ROOT}/opencv2.framework" ${ZLIB_INCLUDE_DIRS})
        set(OpenCV_INCLUDE_DIRS "${libopencv_ROOT}/opencv2.framework")
        # message("ZLIB_INCLUDE_DIRS: " ${ZLIB_INCLUDE_DIRS})
        set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
        add_library(libopencv SHARED IMPORTED)
        set_target_properties(libopencv PROPERTIES
            IMPORTED_LOCATION "${libopencv_ROOT}/opencv2.framework/opencv2"
        )
		set(OpenCV_LIBS libopencv)

    elseif(UNIX)
        if("${LINUX_DISTRIBUTION}" STREQUAL "ubuntu")
            set(libopencv_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/${_NAME}/ubuntu")
        elseif("${LINUX_DISTRIBUTION}" STREQUAL "centos")
            set(libopencv_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/${_NAME}/centos")
        endif()
        foreach(opencv_module opencv_video opencv_calib3d opencv_features2d opencv_imgproc opencv_core)
            add_library(${opencv_module} STATIC IMPORTED)
            set_target_properties(${opencv_module} PROPERTIES
                IMPORTED_LOCATION "${libopencv_ROOT}/lib/lib${opencv_module}.a"
            )
            message(STATUS "cv path: ${libopencv_ROOT}/lib/lib${opencv_module}.a")
        endforeach(opencv_module)
        find_package(ZLIB REQUIRED)
        set(OpenCV_LIBS opencv_video opencv_calib3d opencv_features2d opencv_imgproc opencv_core ${ZLIB_LIBRARIES})
        set(OpenCV_INCLUDE_DIRS "${libopencv_ROOT}/include" "${libopencv_ROOT}/include/opencv" ${ZLIB_INCLUDE_DIRS})
    endif()
endif()