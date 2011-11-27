/********************************************************/
// FILE: SSEType.h
// DESCRIPTION: SSE Type for making Eigen and other values
// AUTHOR: Jan Schmid (jaschmid@eml.cc)    
/********************************************************/
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc/3.0/ or send 
// a letter to Creative Commons, 444 Castro Street, 
// Suite 900, Mountain View, California, 94041, USA.
/********************************************************/


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef RAYTRACE_SIMD_TYPE_GUARD
#define RAYTRACE_SIMD_TYPE_GUARD

#include <emmintrin.h>
#include <array>
#include <bitset>
#include <Eigen/Eigen>

namespace Raytrace
{
	template<class _Base,int _Width> struct SimdType
	{
		SimdType()
		{
			static_assert(false,"Unsupported Simd Type");
		}
	};

	// non simd passthrough
	template<class _Base> struct SimdType<_Base,1>
	{
		static const int Width = 1;
		typedef _Base				Base;
		static const int Size = Width;
		typedef Base Element;
		typedef SimdType<_Base,1>	ThisType;
		typedef bool				Boolean;
		typedef bool				BooleanMask;

		inline SimdType() {}
		inline SimdType(const ThisType& right) : _value(right._value) {}
		inline SimdType(const std::array<Base,Width>& right) : _value(right[0]) {}
		inline SimdType(const Base& right) : _value(right) {}
	public:
		
		inline Base& operator [](int i)
		{
			return ((Base*)&_value)[i];
		}
		
		inline const Base& operator [](int i) const
		{
			return ((const Base*)&_value)[i];
		}

		inline ~SimdType() {}

		inline ThisType& operator =(const ThisType& other)
		{
			_value = other._value;
			return *this;
		}
		
		inline ThisType& operator =(const Base& other)
		{
			_value = other;
			return *this;
		}

		inline ThisType operator *(const ThisType& other) const
		{
			return other._value * _value;
		}

		inline ThisType operator /(const ThisType& other) const
		{
			return other._value / _value;
		}
		
		inline ThisType operator +(const ThisType& other) const
		{
			return other._value + _value;
		}
		
		inline ThisType operator -(const ThisType& other) const
		{
			return other._value - _value;
		}
		
		inline ThisType& operator *=(const ThisType& other)
		{
			_value *= other._value;
			return *this;
		}

		inline ThisType& operator /=(const ThisType& other)
		{
			_value /= other._value;
			return *this;
		}
		
		inline ThisType& operator +=(const ThisType& other)
		{
			_value += other._value;
			return *this;
		}
		
		inline ThisType& operator -=(const ThisType& other)
		{
			_value -= other._value;
			return *this;
		}

		inline ThisType operator -() const
		{
			return -_value;
		}
		
		inline static ThisType One()
		{
			return ThisType((Base)1.0);
		}

		inline static ThisType Zero()
		{
			return ThisType((Base)0.0);
		}
		
		inline ThisType Absolute() const
		{
			return fabs(_value);
		}
		
		inline ThisType Sqrt() const
		{
			return sqrt(_value);
		}
		
		inline ThisType Exp() const
		{
			return exp(_value);
		}
		
		inline ThisType Log() const
		{
			return log(_value);
		}

		inline ThisType Sin() const
		{
			return sin(_value);
		}

		inline ThisType Cos() const
		{
			return cos(_value);
		}
		
		inline ThisType Reciprocal() const
		{
			return One() / *this;
		}

		inline ThisType Pow(const ThisType& right) const
		{
			return pow(_value,right._value);
		}

		inline Boolean operator ==(const ThisType& right) const
		{
			return _value == right._value;
		}
		
		inline Boolean operator !=(const ThisType& right) const
		{
			return _value != right._value;
		}
		
		inline Boolean operator <=(const ThisType& right) const
		{
			return _value <= right._value;
		}
		
		inline Boolean operator <(const ThisType& right) const
		{
			return _value < right._value;
		}
		
		inline Boolean operator >=(const ThisType& right) const
		{
			return _value >= right._value;
		}
		
		inline Boolean operator >(const ThisType& right) const
		{
			return _value > right._value;
		}

		inline static ThisType Condition(const Boolean& condition,const ThisType& trueVal,const ThisType& falseVal)
		{
			return condition ? trueVal : falseVal;
		}

		inline void ConditionalAssign(const Boolean& condition,const ThisType& trueVal,const ThisType& falseVal)
		{
			*this = Condition(condition,trueVal,falseVal);
		}

	private:
		Base _value;
	};
	
	
	template<> struct SimdType<int,4>
	{
	public:
		static const int Width = 4;
		typedef int Base;
		static const int Size = Width;
		typedef Base Element;
		typedef SimdType<Base,Width> ThisType;
		typedef ThisType Boolean;
		typedef int		 BooleanMask;

		inline SimdType() {}
		inline SimdType(const Base& base) : _value(_mm_set1_epi32(base)) {}
		inline SimdType(const ThisType& boolean) : _value(boolean._value) {}
		inline SimdType(const std::array<Base,Width>& right) : _value(_mm_loadu_si128((const __m128i*)right.data())) {}
		/*
		inline SimdType(const std::bitset<4>& right)
		{
			static const int one = 0xffffffff;
			static const int zero = 0x00000000;
			union
			{
				int							values[4];
				__m128i						values_i;
			};
			values[0] = right[0] ? one : zero;
			values[1] = right[1] ? one : zero;
			values[2] = right[2] ? one : zero;
			values[3] = right[3] ? one : zero;
			_value=_mm_load_si128(&values_i);
		}
		operator std::bitset<4> () const
		{
			std::bitset<4> result;
			union
			{
				int							values[4];
				__m128i						values_i;
			};
			_mm_store_si128(&values_i,_value);
			result[0] = (bool)(*(int*)&values[0] != 0);
			result[1] = (bool)(*(int*)&values[1] != 0);
			result[2] = (bool)(*(int*)&values[2] != 0);
			result[3] = (bool)(*(int*)&values[3] != 0);
			return result;
		}*/
		
	private:
		inline SimdType(const __m128& right) : _value(_mm_castps_si128(right)) {}
		inline SimdType(const __m128i& right) : _value(right) {}
	public:

		inline int mask() const
		{
			return _mm_movemask_ps( _mm_castsi128_ps( _value) );
		}

		inline Base& operator [](int i)
		{
			return ((Base*)&_value)[i];
		}
		
		inline const Base& operator [](int i) const
		{
			return ((const Base*)&_value)[i];
		}

		inline ThisType& operator =(const Base& right)
		{
			_value = _mm_set1_epi32(right);
			return *this;
		}

		inline ThisType& operator =(const ThisType& other)
		{
			_value = other._value;
			return *this;
		}

		inline ThisType operator&(const ThisType& other) const
		{
			return _mm_and_si128( _value, other._value);
		}
		
		inline ThisType operator|(const ThisType& other) const
		{
			return _mm_or_si128( _value, other._value);
		}
		
		inline ThisType operator^(const ThisType& other) const
		{
			return _mm_xor_si128( _value, other._value);
		}

		inline ThisType AndNot(const ThisType& other) const
		{
			return _mm_andnot_si128(other._value,_value);
		}

		inline ThisType& operator&=(const ThisType& other)
		{
			_value = _mm_and_si128( _value, other._value);
			return *this;
		}
		
		inline ThisType& operator|=(const ThisType& other)
		{
			_value = _mm_or_si128( _value, other._value);
			return *this;
		}
		
		inline ThisType& operator^=(const ThisType& other)
		{
			_value = _mm_xor_si128( _value, other._value);
			return *this;
		}
		
		inline Boolean operator ==(const ThisType& right) const
		{
			return _mm_cmpeq_epi32(_value,right._value);
		}
		
		inline Boolean operator <(const ThisType& right) const
		{
			return _mm_cmplt_epi32(_value,right._value);
		}
		
		inline Boolean operator >(const ThisType& right) const
		{
			return _mm_cmpgt_epi32(_value,right._value);
		}

		inline static ThisType One()
		{
			return ThisType((Base)1);
		}

		inline static ThisType Zero()
		{
			return ThisType((Base)0);
		}
		
		inline static ThisType Condition(const Boolean& condition,const ThisType& trueVal,const ThisType& falseVal)
		{
			return _mm_or_si128(_mm_and_si128(condition._value,trueVal._value),_mm_andnot_si128(condition._value,falseVal._value));
		}

		inline void ConditionalAssign(const Boolean& condition,const ThisType& trueVal,const ThisType& falseVal)
		{
			*this = Condition(condition,trueVal,falseVal);
		}

	private:
		friend struct SimdType<float,4>;
		__m128i	_value;
	};

	template<> struct SimdType<float,4>
	{
		static const int Width = 4;
		typedef float Base;
		static const int Size = Width;
		typedef Base Element;
		typedef SimdType<Base,Width> ThisType;
		typedef SimdType<int,Width> Boolean;
		typedef int					BooleanMask;

		inline SimdType() {}
		inline SimdType(const ThisType& right) : _value(right._value) {}
		inline SimdType(const std::array<Base,Width>& right) : _value(_mm_loadu_ps((Base*)right.data())) {}
		inline SimdType(const Base& right)
		{
			_value = _mm_set1_ps(right);
		}
	private:
		inline SimdType(const __m128& right) : _value(right) {}
	public:

		inline Base& operator [](int i)
		{
			return ((Base*)&_value)[i];
		}
		
		inline const Base& operator [](int i) const
		{
			return ((const Base*)&_value)[i];
		}
		
		inline ThisType& operator =(const Base& right)
		{
			_value = _mm_set1_ps(right);
			return *this;
		}

		inline ThisType& operator =(const ThisType& other)
		{
			_value = other._value;
			return *this;
		}

		inline ThisType operator *(const ThisType& other) const
		{
			return _mm_mul_ps(_value,other._value);
		}

		inline ThisType operator /(const ThisType& other) const
		{
			return _mm_div_ps(_value,other._value);
		}
		
		inline ThisType operator +(const ThisType& other) const
		{
			return _mm_add_ps(_value,other._value);
		}
		
		inline ThisType operator -(const ThisType& other) const
		{
			return _mm_sub_ps(_value,other._value);
		}
		
		inline ThisType& operator *=(const ThisType& other)
		{
			_value = _mm_mul_ps(_value,other._value);
			return *this;
		}

		inline ThisType& operator /=(const ThisType& other)
		{
			_value = _mm_div_ps(_value,other._value);
			return *this;
		}
		
		inline ThisType& operator +=(const ThisType& other)
		{
			_value = _mm_add_ps(_value,other._value);
			return *this;
		}
		
		inline ThisType& operator -=(const ThisType& other)
		{
			_value = _mm_sub_ps(_value,other._value);
			return *this;
		}

		inline ThisType operator -() const
		{
			static const unsigned int sign_mask = 0x80000000;
			return _mm_xor_ps(_value,_mm_set1_ps(*(float *)&sign_mask));
		}

		inline ThisType operator&(const Boolean& other) const
		{
			return _mm_and_ps( _value, _mm_castsi128_ps(other._value));
		}
		
		inline ThisType operator|(const Boolean& other) const
		{
			return _mm_or_ps( _value, _mm_castsi128_ps(other._value));
		}
		
		inline ThisType operator^(const Boolean& other) const
		{
			return _mm_xor_ps( _value, _mm_castsi128_ps(other._value));
		}

		inline ThisType AndNot(const Boolean& other) const
		{
			return _mm_andnot_ps(_mm_castsi128_ps(other._value),_value);
		}

		inline ThisType& operator&=(const Boolean& other)
		{
			_value = _mm_and_ps( _value, _mm_castsi128_ps(other._value));
			return *this;
		}
		
		inline ThisType& operator|=(const Boolean& other)
		{
			_value = _mm_or_ps( _value, _mm_castsi128_ps(other._value));
			return *this;
		}
		
		inline ThisType& operator^=(const Boolean& other)
		{
			_value = _mm_xor_ps( _value, _mm_castsi128_ps(other._value));
			return *this;
		}

		inline static ThisType Epsilon()
		{
			return _mm_set1_ps(.00001f);
		}
		
		inline static ThisType One()
		{
			return _mm_set1_ps(1.0f);
		}

		inline static ThisType Zero()
		{
			return _mm_setzero_ps();
		}

		inline ThisType Absolute() const
		{
			static const unsigned int clear_sign_mask = 0x7fffffff;
			return _mm_and_ps(_value,_mm_set1_ps(*(float*)(&clear_sign_mask)));
		}
		
		inline ThisType Sqrt() const
		{
			return _mm_sqrt_ps(_value);
		}
		
		inline ThisType Reciprocal() const
		{
			return _mm_rcp_ps(_value);
		}
		
		inline ThisType ReciprocalHighPrecision() const
		{
			__m128 low = _mm_rcp_ps(_value);
			__m128 high = _mm_mul_ps(_mm_mul_ps(_value,low),low);
			return _mm_sub_ps(_mm_add_ps(low,low),high);
		}

		inline ThisType Exp() const
		{
			// TO DO
			assert(false);
			return Zero();
		}
		
		inline ThisType Log() const
		{
			// TO DO
			assert(false);
			return Zero();
		}

		inline ThisType Sin() const
		{
			// TO DO
			assert(false);
			return Zero();
		}

		inline ThisType Cos() const
		{
			// TO DO
			assert(false);
			return Zero();
		}

		inline ThisType Pow(const ThisType& exponent) const
		{
			// TO DO
			assert(false);
			return Zero();
		}

		inline ThisType Min(const ThisType& right) const
		{
			return _mm_min_ps(_value,right._value);
		}
		
		inline ThisType Max(const ThisType& right) const
		{
			return _mm_max_ps(_value,right._value);
		}
		
		inline ThisType Min() const
		{
			__m128 intermediate = _mm_min_ps(_value,_mm_shuffle_ps(_value,_value, _MM_SHUFFLE(2,3,0,1)));
			__m128 result = _mm_min_ps(intermediate,_mm_shuffle_ps(intermediate,intermediate, _MM_SHUFFLE(0,0,2,2)));
			return result;
		}
		
		inline ThisType Max() const
		{
			__m128 intermediate = _mm_max_ps(_value,_mm_shuffle_ps(_value,_value, _MM_SHUFFLE(2,3,0,1)));
			__m128 result = _mm_max_ps(intermediate,_mm_shuffle_ps(intermediate,intermediate, _MM_SHUFFLE(0,0,2,2)));
			return result;
		}

		inline Boolean operator ==(const ThisType& right) const
		{
			return _mm_cmpeq_ps(_value,right._value);
		}
		
		inline Boolean operator !=(const ThisType& right) const
		{
			return _mm_cmpneq_ps(_value,right._value);
		}
		
		inline Boolean operator <=(const ThisType& right) const
		{
			return _mm_cmple_ps(_value,right._value);
		}
		
		inline Boolean operator <(const ThisType& right) const
		{
			return _mm_cmplt_ps(_value,right._value);
		}
		
		inline Boolean operator >=(const ThisType& right) const
		{
			return _mm_cmpge_ps(_value,right._value);
		}
		
		inline Boolean operator >(const ThisType& right) const
		{
			return _mm_cmpgt_ps(_value,right._value);
		}

		inline static void ConditionalSwap(const Boolean& condition,ThisType& valA,ThisType& valB)
		{
			valA._value = _mm_xor_ps(valA._value,valB._value);
			__m128 temp = _mm_and_ps(_mm_castsi128_ps(condition._value),valA._value);
			valB._value = _mm_xor_ps(valB._value,temp);
			valA._value = _mm_xor_ps(valA._value,valB._value);
		}

		inline static ThisType Condition(const Boolean& condition,const ThisType& trueVal,const ThisType& falseVal)
		{
			return _mm_or_ps(_mm_and_ps(_mm_castsi128_ps(condition._value),trueVal._value),_mm_andnot_ps(_mm_castsi128_ps(condition._value),falseVal._value));
		}

		inline void ConditionalAssign(const Boolean& condition,const ThisType& trueVal,const ThisType& falseVal)
		{
			*this = Condition(condition,trueVal,falseVal);
		}
	private:
		__m128	_value;
	};

	//mappings for eigen
	namespace internal
	{
		template<class _SIMD> inline const _SIMD& conj(const _SIMD& x)  { return x; }
		template<class _SIMD> inline const _SIMD& real(const _SIMD& x)  { return x; }
		template<class _SIMD> inline _SIMD imag(const _SIMD&)    { return _SIMD::Zero(); }
		template<class _SIMD> inline _SIMD abs(const _SIMD&  x)  { return x.Absolute(); }
		template<class _SIMD> inline _SIMD abs2(const _SIMD& x)  { return x*x; }
		template<class _SIMD> inline _SIMD sqrt(const _SIMD& x)  { return x.Sqrt(); }
		template<class _SIMD> inline _SIMD exp(const _SIMD&  x)  { return x.Exp(); }
		template<class _SIMD> inline _SIMD log(const _SIMD&  x)  { return x.Log(); }
		template<class _SIMD> inline _SIMD sin(const _SIMD&  x)  { return x.Sin(); }
		template<class _SIMD> inline _SIMD cos(const _SIMD&  x)  { return x.Cos(); }
		template<class _SIMD> inline _SIMD pow(const _SIMD& x, const _SIMD& y)  { return x.Pow(y); }
	}
}

namespace Eigen {

template<class _Base,int _Width> struct NumTraits<::Raytrace::SimdType<_Base,_Width>>
{
  typedef typename NumTraits<_Base>::Real Real;
  typedef ::Raytrace::SimdType<typename NumTraits<_Base>::NonInteger,_Width> NonInteger;
  typedef ::Raytrace::SimdType<typename NumTraits<_Base>::Nested,_Width> Nested;

  enum {
    IsComplex = false,
    IsInteger = false,
    IsSigned = true,
    ReadCost = NumTraits<_Base>::ReadCost,
    AddCost = NumTraits<_Base>::AddCost,
    MulCost = NumTraits<_Base>::MulCost
  };

  static Real epsilon()
  {
	  return NumTraits<Real>::epsilon();
  }
  static Real dummy_precision()
  {
	  return NumTraits<Real>::dummy_precision();
  }
  static Real highest()
  {
	  return NumTraits<Real>::dummy_precision();
  }
  static Real lowest()
  {
	  return NumTraits<Real>::dummy_precision();
  }
};

}

namespace Raytrace
{
	template<int _Width> struct Vector2v
	{
		typedef Eigen::Matrix<SimdType<float,_Width>,2,1>			type;
	};
	template<int _Width> struct Vector3v
	{
		typedef Eigen::Matrix<SimdType<float,_Width>,3,1>			type;
	};
	template<int _Width> struct Vector4v
	{
		typedef Eigen::Matrix<SimdType<float,_Width>,4,1>			type;
	};

}



#endif