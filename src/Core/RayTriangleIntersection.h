/********************************************************/
// FILE: RayTriangleIntersection.h
// DESCRIPTION: RayTriangleIntersection struct
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

#ifndef RAYTRACE_RAY_TRIANGLE_INTERSECTION_GUARD
#define RAYTRACE_RAY_TRIANGLE_INTERSECTION_GUARD

#include <RaytraceCommon.h>
#include "IntersectorBase.h"
#include "Ray.h"
#include "Triangle.h"
#include "SIMDType.h"
#include "MathHelper.h"

namespace Raytrace {
	
template<class _Options,class _Method,class _RayMode> struct RayTriangleIntersection;
struct RayTriangleIntersectionMethodHavel;
struct Tag_RayTriangleIntersectionMethod;


struct Tag_RayTriangleIntersectionMethod
{
	typedef RayTriangleIntersectionMethodHavel DefaultValue;
};

struct RayTriangleIntersectionMethodHavel 
{
	typedef Tag_RayTriangleIntersectionMethod optionsTag;
	static const bool optionsDefined = true;
};

namespace detail{

	template<class _Options> struct IntersectorResolver<_Options,PrimitiveClassTriangle>
	{
		typedef RayTriangleIntersection<_Options, 
			typename getOptionByTag<_Options,Tag_RayTriangleIntersectionMethod>::type,
			typename getOptionByTag<_Options,Tag_RayMode>::type> type;
	};

	
	template<class _Options,class _Method> struct RayTriangleIntersectionBase
	{
		typedef typename getOptionByTag<_Options,Tag_RayType>::type::RayType RayType;
		typedef typename getOptionByTag<_Options,Tag_PrimitiveType>::type::PrimitiveType TriangleType;
		typedef _Method Method;
	
		static const size_t RayCount = getOptionByTag<_Options,Tag_RayCount>::type::value;
		static const size_t TriangleCount = getOptionByTag<_Options,Tag_PrimitiveCount>::type::value;
		static const size_t ResultCount = RayCount*TriangleCount;
		
		typedef typename RayType::Scalar_T Ray_Scalar_T;
		typedef typename RayType::Vector_T Ray_Vector_T;
		typedef typename Ray_Scalar_T::Boolean Ray_Boolean;
		typedef typename Ray_Scalar_T::BooleanMask Ray_BooleanMask;

		typedef typename TriangleType::Scalar_T Triangle_Scalar_T;
		typedef typename TriangleType::Vector_T Triangle_Vector_T;
		typedef typename Triangle_Scalar_T::Boolean Triangle_Boolean;
		typedef typename Triangle_Scalar_T::BooleanMask Triangle_BooleanMask;
	
		typedef typename findCommonType<Ray_Scalar_T,Triangle_Scalar_T>::type Scalar_T;
		typedef typename findCommonType<Ray_Vector_T,Triangle_Vector_T>::type Vector_T;
		typedef typename findCommonType<Ray_Boolean,Triangle_Boolean>::type Boolean;
		typedef typename findCommonType<Ray_BooleanMask,Triangle_BooleanMask>::type BooleanMask;
		typedef Eigen::Matrix<Scalar_T,2,1>		Vector2_T;

		inline RayTriangleIntersectionBase()
		{
			static_assert(false,"Unknown TriangleIntersection Method");
		}
	};

	template<class _Options> struct RayTriangleIntersectionBase<_Options,RayTriangleIntersectionMethodHavel>
	{
		typedef typename getOptionByTag<_Options,Tag_RayType>::type::type RayType;
		typedef typename getOptionByTag<_Options,Tag_PrimitiveType>::type::type TriangleType;
		typedef RayTriangleIntersectionMethodHavel Method;
	
		static const size_t RayCount = getOptionByTag<_Options,Tag_RayCount>::type::value;
		static const size_t TriangleCount = getOptionByTag<_Options,Tag_PrimitiveCount>::type::value;
		static const size_t ResultCount = RayCount*TriangleCount;
	
		typedef typename RayType::Scalar_T Ray_Scalar_T;
		typedef typename RayType::Vector_T Ray_Vector_T;
		typedef typename Ray_Scalar_T::Boolean Ray_Boolean;
		typedef typename Ray_Scalar_T::BooleanMask Ray_BooleanMask;

		typedef typename TriangleType::Scalar_T Triangle_Scalar_T;
		typedef typename TriangleType::Vector_T Triangle_Vector_T;
		typedef typename Triangle_Scalar_T::Boolean Triangle_Boolean;
		typedef typename Triangle_Scalar_T::BooleanMask Triangle_BooleanMask;
	
		typedef typename findCommonType<Ray_Scalar_T,Triangle_Scalar_T>::type Scalar_T;
		typedef typename findCommonType<Ray_Vector_T,Triangle_Vector_T>::type Vector_T;
		typedef typename findCommonType<Ray_Boolean,Triangle_Boolean>::type Boolean;
		typedef typename findCommonType<Ray_BooleanMask,Triangle_BooleanMask>::type BooleanMask;
		typedef Eigen::Matrix<Scalar_T,2,1>		Vector2_T;

		inline RayTriangleIntersectionBase()
		{
			static_assert(!std::is_same<Scalar_T,void>::value,"Ray and Triangle type not compatible");
			static_assert(!std::is_same<Vector_T,void>::value,"Ray and Triangle type not compatible");
			static_assert(!std::is_same<Boolean,void>::value,"Ray and Triangle type not compatible");
			static_assert(!std::is_same<BooleanMask,void>::value,"Ray and Triangle type not compatible");
		}
	
		template<class _RawRayArray,class _RawTriangleArray,class _RawTArray,class _RawUVArray,class _RawValidArray>
		inline void operator()	(
				const _RawRayArray& rawRayArray,
				const _RawTriangleArray& rawTriangleArray,
				_RawTArray& rawTArray,
				_RawUVArray& rawUVArray,
				_RawValidArray& rawValidArray
			) const
		{
			ConstArrayWrapper<_RawRayArray,RayType,RayCount> rays(rawRayArray);
			ConstArrayWrapper<_RawTriangleArray,TriangleType,TriangleCount> triangles(rawTriangleArray);
			ArrayWrapper<_RawTArray,Scalar_T,ResultCount> t_out(rawTArray);
			ArrayWrapper<_RawUVArray,Vector2_T,ResultCount> uv_out(rawUVArray);
			ArrayWrapper<_RawValidArray,Boolean,ResultCount> valid(rawValidArray);

	#define FOREACH for(size_t i = 0; i< RayCount; ++i)for(size_t j = 0; j< TriangleCount; ++j)
	#define INDEX (i*TriangleCount + j)
		
			std::array<Scalar_T,ResultCount> inv_det;
			std::array<Vector_T,ResultCount> P_;

			//calculate ray plane intersection
			FOREACH inv_det[INDEX] = rays[INDEX].direction().dot( triangles[INDEX]._n_v ).ReciprocalHighPrecision();
			FOREACH t_out[INDEX] = -triangles[INDEX]._n_d - (rays[INDEX].origin().dot( triangles[INDEX]._n_v) );
			FOREACH t_out[INDEX] *= inv_det[INDEX];
			FOREACH P_[INDEX] = rays[INDEX].origin() + t_out[INDEX] * rays[INDEX].direction();

			// calculate impact values
			FOREACH uv_out[INDEX].x() = P_[INDEX].dot( triangles[INDEX]._u_v ) + triangles[INDEX]._u_d;
			FOREACH uv_out[INDEX].y() = P_[INDEX].dot( triangles[INDEX]._v_v ) + triangles[INDEX]._v_d;

			// check if we found any results

			//Boolean d_valid = (det > Scalar_T::Zero());
			FOREACH valid[INDEX] = ( (uv_out[INDEX].x() + uv_out[INDEX].y()) <= Scalar_T::One() ) & ( uv_out[INDEX].x() >= Scalar_T::Zero() ) & ( uv_out[INDEX].y() >= Scalar_T::Zero() );

	#undef FOREACH
	#undef INDEX
		}
	};
}

template<class _Options,class _Method,class _RayMode> struct RayTriangleIntersection :
	public detail::RayTriangleIntersectionBase<_Options,_Method>
{
	// return closest hit by id with intersection
	
	typedef detail::RayTriangleIntersectionBase<_Options,_Method> Base;

	template<class _RawRayArray,class _RawTriangleArray,class _RawTArray,class _RawUVArray,class _RawIdArray>
	inline bool operator()	(
		const _RawRayArray& rawRays,
		const _RawTriangleArray& rawTriangles,
		_RawTArray& rawT,
		_RawUVArray& rawUV,
		_RawIdArray& rawId
		) const
	{
		// check for hit
		
		std::array<Vector2_T,ResultCount> uvFound;
		std::array<Scalar_T,ResultCount> tFound;
		std::array<Boolean,ResultCount> valid;

		Base::operator()(rawRays,rawTriangles,tFound,uvFound,valid);
		
		ConstArrayWrapper<_RawTriangleArray,TriangleType,TriangleCount> triangles(rawTriangles);
		ArrayWrapper<_RawTArray,Scalar_T,RayCount> t(rawT);
		ArrayWrapper<_RawUVArray,Vector2_T,RayCount> uv(rawUV);
		ArrayWrapper<_RawIdArray,typename TriangleType::IndexType,RayCount> ids(rawId);

		bool result = false;
		
		for(int i = 0; i < RayCount; ++i)
			for(int j = 0; j < TriangleCount; ++j)
			{
				size_t index = i*TriangleCount + j; 
				valid[index] &= (tFound[index] >= Scalar_T::Epsilon()) & (tFound[index] < t[i]);

				if(valid[index].mask())
				{
					t[i].ConditionalAssign(valid[index],tFound[index], t[i]);
					t[i] = t[i].Min();
					Scalar_T::Boolean resultMask = (t[i] == tFound[index]);
					ids[i] = triangles[j].index() & resultMask;
					uv[i].x() = uvFound[index].x() & resultMask;
					uv[i].y() = uvFound[index].y() & resultMask;
					result = true;
				}
			}

		return result;
	}
};

template<class _Options,class _Method> struct RayTriangleIntersection<_Options,_Method,RayModeConstant> :
	public detail::RayTriangleIntersectionBase<_Options,_Method>
{
	// return all hits

	typedef detail::RayTriangleIntersectionBase<_Options,_Method> Base;
	
	template<class RawRayArray,class RawTriangleArray,class RawTArray,class RawResultArray>
	inline void operator()	(
		const RawRayArray& rawRays,
		const RawTriangleArray& rawTriangles,
		const RawTArray& rawT,
		RawResultArray& rawResult
		) const
	{
		// check for hit
		
		std::array<Scalar_T,ResultCount> tFound
		std::array<Vector2_T,ResultCount> uvFound
		std::array<Boolean,ResultCount> valid;

		Base::operator()(rawRays,rawTriangles,tFound,uvFound,valid);
		
		ConstArrayWrapper<RawTArray,Scalar_T,RayCount> t(rawT);
		ArrayWrapper<RawResultArray,BooleanMask,ResultCount> result(rawResult);

		for(int i = 0; i < ResultCount; ++i)
		{
			valid[i]  &= (tFound[i] < t[i/TriangleCount] );
			valid[i]  &= (tFound[i] > Scalar_T::Epsilon());
			result[i] = valid[i].mask();
		}
	}
	
	// return any hit
	
	template<class RawRayArray,class RawTriangleArray,class RawTArray>
	inline bool operator()	(
		const RawRayArray& rawRays,
		const RawTriangleArray& rawTriangles,
		const RawTArray& rawT
		) const
	{
		// check for hit
		
		std::array<Scalar_T,ResultCount> tFound;
		std::array<Vector2_T,ResultCount> uvFound;
		std::array<Boolean,ResultCount> valid;

		Base::operator()(rawRays,rawTriangles,tFound,uvFound,valid);
		
		ConstArrayWrapper<RawTArray,Scalar_T,RayCount> t(rawT);

		for(int i = 0; i < ResultCount; ++i)
		{
			valid[i]  &= (tFound[i] < t[i/TriangleCount] );
			valid[i]  &= (tFound[i] > Scalar_T::Epsilon());
			if(valid[i].mask())
				return true;
		}
		return false;
	}
};

}

#endif