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
#include <Eigen/Eigen>

namespace Math {

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
