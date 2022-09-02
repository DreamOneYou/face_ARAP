# mtface 引用规则
# 包含以下可用变量
# libmtface_INCLUDE_DIRS            头文件目录
# libmtface_LIBS                    LinkTarget
# libmtface_DYLIBS                  动态库
# libmtface_MODELS                  模型

set(_ROOT "${CMAKE_CURRENT_LIST_DIR}/../3rdparty/libmtface")

if(MSVC)
    # windows
    set(libmtface_ROOT "${_ROOT}/Win")
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
        set(libmtface_ROOT "${libmtface_ROOT}/v150/Win32")
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
        set(libmtface_ROOT "${libmtface_ROOT}/v150/x64")
    endif("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")

    add_library(mtface STATIC IMPORTED)
    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set_target_properties(mtface PROPERTIES IMPORTED_LOCATION "${libmtface_ROOT}/lib/libmtfaced.lib")
    else("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        set_target_properties(mtface PROPERTIES IMPORTED_LOCATION "${libmtface_ROOT}/lib/libmtface.lib")
    endif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

    set(libmtface_LIBS mtface)
    add_definitions(-DMEITUFDFA_SOURCE)
else()
    if(ANDROID)
        # android
        set(libmtface_ROOT "${_ROOT}/android/${ANDROID_ABI}")
        add_library(libmtface STATIC IMPORTED)
        set_target_properties(libmtface PROPERTIES IMPORTED_LOCATION "${libmtface_ROOT}/lib/libmtface.a")
    elseif(APPLE)
        if(IOS_DEPLOYMENT_TARGET)
            # ios
            set(libmtface_ROOT "${_ROOT}/iOS")
        else()
            # macos
            set(libmtface_ROOT "${_ROOT}/macos_shared")
        endif()
        add_library(libmtface STATIC IMPORTED)
        set_target_properties(libmtface PROPERTIES IMPORTED_LOCATION "${libmtface_ROOT}/mtface.framework/mtface")
        # message("libmtface_ROOT-----------:" libmtface_ROOT)
        # set(libmtface_FRAMEWORKS "${libmtface_ROOT}/mtface.framework")
    else()
        # linux
        add_library(libmtface SHARED IMPORTED)
#        if("${LINUX_DISTRIBUTION}" STREQUAL "ubuntu")
            set(libmtface_ROOT "${_ROOT}/ubuntu")
            set_target_properties(libmtface PROPERTIES IMPORTED_LOCATION "${libmtface_ROOT}/lib/libmtface.so")

#        elseif("${LINUX_DISTRIBUTION}" STREQUAL "centos")
#            set(libmtface_ROOT "${_ROOT}/centos")
#            set_target_properties(libmtface PROPERTIES IMPORTED_LOCATION "${libmtface_ROOT}/lib/libmtface.a")
#            message("222222")
#        endif()
        set(libmtface_DYLIBS "${libmtface_ROOT}/lib/libmtface.so")
    endif()
    set(libmtface_LIBS libmtface)
endif()

if(APPLE)
    set(libmtface_INCLUDE_DIRS "${libmtface_ROOT}/mtface.framework" )
    
else()
    set(libmtface_INCLUDE_DIRS "${libmtface_ROOT}/include" )
endif()
set(libmtface_MODELS "${libmtface_ROOT}/models")
# message("libmtface_INCLUDE_DIRS:------" ${libmtface_INCLUDE_DIRS})