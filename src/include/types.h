/********************************************************/
/* FILE: types.h                                        */
/* DESCRIPTION: Declares basic types for HAGE.          */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include "preproc.h"

namespace Raytrace {

typedef float f32;
typedef double f64;

#ifdef COMPILER_MSVC
    typedef unsigned __int64 u64;
    typedef signed __int64 i64;
    typedef unsigned long u32;
    typedef signed long i32;
#elif defined(COMPILER_GCC)
    typedef unsigned long long u64;
    typedef signed long long i64;
    typedef unsigned int u32;
    typedef signed int i32;
#else
    #error UNKNOWN COMPILER
#endif

typedef unsigned short u16;
typedef signed short i16;
typedef unsigned char u8;
typedef signed char i8;

#ifdef TARGET_X64
    #ifdef COMPILER_MSVC
        typedef unsigned __int64 up;
        typedef signed __int64 ip;
    #else
        typedef unsigned long long up;
        typedef signed long long ip;
    #endif
#elif defined(TARGET_X86)
    typedef unsigned long up;
    typedef signed long ip;
#else
    #error UNKNOWN TARGET
#endif

}

#endif
