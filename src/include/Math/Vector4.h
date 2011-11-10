/********************************************************/
/* FILE: Vector4.h                                      */
/* DESCRIPTION: Raytrace Vector4 class                      */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include "MathTypes.h"
#include "VectorBase.h"
#include "Vector2.h"
#include "Vector3.h"


#ifndef Raytrace_MATH_VECTOR4_H_INCLUDED
#define Raytrace_MATH_VECTOR4_H_INCLUDED

namespace Math {

template<typename _T> struct Vector4 : public BaseVector<_T,4>
{
protected:
	typedef BaseVector<_T,4> Base;
	typedef typename Base::Base EigenBase;
public:
	typedef Vector4<_T> ThisType;

	// access operations

	inline _T& x()
	{
		return Base::operator[](0);
	}
	inline _T& y()
	{
		return Base::operator[](1);
	}
	inline _T& z()
	{
		return Base::operator[](2);
	}
	inline _T& w()
	{
		return Base::operator[](3);
	}

	// const access functions
	
	inline const _T& x() const
	{
		return Base::operator[](0);
	}
	inline const _T& y() const
	{
		return Base::operator[](1);
	}
	inline const _T& z() const
	{
		return Base::operator[](2);
	}
	inline const _T& w() const
	{
		return Base::operator[](3);
	}

	// conversion

	inline Vector3<_T> xyz() const
	{
		return Vector3<_T>(x(),y(),z());
	}
	inline Vector2<_T> xy() const
	{
		return Vector2<_T>(x(),y());
	}

	// constructors

	inline ThisType(const _T& _1,const _T& _2,const _T& _3,const _T& _4) : Base(_1,_2,_3,_4)
	{
	}

	template<class _T2> inline ThisType(const Vector3<_T2>& _v,const _T& _4) : Base((_T)_v.x(),(_T)_v.y(),(_T)_v.z(),(_T)_4)
	{
	}
	
	template<class _T2> inline ThisType(const Vector4<_T2>& _v) : Base((_T)_v.x(),(_T)_v.y(),(_T)_v.z(),(_T)_v.w)
	{
	}

	template<class _T2> inline ThisType(const _T2& _1,const _T2& _2,const _T2& _3,const _T2& _4) : Base((_T)_1,(_T)_2,(_T)_3,(_T)_4)
	{
	}

	inline ThisType()
	{
	}

	inline ThisType(const Base& _v) : Base(typename Base::Base(_v))
	{
	}
};

}

#endif
