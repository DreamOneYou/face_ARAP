#pragma once

// MTFACE_API 
#if defined(_WIN32_) || defined(WIN32) || defined(_WIN64_) || defined(WIN64)
    #if defined(MTFACE_BUILD_LIB)
        #define MTFACE_API  __declspec(dllexport)
    #elif defined(MTFACE_SOURCE_DEV)
        #define MTFACE_API 
    #else
        #define MTFACE_API  __declspec(dllimport)
    #endif
#elif defined(_ADNROID_) || defined(ANDROID) || defined(__APPLE__) || defined(__linux__)
    #if defined(MTFACE_BUILD_LIB)
        #define MTFACE_API __attribute__((visibility("default"))) 
    #else
        #define MTFACE_API
    #endif
#else
    #define MTFACE_API
#endif
