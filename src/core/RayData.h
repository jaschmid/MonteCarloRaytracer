/********************************************************/
// FILE: RayData.h
// DESCRIPTION: Data container for Rays
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

#ifndef RAYTRACE_RAY_DATA_GUARD
#define RAYTRACE_RAY_DATA_GUARD

#include <RaytraceCommon.h>
#include <boost/fusion/include/map.hpp>
#include <boost/fusion/include/at_key.hpp>
#include "Ray.h"
#include "EngineBase.h"
#include "SplitUseWorkQueue.h"
#include "MathHelper.h"


namespace Raytrace {
	
struct AnyHitRay{};
struct FirstHitRay{};

namespace fusion = boost::fusion;
namespace mpl = boost::mpl;

typedef boost::mpl::vector< AnyHitRay , FirstHitRay >::type RayClassifications;

template<class _RayType,class _PrimitiveType,int _RaysPerBlock> struct SimpleRayData
{
	
	template<class _RayClassification> struct Element;

public:
	
	static const size_t RaysPerBlock = _RaysPerBlock;
	static const size_t Dimensions = _RayType::Dimensions;

	typedef _RayType RayType;
	typedef _PrimitiveType PrimitiveType;
	typedef typename _RayType::Scalar_T Scalar;
	typedef typename _RayType::RelativeLocation			RayRelativeIntersection;
	typedef typename _PrimitiveType::RelativeLocation	PrimitiveRelativeIntersection;
	typedef typename _PrimitiveType::UserData			PrimitiveUserData;
	typedef Eigen::Matrix<Scalar,Dimensions,1>			AbsoluteIntersectionLocation;
	

	static_assert(_RayType::Dimensions == _PrimitiveType::Dimensions, "Ray type and Primitive type must have the same amount of dimensions!");
	static_assert(std::is_same<typename _RayType::Scalar_T,typename _PrimitiveType::Scalar_T>::value, "Ray and primitive must have same scalar type!");

	
	inline void PrepareIntersectST()
	{
	}
	
	inline void PrepareIntegrateST()
	{
	}
	
	inline void CompleteIntersectST()
	{
		fusion::at_key<FirstHitRay>(_data).clear();
		fusion::at_key<AnyHitRay>(_data).clear();
	}
	
	inline void CompleteIntegrateST()
	{
	}

	template<class _RayClass> inline bool popRay(size_t threadId,Element<_RayClass>& dataOut)
	{
		fusion::at_key<_RayClass>(_data).advanceElement(threadId);
		Element<_RayClass>* p_ray = fusion::at_key<_RayClass>(_data).currElement(threadId);
		if(p_ray)
		{
			dataOut = *p_ray;
			return true;
		}
		else
			return false;
	}

	inline void pushRay(size_t threadId,const RayType& ray,
		AbsoluteIntersectionLocation* absoluteIntersectionLocation,
		RayRelativeIntersection* rayRelativeIntersectionLocation,
		PrimitiveRelativeIntersection* primitiveRelativeIntersectionLocation,
		PrimitiveUserData* primitiveIdentifier)
	{
		Element<FirstHitRay> element;

		element.ray = ray;
		element.absoluteIntersectionLocation = absoluteIntersectionLocation;
		element.rayRelativeIntersectionLocation = rayRelativeIntersectionLocation;
		element.primitiveRelativeIntersectionLocation = primitiveRelativeIntersectionLocation;
		element.primitiveIdentifier = primitiveIdentifier;

		fusion::at_key<FirstHitRay>(_data).pushElement(element,threadId);
	}
	
	inline void pushRay(size_t threadId,const RayType& ray,u32* pResultOut,u32 resultIndex = 0)
	{
		Element<AnyHitRay> element;
		element.ray = ray;
		element.resultOut = pResultOut;
		element.resultIndex = resultIndex;

		fusion::at_key<AnyHitRay>(_data).pushElement(element,threadId);
	}

	inline void InitializeST(size_t numThreads)
	{
		fusion::at_key<FirstHitRay>(_data).prepare(numThreads);
		fusion::at_key<AnyHitRay>(_data).prepare(numThreads);
	}
	
	inline size_t getNumFirstHitRays() const
	{
		return fusion::at_key<FirstHitRay>(_data).size();
	}

	inline size_t getNumAnyHitRays() const
	{
		return fusion::at_key<AnyHitRay>(_data).size();
	}

	template<class _RayClassification> struct Element
	{
		RayType ray;
		AbsoluteIntersectionLocation* absoluteIntersectionLocation;
		RayRelativeIntersection* rayRelativeIntersectionLocation;
		PrimitiveRelativeIntersection* primitiveRelativeIntersectionLocation;
		PrimitiveUserData* primitiveIdentifier;
	};

	template<> struct Element<AnyHitRay>
	{
		RayType ray;
		up* resultOut;
		up	 resultIndex;
	};

	typedef SplitUseWorkQueue<Element<AnyHitRay>,RaysPerBlock> AnyRayData;
	typedef SplitUseWorkQueue<Element<FirstHitRay>,RaysPerBlock> FirstRayData;

	typedef boost::fusion::map<
				std::pair<AnyHitRay,AnyRayData>,
				std::pair<FirstHitRay,FirstRayData>
	> DataMapType;

	DataMapType	_data;
};

}
#endif