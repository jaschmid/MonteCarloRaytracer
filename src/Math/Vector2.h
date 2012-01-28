/********************************************************/
/* FILE: Vector2.h                                      */
/* DESCRIPTION: Raytrace Vector2 class                      */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include "MathTypes.h"
#include "VectorBase.h"

#ifndef Raytrace_MATH_VECTOR2_H_INCLUDED
#define Raytrace_MATH_VECTOR2_H_INCLUDED

namespace Math {


template<typename _T> struct Vector2 : public BaseVector<_T,2>
{
protected:
	typedef BaseVector<_T,2> Base;
public:
	typedef Vector2<_T> ThisType;

	// access operations

	inline _T& x()
	{
		return Base::operator[](0);
	}
	inline _T& y()
	{
		return Base::operator[](1);
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

	// constructors

	inline ThisType(const _T& _1,const _T& _2) : Base(typename Base::Base(_1,_2))
	{
	}

	template<class _T2> inline ThisType(const Vector2<_T2>& _v) : Base(typename Base::Base((_T)_v.x,(_T)_v.y))
	{
	}

	template<class _T2> inline ThisType(const _T2& _1,const _T2& _2) : Base(typename Base::Base((_T)_1,(_T)_2))
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
