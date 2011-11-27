/********************************************************/
// FILE: RayAABBIntersection.h
// DESCRIPTION: RayAABBIntersection struct
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

#ifndef RAYTRACE_RAY_AABB_INTERSECTION_GUARD
#define RAYTRACE_RAY_AABB_INTERSECTION_GUARD

#include <RaytraceCommon.h>
#include "IntersectorBase.h"
#include "Ray.h"
#include "AABB.h"
#include "SIMDType.h"
#include "MathHelper.h"

namespace Raytrace {

struct RayAABBIntersectionMethodWilliams;
struct Tag_RayAABBIntersectionMethod;

struct Tag_RayAABBIntersectionMethod
{
	typedef RayAABBIntersectionMethodWilliams DefaultValue;
};

struct RayAABBIntersectionMethodWilliams 
{
	typedef Tag_RayAABBIntersectionMethod optionsTag;
	static const bool optionsDefined = true;
};

template<class _Options,class _Method,class _RayMode> struct RayAABBIntersection;

namespace detail{
	

	template<class _Options> struct IntersectorResolver<_Options,PrimitiveClassAxisAlignedBox>
	{
		typedef RayAABBIntersection<_Options, 
			typename getOptionByTag<_Options,Tag_RayAABBIntersectionMethod>::type,
			typename getOptionByTag<_Options,Tag_RayMode>::type> type;
	};

	// _RayType,class _AABBType,int _RayCount,int _AABBCount
	template<class _Options,class _Method> struct RayAABBIntersectionBase
	{
		typedef typename getOptionByTag<_Options,Tag_RayType>::type::RayType RayType;
		typedef typename getOptionByTag<_Options,Tag_PrimitiveType>::type::PrimitiveType AABBType;
		typedef _Method Method;

		static const size_t RayCount = getOptionByTag<_Options,Tag_PrimitiveType>::type::value;
		static const size_t AABBCount = getOptionByTag<_Options,Tag_PrimitiveType>::type::value;
		static const size_t ResultCount = RayCount*AABBCount;
		
		typedef typename RayType::Scalar_T Ray_Scalar_T;
		typedef typename RayType::Vector_T Ray_Vector_T;
		typedef typename Ray_Scalar_T::Boolean Ray_Boolean;
		typedef typename Ray_Scalar_T::BooleanMask Ray_BooleanMask;

		typedef typename AABBType::Scalar_T AABB_Scalar_T;
		typedef typename AABBType::Vector_T AABB_Vector_T;
		typedef typename AABB_Scalar_T::Boolean AABB_Boolean;
		typedef typename AABB_Scalar_T::BooleanMask AABB_BooleanMask;
	
		typedef typename findCommonType<Ray_Scalar_T,AABB_Scalar_T>::type Scalar_T;
		typedef typename findCommonType<Ray_Vector_T,AABB_Vector_T>::type Vector_T;
		typedef typename findCommonType<Ray_Boolean,AABB_Boolean>::type Boolean;
		typedef typename findCommonType<Ray_BooleanMask,AABB_BooleanMask>::type BooleanMask;

		inline RayAABBIntersectionBase()
		{
			static_assert(false,"Unknown AABBIntersection Method");
		}
	};

	template<class _Options> struct RayAABBIntersectionBase<_Options,RayAABBIntersectionMethodWilliams>
	{
		typedef typename getOptionByTag<_Options,Tag_RayType>::type::type RayType;
		typedef typename getOptionByTag<_Options,Tag_PrimitiveType>::type::type AABBType;
		typedef RayAABBIntersectionMethodWilliams Method;
	
		static const size_t RayCount = getOptionByTag<_Options,Tag_RayCount>::type::value;
		static const size_t AABBCount = getOptionByTag<_Options,Tag_PrimitiveCount>::type::value;
		static const size_t ResultCount = RayCount*AABBCount;
	
		typedef typename RayType::Scalar_T Ray_Scalar_T;
		typedef typename RayType::Vector_T Ray_Vector_T;
		typedef typename Ray_Scalar_T::Boolean Ray_Boolean;
		typedef typename Ray_Scalar_T::BooleanMask Ray_BooleanMask;

		typedef typename AABBType::Scalar_T AABB_Scalar_T;
		typedef typename AABBType::Vector_T AABB_Vector_T;
		typedef typename AABB_Scalar_T::Boolean AABB_Boolean;
		typedef typename AABB_Scalar_T::BooleanMask AABB_BooleanMask;
	
		typedef typename findCommonType<Ray_Scalar_T,AABB_Scalar_T>::type Scalar_T;
		typedef typename findCommonType<Ray_Vector_T,AABB_Vector_T>::type Vector_T;
		typedef typename findCommonType<Ray_Boolean,AABB_Boolean>::type Boolean;
		typedef typename findCommonType<Ray_BooleanMask,AABB_BooleanMask>::type BooleanMask;
		/*
		template<int _RayCount,int _PrimitiveCount> struct test
		{
			static_assert(_RayCount >= 5,"pop");
		};*/

		inline RayAABBIntersectionBase()
		{
			//test<RayCount,AABBCount> pop;

			static_assert(!std::is_same<Scalar_T,void>::value,"Ray and AABB type not compatible");
			static_assert(!std::is_same<Vector_T,void>::value,"Ray and AABB type not compatible");
			static_assert(!std::is_same<Boolean,void>::value,"Ray and AABB type not compatible");
			static_assert(!std::is_same<BooleanMask,void>::value,"Ray and AABB type not compatible");
		}
	
		template<class _RawRayArray,class _RawAABBArray,class _RawTArrayMin,class _RawTArrayMax,class _RawValidArray>
		inline void operator()	(
			const _RawRayArray& rawRays,
			const _RawAABBArray& rawAABBs,
			_RawTArrayMin& rawTMins,
			_RawTArrayMax& rawTMaxs,
			_RawValidArray& rawValids
			) const
		{
			ConstArrayWrapper<_RawRayArray,RayType,RayCount> rays(rawRays);
			ConstArrayWrapper<_RawAABBArray,AABBType,AABBCount> aabbs(rawAABBs);
			ArrayWrapper<_RawTArrayMin,Scalar_T,ResultCount> tMin(rawTMins);
			ArrayWrapper<_RawTArrayMax,Scalar_T,ResultCount> tMax(rawTMaxs);
			ArrayWrapper<_RawValidArray,Boolean,ResultCount> valid(rawValids);

	#define FOREACH for(size_t i = 0; i< RayCount; ++i)for(size_t j = 0; j< AABBCount; ++j)
	#define INDEX (i*AABBCount + j)
			// X

			std::array<Scalar_T,ResultCount> tXmax,tXmin,tYmax,tYmin,tZmax,tZmin;

			FOREACH	tXmax[INDEX] = (aabbs[j].max().x() - rays[i].origin().x())* rays[i].invDirection().x();
		
			FOREACH	tXmin[INDEX] = (aabbs[j].min().x() - rays[i].origin().x())* rays[i].invDirection().x();

			FOREACH	Scalar_T::ConditionalSwap( rays[i].sign(0), tXmax[INDEX], tXmin[INDEX] );
		
			// Y
		
			FOREACH	tYmax[INDEX] = (aabbs[j].max().y() - rays[i].origin().y())* rays[i].invDirection().y();
			FOREACH	tYmin[INDEX] = (aabbs[j].min().y() - rays[i].origin().y())* rays[i].invDirection().y();

			FOREACH	Scalar_T::ConditionalSwap( rays[i].sign(1), tYmax[INDEX], tYmin[INDEX] );
		
			FOREACH	valid[INDEX] = (tXmin[INDEX] <= tYmax[INDEX]); 

			FOREACH	valid[INDEX] &= (tYmin[INDEX] <= tXmax[INDEX]);
		
			FOREACH	tXmin[INDEX] = tXmin[INDEX].Max( tYmin[INDEX] );

			FOREACH	tXmax[INDEX] = tXmax[INDEX].Min( tYmax[INDEX] );

			// Z
		
			FOREACH	tZmax[INDEX] = (aabbs[j].max().z() - rays[i].origin().z())* rays[i].invDirection().z();
		
			FOREACH	tZmin[INDEX] = (aabbs[j].min().z() - rays[i].origin().z())* rays[i].invDirection().z();
		
			FOREACH	Scalar_T::ConditionalSwap( rays[i].sign(2), tZmax[INDEX], tZmin[INDEX] );
		
			FOREACH	valid[INDEX] &= (tXmin[INDEX] <= tZmax[INDEX]); 
			FOREACH	valid[INDEX] &= (tZmin[INDEX] <= tXmax[INDEX]);
		
			FOREACH	tXmin[INDEX] = tXmin[INDEX].Max( tZmin[INDEX] );
			FOREACH	tXmax[INDEX] = tXmax[INDEX].Min( tZmax[INDEX] );

			FOREACH tMin[INDEX] = tXmin[INDEX];
			FOREACH tMax[INDEX] = tXmax[INDEX];
	#undef FOREACH
	#undef INDEX
		}
	
	
	};
}


template<class _Options,class _Method,class _RayMode> struct RayAABBIntersection :
	public detail::RayAABBIntersectionBase<_Options,_Method>
{
	typedef detail::RayAABBIntersectionBase<_Options,_Method> Base;
	
	template<class _RawRayArray,class _RawAABBArray,class _RawTArray,class _RawResultArray>
	inline void operator()	(
		const _RawRayArray& rawRays,
		const _RawAABBArray& rawAABBs,
		_RawTArray& rawT,
		_RawResultArray& rawResult
		) const
	{
		// check for hit
		
		std::array<Scalar_T,ResultCount> tMin;
		std::array<Scalar_T,ResultCount> tMax;
		std::array<Boolean,ResultCount> valid;

		Base::operator()(rawRays,rawAABBs,tMin,tMax,valid);
		
		ArrayWrapper<_RawTArray,Scalar_T,ResultCount> t(rawT);
		ArrayWrapper<_RawResultArray,BooleanMask,ResultCount> result(rawResult);

		for(int i = 0; i < ResultCount; ++i)
		{
			valid[i]  &= (tMin[i] < t[i] );
			valid[i]  &= (tMax[i] > Scalar_T::Epsilon());
			t[i].ConditionalAssign(valid[i], tMin[i], t[i]);
			result[i] = valid[i].mask();
		}
	}
};

template<class _Options,class _Method> struct RayAABBIntersection<_Options,_Method,RayModeConstant> :
	public detail::RayAABBIntersectionBase<_Options,_Method>
{
	typedef detail::RayAABBIntersectionBase<_Options,_Method> Base;
	
	template<class _RawRayArray,class _RawAABBArray,class _RawTArray,class _RawResultArray>
	inline void operator()	(
		const _RawRayArray& rawRays,
		const _RawAABBArray& rawAABBs,
		const _RawTArray& rawT,
		_RawResultArray& rawResult
		) const
	{
		// check for hit
		
		std::array<Scalar_T,ResultCount> tMin;
		std::array<Scalar_T,ResultCount> tMax;
		std::array<Boolean,ResultCount> valid;

		Base::operator()(rawRays,rawAABBs,tMin,tMax,valid);
		
		ConstArrayWrapper<_RawTArray,Scalar_T,RayCount> t(rawT);
		ArrayWrapper<_RawResultArray,BooleanMask,ResultCount> result(rawResult);

		for(int i = 0; i < ResultCount; ++i)
		{
			valid[i]  &= (tMin[i] < t[i/AABBCount] );
			valid[i]  &= (tMax[i] > Scalar_T::Epsilon());
			result[i] = valid[i].mask();
		}
	}
};


}

#endif