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
#include "ConcurrentStorage.h"
#include "MathHelper.h"


namespace Raytrace {

#ifdef _DEBUG
static const int RAYS_PER_BLOCK		= 4;
#else
static const int RAYS_PER_BLOCK		= 16;
#endif

template<class _SampleIdType = u32,class _RayIdType = u32> struct SimpleRayData
{
	typedef _SampleIdType SampleIdType;
	typedef _RayIdType	RayIdType;

	inline SimpleRayData()
	{
	}

	inline void prepare(int numThreads) 
	{
		_rayData.prepare(numThreads);
	}

	inline void prepareIntersecting()
	{
		_rayIterator = _rayData.begin(0);
	}

	inline bool hasRaysToIntersect() const
	{
		if(_rayData.endElementId() > 0)
			return true;
		else
			return false;
	}

	inline void threadCompleteIntersecting(int threadId,int nextIntersect)
	{
		//_threadData[threadId]._nextIntersect = nextIntersect;
	}

	inline void completeIntersecting()
	{
		_rayData.popElements(0);
	}
	
	inline RayIdType getNextRays(int thread,RayIdType& count)
	{
		return _rayIterator.getNextElements(thread,count);
	}

	inline RayIdType allocateRays(int threadId,const RayIdType& count)
	{
		return  _rayData.pushElements(threadId,count);
	}

	inline void prepareAllocatingRays()
	{
		//_rayData.setMaxElement(_rayData.endElementId() + _maxRaysPerStack);
	}

	inline void completeAllocatingRays()
	{
		_rayData.finishWritingBlock();
	}

	inline void setRayInitialData(int rayId,const Ray& ray,const SampleIdType& outputId)
	{
		RayElement& e = getRay(rayId);
		e._ray = ray;
		e._outputSampleId =  outputId;
	}

	inline void getRayInfo(int rayId,Ray& ray,SampleIdType& outputSample) const
	{
		const RayElement& e = getRay(rayId);
		ray = e._ray;
		outputSample = e._outputSampleId;
	}
	
	struct RayElement
	{
		inline RayElement() {}

		Ray					_ray;
		SampleIdType		_outputSampleId;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

private:

	static const int CACHE_SIZE = 64;

	inline RayElement& getRay(int rayId)
	{
		return _rayData.getElement(rayId);
	}
	
	inline const RayElement& getRay(int rayId) const
	{
		return _rayData.getElement(rayId);
	}

	typedef ConcurrentStorage<RayElement,RAYS_PER_BLOCK> RayDataType;
		
	typename RayDataType::MultiAccessIterator	_rayIterator;

	RayDataType									_rayData;

};

template<class _SampleIdType = u32,class _RayIdType = u32,class _ShadowIdType = u32> struct DualRayData
{
	typedef _SampleIdType SampleIdType;
	typedef _RayIdType	RayIdType;
	typedef _ShadowIdType	ShadowIdType;

	typedef struct _AnyHitRay{} AnyHitRay;
	typedef struct _FirstHitRay{} FirstHitRay;

	inline DualRayData()
	{
	}

	inline void prepare(int numThreads) 
	{
		boost::fusion::at_key<AnyHitRay>(_data).prepare(numThreads);
		boost::fusion::at_key<FirstHitRay>(_data).prepare(numThreads);
	}

	inline void prepareIntersecting()
	{
		boost::fusion::at_key<AnyHitRay>(_data).prepareIntersecting();
		boost::fusion::at_key<FirstHitRay>(_data).prepareIntersecting();
	}

	inline bool hasRaysToIntersect() const
	{
		return boost::fusion::at_key<AnyHitRay>(_data).hasRaysToIntersect() || boost::fusion::at_key<FirstHitRay>(_data).hasRaysToIntersect();
	}

	template<class _RayType> inline void threadCompleteIntersecting(int threadId,int nextIntersect)
	{
		boost::fusion::at_key<_RayType>(_data).threadCompleteIntersecting(threadId,nextIntersect);
	}

	inline void completeIntersecting()
	{
		boost::fusion::at_key<AnyHitRay>(_data).completeIntersecting();
		boost::fusion::at_key<FirstHitRay>(_data).completeIntersecting();
	}
	
	template<class _RayType> inline RayIdType getNextRays(int thread,RayIdType& count)
	{
		return boost::fusion::at_key<_RayType>(_data).getNextRays(thread,count);
	}
		
	template<class _RayType> inline RayIdType allocateRays(int threadId,const RayIdType& count)
	{
		return boost::fusion::at_key<_RayType>(_data).allocateRays(threadId,count);
	}

	inline void prepareAllocatingRays()
	{
		boost::fusion::at_key<AnyHitRay>(_data).prepareAllocatingRays();
		boost::fusion::at_key<FirstHitRay>(_data).prepareAllocatingRays();
	}

	inline void completeAllocatingRays()
	{
		boost::fusion::at_key<AnyHitRay>(_data).completeAllocatingRays();
		boost::fusion::at_key<FirstHitRay>(_data).completeAllocatingRays();
	}

	template<class _RayType> struct OutputIdType
	{
		typedef void type;
	};

	template<> struct OutputIdType<AnyHitRay>
	{
		typedef ShadowIdType type;
	};
	template<> struct OutputIdType<FirstHitRay>
	{
		typedef SampleIdType type;
	};

	template<class _RayType> inline void setRayInitialData(RayIdType rayId,const Ray& ray,const typename OutputIdType<_RayType>::type& outputId)
	{
		return  boost::fusion::at_key<_RayType>(_data).setRayInitialData(rayId,ray,outputId);
	}

	template<class _RayType> inline void getRayInfo(RayIdType rayId,Ray& ray,typename OutputIdType<_RayType>::type& outputId) const
	{
		return  boost::fusion::at_key<_RayType>(_data).getRayInfo(rayId,ray,outputId);
	}
private:


	typedef SimpleRayData<SampleIdType,RayIdType> FirstRayData;
	typedef SimpleRayData<ShadowIdType,RayIdType> AnyRayData;

	typedef boost::fusion::map<
				std::pair<AnyHitRay,AnyRayData>,
				std::pair<FirstHitRay,FirstRayData>
	> DataMapType;

	DataMapType	_data;
};

}
#endif