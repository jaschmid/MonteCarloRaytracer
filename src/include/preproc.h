/********************************************************/
/* FILE: preproc.h                                      */
/* DESCRIPTION: Defines common preprocessor macros.     */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef RAYTRACE__PREPROCESSOR
#define RAYTRACE__PREPROCESSOR

#undef NDEBUG

#ifndef DATA_COLLECTION_LEVEL
#define DATA_COLLECTION_LEVEL 5
#endif

#ifndef _Windows
 #if defined(_WIN32)
  #define _Win32
  #define _Windows
 #elif defined(WIN32)
  #define _Win32
  #define _Windows
 #elif defined(__WIN32__)
  #define _Win32
  #define _Windows
 #elif defined(__Win32__)
  #define _Win32
  #define _Windows
 #elif defined(_WINDOWS)
  #define _Windows
 #elif defined(__INTEL__) && defined(__MWERKS__)
 // Metrowerks CodeWarrior doesn't build anything other than Win32 on INTEL, no DOS
  #define _Windows
  #define _Win32
 #endif
#else
 #if defined __Win32__ || defined _WIN32
  #ifndef _Win32
   #define _Win32
  #endif
 #endif
#endif

#if defined(_Windows)
    #define TARGET_WINDOWS
#elif defined(__unix__) || defined(unix)
    #define TARGET_LINUX
#else
    #define TARGET_MACOS
#endif

#ifdef _MSC_VER
    #define COMPILER_MSVC
    #define WEAK_EXTERNAL
	#define ALIGN_SIMD __declspec(align(16))
	#define ALIGN_CACHE __declspec(align(64))
    #pragma warning( disable :4996)
#elif defined(__GNUC__)
    #define COMPILER_GCC
    #define WEAK_EXTERNAL __attribute__ ((visibility("protected")))
	#define ALIGN_SIMD __attribute__((__align__(16)))
	#define ALIGN_CACHE __attribute__((__align__(64)))
#else
    #error UNKNOWN COMPILER
#endif

#if defined(_WIN64) || defined(__LP64__)
    #define TARGET_X64
#else
    #define TARGET_X86
#endif

#endif
