#ifndef _MANIS_EXPORT_HPP__
#define _MANIS_EXPORT_HPP__

// EXPORT
#if defined(_WIN32) || defined(_WIN32_) || defined(WIN32) || defined(_WIN64_) || defined(WIN64) || defined(_WIN64)
	#ifdef MANIS_DYLIB
	#define MANIS_EXPORT __declspec(dllexport)
	#else
        #ifdef IMPORT_MANIS_DYLIB
        #define MANIS_EXPORT __declspec(dllimport)
        #else
        #define MANIS_EXPORT
        #endif
	#endif
#elif defined(_ADNROID_) || defined(ANDROID) || defined(__APPLE__) || defined(__linux__)
	#ifdef MANIS_DYLIB
	#define MANIS_EXPORT __attribute__((visibility("default")))
	#else
	#define MANIS_EXPORT
	#endif
#else
	#define MANIS_EXPORT
#endif

#if defined(__GNUC__) || defined(__clang__)
#define _GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
    #if _GCC_VERSION <  40500
        #define _MANIS_DEPRECATED(explanation)   __attribute__((deprecated))
    #else
        #define _MANIS_DEPRECATED(explanation)   __attribute__((deprecated(explanation)))
    #endif
#else
    #if defined(_MSC_VER)|| defined (__INTEL_COMPILER)  
        #define _MANIS_DEPRECATED(EXPLANATION) __declspec(deprecated(EXPLANATION))
    #else
        #define _MANIS_DEPRECATED(explanation) 
    #endif
#endif

//disable class copy and assign
#ifndef MANIS_DISABLE_COPY_ASSIGN
#define MANIS_DISABLE_COPY_ASSIGN(CLASSNAME)        \
private:                                            \
    CLASSNAME(const CLASSNAME &) = delete;          \
    CLASSNAME &operator=(const CLASSNAME &) = delete
#endif

#endif
