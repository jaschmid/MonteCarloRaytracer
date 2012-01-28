/********************************************************/
/* FILE: VectorBase.h                                      */
/* DESCRIPTION: Raytrace Base class                      */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include "MathTypes.h"

#ifndef Raytrace_MATH_BASEVECTOR_H_INCLUDED
#define Raytrace_MATH_BASEVECTOR_H_INCLUDED

namespace Math {
	
template<class _T, int _Columns,int _Rows> struct Matrix;

namespace detail
{

	template<class _T, int _Columns,int _Rows> struct MatrixBase
	{
		typedef Matrix<_T,_Columns,_Rows>					ThisType;
		typedef MatrixBase<_T,_Columns,_Rows>				BaseType;
		static const int									Rows = _Rows;
		static const int									Columns = _Columns;
		typedef _T											T;
		typedef _T											Scale;
		typedef ThisType									Offset;
		typedef Matrix<_T,_Rows,_Columns>					Multiply;

		const T& operator()(int column,int row) const
		{
			return _data[row * Columns + column];
		}

		T& operator()(int column,int row)
		{
			return _data[row * Columns + column];
		}

	private:
		std::array<T,Rows*Columns>			_data;
	};

	// functions that appear in matrices of size _Rows/_Columns
	template<class _T,int _Columns,int _Rows,class _Base> struct MatrixExtraExact : public _Base
	{
	};
	
	// functions that appear in matrices of any size greater than or eequal _Rows/_Columns
	template<class _T,int _Columns,int _Rows,class _Base> struct MatrixExtraPlus : public _Base
	{
	};

	// retrieve the next matrix stack
	template<class _T,int _Columns,int _Rows,class _Base> struct NextMatrixStack
	{
		typedef MatrixExtraPlus<_T,_Rows,_Columns,MatrixStack<_T,_Columns,_Rows-1,_Base>> type;
	};
	
	template<class _T,int _Rows,class _Base> struct NextMatrixStack<_T,0,_Rows,_Base>
	{
		typedef MatrixStack<_T,_Base::_Columns,_Rows-1,_Base> type;
	};

	template<class _T,int _Columns,class _Base> struct NextMatrixStack<_T,_Columns,0,_Base>
	{
		typedef _Base type;
	};

	// actual implementation
	template<class _T,int _Rows,int _Columns> struct MatrixStack : 
		public MatrixExtraPlus<_T,_Rows,_Columns, typename NextMatrixStack<_T,_Rows,_Columns, MatrixBase<_T,_Rows,_Columns> >::type >
	{
	};
}

template<class _T,int _Rows,int _Columns> struct Matrix :
	public detail::conditional_inherit< (_Dimension == 2), detail::Vector2<_T> >,
	public detail::conditional_inherit< (_Dimension >= 2), detail::Vector3Plus<_T, _Dimension> >,
	public detail::conditional_inherit< (_Dimension == 3), detail::Vector3<_T> >,
	public detail::conditional_inherit< (_Dimension >= 3), detail::Vector3Plus<_T, _Dimension> >,
	public detail::conditional_inherit< (_Dimension == 4), detail::Vector4<_T> >,
	public detail::conditional_inherit< (_Dimension >= 4), detail::Vector4Plus<_T, _Dimension> >
{
public:

	inline BaseVector(const BaseVector& other) : Base(base) {}

	typedef BaseVector<_T,_Dimension>						ThisType;
	static const int										Dimension = _Dimension;
	typedef _T												T;
	typedef _T												Scale;
	typedef BaseVector<_T,_Dimension>						Offset;

	//constructors
	
	inline BaseVector(const ThisType& right) {_data = right._data}
	inline BaseVector(const _T& _1)  { static_assert(_Dimension == 1,"wrong number of construction arguments"); _data[0] = _1;}
	inline BaseVector(const _T& _1,const _T& _2)  { static_assert(_Dimension == 2,"wrong number of construction arguments"); _data[0] = _1; _data[1] = _2;}
	inline BaseVector(const _T& _1,const _T& _2,const _T& _3)  { static_assert(_Dimension == 3,"wrong number of construction arguments"); _data[0] = _1; _data[1] = _2; _data[2] = _3;}
	inline BaseVector(const _T& _1,const _T& _2,const _T& _3,const _T& _4)  { static_assert(_Dimension == 4,"wrong number of construction arguments"); _data[0] = _1; _data[1] = _2; _data[2] = _3; _data[3] = _4;}
	inline BaseVector(const _T* _a) : Base(_a) {}
	
	inline BaseVector() {}
	

	// access operations
	_T& operator[](const up& i)
	{
		return _data(i);
	}

	// const access functions
	
	const _T& operator[](const up& i) const
	{
		return _data(i);
	}

	//assignment
	inline ThisType& operator = (const ThisType& v) 
	{
		_data = v._data;
		return *(ThisType*)this;
	}

	// conversion

	template<int i> _T& swizzle()
	{
		static_assert(i >= 0 && i < Dimension,"swizzle operator 1 must be in range");
		return operator[](i);
	}
	template<int i> const _T& swizzle() const
	{
		static_assert(i >= 0 && i < Dimension,"swizzle operator 1 must be in range");
		return operator[](i);
	}
	template<int i,int j> const BaseVector<_T,2> swizzle() const
	{
		static_assert(i >= 0 && i < Dimension,"swizzle operator 1 must be in range");
		static_assert(j >= 0 && j < Dimension,"swizzle operator 2 must be in range");
		return BaseVector<_T,2>(operator[](i),operator[](j));
	}
	template<int i,int j,int k> const BaseVector<_T,3> swizzle() const
	{
		static_assert(i >= 0 && i < Dimension,"swizzle operator 1 must be in range");
		static_assert(j >= 0 && j < Dimension,"swizzle operator 2 must be in range");
		static_assert(k >= 0 && k < Dimension,"swizzle operator 3 must be in range");
		return BaseVector<_T,3>(operator[](i),operator[](j),operator[](k));
	}
	template<int i,int j,int k,int l> const BaseVector<_T,4> swizzle() const
	{
		static_assert(i >= 0 && i < Dimension,"swizzle operator 1 must be in range");
		static_assert(j >= 0 && j < Dimension,"swizzle operator 2 must be in range");
		static_assert(k >= 0 && k < Dimension,"swizzle operator 3 must be in range");
		static_assert(l >= 0 && l < Dimension,"swizzle operator 4 must be in range");
		return BaseVector<_T,4>(operator[](i),operator[](j),operator[](k),operator[](l));
	}

	//swizzle aliases
	
	// 1d
	inline const _T& x() const { return swizzle<0>() }
	inline const _T& y() const { return swizzle<1>() }
	inline const _T& z() const { return swizzle<2>() }
	inline const _T& w() const { return swizzle<3>() }

	inline _T& x() { return swizzle<0>() }
	inline _T& y() { return swizzle<1>() }
	inline _T& z() { return swizzle<2>() }
	inline _T& w() { return swizzle<3>() }

	// 2d

	inline const BaseVector<_T,2> xx() const { return swizzle<0,0>() }
	inline const BaseVector<_T,2> xy() const { return swizzle<0,1>() }
	inline const BaseVector<_T,2> xz() const { return swizzle<0,2>() }
	inline const BaseVector<_T,2> xw() const { return swizzle<0,3>() }

	inline const BaseVector<_T,2> yx() const { return swizzle<1,0>() }
	inline const BaseVector<_T,2> yy() const { return swizzle<1,1>() }
	inline const BaseVector<_T,2> yz() const { return swizzle<1,2>() }
	inline const BaseVector<_T,2> yw() const { return swizzle<1,3>() }

	inline const BaseVector<_T,2> zx() const { return swizzle<2,0>() }
	inline const BaseVector<_T,2> zy() const { return swizzle<2,1>() }
	inline const BaseVector<_T,2> zz() const { return swizzle<2,2>() }
	inline const BaseVector<_T,2> zw() const { return swizzle<2,3>() }

	inline const BaseVector<_T,2> wx() const { return swizzle<3,0>() }
	inline const BaseVector<_T,2> wy() const { return swizzle<3,1>() }
	inline const BaseVector<_T,2> wz() const { return swizzle<3,2>() }
	inline const BaseVector<_T,2> ww() const { return swizzle<3,3>() }

	// 3d

	inline const BaseVector<_T,3> xxx() const { return swizzle<0,0,0>() }
	inline const BaseVector<_T,3> xxy() const { return swizzle<0,0,1>() }
	inline const BaseVector<_T,3> xxz() const { return swizzle<0,0,2>() }
	inline const BaseVector<_T,3> xxw() const { return swizzle<0,0,3>() }
	inline const BaseVector<_T,3> xyx() const { return swizzle<0,1,0>() }
	inline const BaseVector<_T,3> xyy() const { return swizzle<0,1,1>() }
	inline const BaseVector<_T,3> xyz() const { return swizzle<0,1,2>() }
	inline const BaseVector<_T,3> xyw() const { return swizzle<0,1,3>() }
	inline const BaseVector<_T,3> xzx() const { return swizzle<0,2,0>() }
	inline const BaseVector<_T,3> xzy() const { return swizzle<0,2,1>() }
	inline const BaseVector<_T,3> xzz() const { return swizzle<0,2,2>() }
	inline const BaseVector<_T,3> xzw() const { return swizzle<0,2,3>() }
	inline const BaseVector<_T,3> xwx() const { return swizzle<0,3,0>() }
	inline const BaseVector<_T,3> xwy() const { return swizzle<0,3,1>() }
	inline const BaseVector<_T,3> xwz() const { return swizzle<0,3,2>() }
	inline const BaseVector<_T,3> xww() const { return swizzle<0,3,3>() }
	
	inline const BaseVector<_T,3> yxx() const { return swizzle<1,0,0>() }
	inline const BaseVector<_T,3> yxy() const { return swizzle<1,0,1>() }
	inline const BaseVector<_T,3> yxz() const { return swizzle<1,0,2>() }
	inline const BaseVector<_T,3> yxw() const { return swizzle<1,0,3>() }
	inline const BaseVector<_T,3> yyx() const { return swizzle<1,1,0>() }
	inline const BaseVector<_T,3> yyy() const { return swizzle<1,1,1>() }
	inline const BaseVector<_T,3> yyz() const { return swizzle<1,1,2>() }
	inline const BaseVector<_T,3> yyw() const { return swizzle<1,1,3>() }
	inline const BaseVector<_T,3> yzx() const { return swizzle<1,2,0>() }
	inline const BaseVector<_T,3> yzy() const { return swizzle<1,2,1>() }
	inline const BaseVector<_T,3> yzz() const { return swizzle<1,2,2>() }
	inline const BaseVector<_T,3> yzw() const { return swizzle<1,2,3>() }
	inline const BaseVector<_T,3> ywx() const { return swizzle<1,3,0>() }
	inline const BaseVector<_T,3> ywy() const { return swizzle<1,3,1>() }
	inline const BaseVector<_T,3> ywz() const { return swizzle<1,3,2>() }
	inline const BaseVector<_T,3> yww() const { return swizzle<1,3,3>() }
	
	inline const BaseVector<_T,3> zxx() const { return swizzle<2,0,0>() }
	inline const BaseVector<_T,3> zxy() const { return swizzle<2,0,1>() }
	inline const BaseVector<_T,3> zxz() const { return swizzle<2,0,2>() }
	inline const BaseVector<_T,3> zxw() const { return swizzle<2,0,3>() }
	inline const BaseVector<_T,3> zyx() const { return swizzle<2,1,0>() }
	inline const BaseVector<_T,3> zyy() const { return swizzle<2,1,1>() }
	inline const BaseVector<_T,3> zyz() const { return swizzle<2,1,2>() }
	inline const BaseVector<_T,3> zyw() const { return swizzle<2,1,3>() }
	inline const BaseVector<_T,3> zzx() const { return swizzle<2,2,0>() }
	inline const BaseVector<_T,3> zzy() const { return swizzle<2,2,1>() }
	inline const BaseVector<_T,3> zzz() const { return swizzle<2,2,2>() }
	inline const BaseVector<_T,3> zzw() const { return swizzle<2,2,3>() }
	inline const BaseVector<_T,3> zwx() const { return swizzle<2,3,0>() }
	inline const BaseVector<_T,3> zwy() const { return swizzle<2,3,1>() }
	inline const BaseVector<_T,3> zwz() const { return swizzle<2,3,2>() }
	inline const BaseVector<_T,3> zww() const { return swizzle<2,3,3>() }
	
	inline const BaseVector<_T,3> wxx() const { return swizzle<3,0,0>() }
	inline const BaseVector<_T,3> wxy() const { return swizzle<3,0,1>() }
	inline const BaseVector<_T,3> wxz() const { return swizzle<3,0,2>() }
	inline const BaseVector<_T,3> wxw() const { return swizzle<3,0,3>() }
	inline const BaseVector<_T,3> wyx() const { return swizzle<3,1,0>() }
	inline const BaseVector<_T,3> wyy() const { return swizzle<3,1,1>() }
	inline const BaseVector<_T,3> wyz() const { return swizzle<3,1,2>() }
	inline const BaseVector<_T,3> wyw() const { return swizzle<3,1,3>() }
	inline const BaseVector<_T,3> wzx() const { return swizzle<3,2,0>() }
	inline const BaseVector<_T,3> wzy() const { return swizzle<3,2,1>() }
	inline const BaseVector<_T,3> wzz() const { return swizzle<3,2,2>() }
	inline const BaseVector<_T,3> wzw() const { return swizzle<3,2,3>() }
	inline const BaseVector<_T,3> wwx() const { return swizzle<3,3,0>() }
	inline const BaseVector<_T,3> wwy() const { return swizzle<3,3,1>() }
	inline const BaseVector<_T,3> wwz() const { return swizzle<3,3,2>() }
	inline const BaseVector<_T,3> www() const { return swizzle<3,3,3>() }

	// math operations
	// scale

	inline ThisType operator *(const Scale& v) const
	{
		ThisType result;
		for(up i = 0; i < Dimension; ++i)
			result[i] = _data[i]*v;
		return result;
	}

	inline ThisType operator /(const Scale& v) const
	{
		ThisType result;
		for(up i = 0; i < Dimension; ++i)
			result[i] = _data[i]/v;
		return result;
	}

	// offset

	inline ThisType operator +(const Offset& v) const
	{
		ThisType result;
		for(up i = 0; i < Dimension; ++i)
			result[i] = _data[i]+v[i];
		return result;
	}

	inline ThisType operator -(const Offset& v) const
	{
		ThisType result;
		for(up i = 0; i < Dimension; ++i)
			result[i] = _data[i]-v[i];
		return result;
	}

	// sign
	inline ThisType operator-() const
	{
		ThisType result;
		for(up i = 0; i < Dimension; ++i)
			result[i] = -_data[i];
		return result;
	}

	//special
	//dot
	inline T dot(const ThisType& v) const
	{
		T result = Zero<T>();
		for(up i = 0; i < Dimension; ++i)
			result[i] += _data[i]*v[i];
		return result;
	}
	inline T operator *(const ThisType& v) const
	{
		return dot(v);
	}
	// squared length
	inline T operator!() const
	{
		return sqLength();
	}

	inline T sqLength() const
	{
		return dot(*this);
	}

	inline T length() const
	{
		return ::Math::sqrt(sqLength());
	}

	inline ThisType normalize() const
	{
		return *this / length();
	}

	std::array<_T,_Dimension>	_data;
};

}

#endif