/********************************************************/
/* FILE: Vector3.h                                      */
/* DESCRIPTION: Raytrace Vector3 class                      */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include "MathTypes.h"
#include "VectorBase.h"
#include "Vector2.h"


#ifndef Raytrace_MATH_VECTOR3_H_INCLUDED
#define Raytrace_MATH_VECTOR3_H_INCLUDED

namespace Math {



template<typename _T> struct Vector3 : public BaseVector<_T,3>
{
protected:
	typedef BaseVector<_T,3> Base;
	typedef typename Base::Base EigenBase;
public:
	typedef Vector3<_T> ThisType;

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

	// conversion

	inline operator Vector2<_T>() const
	{
		return Vector2<_T>(x(),y());
	}

	// constructors

	inline ThisType(const _T& _1,const _T& _2,const _T& _3) : Base(typename Base::Base(_1,_2,_3))
	{
	}

	template<class _T2> inline ThisType(const Vector2<_T2>& _v,const _T2& _3) : Base(typename Base::Base((_T)_v.x,(_T)_v.y,(_T)_3))
	{
	}
	
	template<class _T2> inline ThisType(const Vector3<_T2>& _v) : Base(typename Base::Base((_T)_v.x,(_T)_v.y,(_T)_v.z))
	{
	}

	template<class _T2> inline ThisType(const _T2& _1,const _T2& _2,const _T2& _3) : Base(typename Base::Base((_T)_1,(_T)_2,(_T)_3))
	{
	}

	inline ThisType()
	{
	}

	inline ThisType(const Base& _v) : Base(typename Base::Base(_v))
	{
	}

	// special

	inline ThisType operator%(const ThisType& other) const
	{
		return ThisType(Base(EigenBase::cross(other)));
	}
};



}

#endif
