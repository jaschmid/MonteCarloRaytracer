/********************************************************/
/* FILE: MathTypes.h                                    */
/* DESCRIPTION: Basic Math related definitions          */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 


#include "types.h"

#ifndef Raytrace_MATH_TYPES_H_INCLUDED
#define Raytrace_MATH_TYPES_H_INCLUDED


#include <limits>
#include <cmath>
#include <boost/math/special_functions/fpclassify.hpp>

namespace Math {
	
//preproc

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
    #pragma warning( disable :4996)
#elif defined(__GNUC__)
    #define COMPILER_GCC
    #define WEAK_EXTERNAL __attribute__ ((visibility("protected")))
	//olol gcc does not support nullptr

	const                        // this is a const object...
	class {
	public:
	  template<class T>          // convertible to any type
		operator T*() const      // of null non-member
		{ return 0; }            // pointer...
	  template<class C, class T> // or any type of null
		operator T C::*() const  // member pointer...
		{ return 0; }
	private:
	  void operator&() const;    // whose address can't be taken
	} nullptr = {};
#else
    #error UNKNOWN COMPILER
#endif

#if defined(_WIN64) || defined(__LP64__)
    #define TARGET_X64
#else
    #define TARGET_X86
#endif

//types

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

template<typename _T> const _T Zero()
{
	return (_T)0.0;
}
template<typename _T> const _T One()
{
	return (_T)1.0;
}
template<typename _T> const _T NaN()
{
	return (_T)std::numeric_limits<_T>::quiet_NaN();
}
template<typename _T> const _T Infinity()
{
	return (_T)std::numeric_limits<_T>::infinity();
}

template<typename _T> bool IsZero(const _T& v)
{
	return v == Zero<_T>();
}
template<typename _T> bool IsOne(const _T& v)
{
	return v == One<_T>();
}
template<typename _T> bool IsNaN(const _T& v)
{
	return boost::math::isnan(v);
}
template<typename _T> bool IsInfinite(const _T& v)
{
	return boost::math::isinf(v);
}
template<typename _T> bool IsFinite(const _T& v)
{
	return boost::math::isfinite(v);
}

template<typename _T> _T sqrt(const _T& v)
{
	return std::sqrt(v);
}

template<> static f32 sqrt<f32>(const f32& v)
{
	return std::sqrtf(v);
}

template<> static f64 sqrt<f64>(const f64& v)
{
	return std::sqrtl(v);
}

template<class _T = f32> struct Vector2;
template<class _T = f32> struct Vector3;
template<class _T = f32> struct Vector4;
template<class _T = f32> struct Matrix2;
template<class _T = f32> struct Matrix3;
template<class _T = f32> struct Matrix4;
template<class _T = f32> struct Quaternion;
template<class _T = f32> struct Complex;

// Generic types

template<class _T,int _Dimensions> struct GVector
{
	typedef void type;
};
template<class _T,int _Dimensions> struct GMatrix
{
	typedef void type;
};
template<class _T,int _Dimensions> struct GRotation
{
	typedef void type;
};

// specific Generic types

template<class _T> struct GVector<_T,1>
{
	typedef _T type;
};
template<class _T> struct GVector<_T,2>
{
	typedef Vector2<_T> type;
};
template<class _T> struct GVector<_T,3>
{
	typedef Vector3<_T> type;
};
template<class _T> struct GVector<_T,4>
{
	typedef Vector4<_T> type;
};
template<class _T> struct GMatrix<_T,1>
{
	typedef _T type;
};
template<class _T> struct GMatrix<_T,2>
{
	typedef Matrix2<_T> type;
};
template<class _T> struct GMatrix<_T,3>
{
	typedef Matrix3<_T> type;
};
template<class _T> struct GMatrix<_T,4>
{
	typedef Matrix4<_T> type;
};
template<class _T> struct GRotation<_T,1>
{
	typedef _T type;
};
template<class _T> struct GRotation<_T,2>
{
	typedef Complex<_T> type;
};
template<class _T> struct GRotation<_T,3>
{
	typedef Quaternion<_T> type;
};

}

#endif
