#

set(_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/libManis")
set(_NAME libManis)

if(MSVC)
    set(libManis_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/libManis/Win")
    # windows' caffe depends on pthreads
    if("${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
        set(libManis_ROOT "${libManis_ROOT}/x86")
    else("${CMAKE_SIZEOF_VOID_P}" EQUAL "8") 
        set(libManis_ROOT "${libManis_ROOT}/x64")
    endif() 

    add_library(ManisStatic STATIC IMPORTED)

    set_property(TARGET ManisStatic APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(ManisStatic PROPERTIES
       IMPORTED_IMPLIB_DEBUG "${libManis_ROOT}/lib/Manisd.lib"
    )

    set_property(TARGET ManisStatic APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_target_properties(ManisStatic PROPERTIES
        IMPORTED_LOCATION_RELEASE "${libManis_ROOT}/lib/Manis.lib"
    )

    set(libManis_INCLUDE_DIRS "${libManis_ROOT}/include")
    set(libManis_LIBS ManisStatic)
    
else()
    if(ANDROID)
        set(libManis_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/libManis/android/${ANDROID_ABI}/")
        # debug properties
        add_library(libManis SHARED IMPORTED)
        set_target_properties(libManis PROPERTIES
        IMPORTED_LOCATION "${libManis_ROOT}/lib/libManis.so"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
        set(libManis_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/libManis/wasm/")
        # debug properties
        add_library(libManis SHARED IMPORTED)
        set_target_properties(libManis PROPERTIES
        IMPORTED_LOCATION "${libManis_ROOT}/lib/libManis.so"
    )

    elseif(APPLE)
        if(IOS_DEPLOYMENT_TARGET) # IOS 
            set(libManis_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/libManis/iOS/universal")
        else()              # macOS
            set(libManis_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/libManis/macOS/")
        endif()
        # debug properties
        set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
        add_library(libManis SHARED IMPORTED)
        set_target_properties(libManis PROPERTIES
            IMPORTED_LOCATION "${libManis_ROOT}/Manis.framework/Manis"
        )
    else()
#        if("${LINUX_DISTRIBUTION}" STREQUAL "ubuntu")
            set(libManis_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/libManis/ubuntu/")
#        elseif("${LINUX_DISTRIBUTION}" STREQUAL "centos")
#            set(libManis_ROOT "${CMAKE_SOURCE_DIR}/3rdparty/libManis/centos/")
#        endif()
        # debug properties
        add_library(libManis SHARED IMPORTED)
        
        set_target_properties(libManis PROPERTIES
            IMPORTED_LOCATION "${libManis_ROOT}/lib/libManis.so"
        )
    endif()
    if(APPLE)
        set(libManis_INCLUDE_DIRS "${libManis_ROOT}/Manis.framework" )
    else()
        set(libManis_INCLUDE_DIRS "${libManis_ROOT}/include" )
    endif()
    set(libManis_LIBS libManis)

endif()
file(GLOB libManis_DYLIBS "${libManis_ROOT}/lib/*.dll" "${libManis_ROOT}/lib/*.so" "${libManis_ROOT}/lib/*.dylib")
