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
#include "Ray.h"
#include "EngineBase.h"
#include "ConcurrentStorage.h"
#include "MathHelper.h"


namespace Raytrace {

#ifdef _DEBUG
static const int MAX_RAYS_PER_STACK = 128;
static const int RAYS_PER_BLOCK		= 4;
#else
static const int MAX_RAYS_PER_STACK = 16*1024;
static const int RAYS_PER_BLOCK		= 64;
#endif

struct SimpleRayData
{
	inline SimpleRayData() : _nextIntersect(0),_maxRaysPerStack(MAX_RAYS_PER_STACK)
	{
	}
	inline void prepare(int numThreads) 
	{
		_rayData.prepare(numThreads);
		_threadData.resize(numThreads);
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			it->_nextIntersect = _nextIntersect;
	}

	inline void prepareIntersecting()
	{
		_rayIterator = _rayData.begin(_nextIntersect);
	}

	inline bool hasRaysToIntersect() const
	{
		if(_rayData.endElementId() > _nextIntersect)
			return true;
		else
			return false;
	}

	inline void threadCompleteIntersecting(int threadId,int nextIntersect)
	{
		_threadData[threadId]._nextIntersect = nextIntersect;
	}

	inline void completeIntersecting()
	{
		int min = 0x7fffffff;
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			if(it->_nextIntersect < min)
				min = it->_nextIntersect;
		_nextIntersect = min;
	}

	inline void pushStack()
	{
		StackLevel level;
		level._firstRay = _nextIntersect;
		level._endRay = _rayData.endElementId();
		_rayStack.push_back(level);
	}

	inline void popStack()
	{
		_nextIntersect = _rayStack.back()._firstRay;
		_rayData.popElements(_rayStack.back()._endRay);
		_rayStack.pop_back();
	}
	
	inline int getNextRays(int thread,int& count)
	{
		return _rayIterator.getNextElements(thread,count);
	}

	inline int allocateRays(int threadId,int count)
	{
		return  _rayData.pushElements(threadId,count);
	}

	inline void prepareAllocatingRays()
	{
		_rayData.setMaxElement(_rayData.endElementId() + _maxRaysPerStack);
	}

	inline void completeAllocatingRays()
	{
		_rayData.finishWritingBlock();
	}

	inline void setRayInitialData(int rayId,const Ray& ray,float contribution)
	{
		RayElement& e = getRay(rayId);
		e._ray = ray;
		e._contribution = contribution;
	}

	inline void shadeRay(int rayId,const Vector4& color)
	{
		RayElement& e = getRay(rayId);
		e._color = color;
	}

	inline void getRayInfo(int rayId,Ray& ray) const
	{
		const RayElement& e = getRay(rayId);
		ray = e._ray;
	}

	inline void getRayShadingInfo(int rayId,Vector4& color,float& contribution) const
	{
		const RayElement& e = getRay(rayId);
		color = e._color;
		contribution = e._contribution;
	}
	
	struct RayElement
	{
		inline RayElement() : _color(-1.0f,-1.0f,-1.0f,-1.0f) {}

		Ray		_ray;
		float	_contribution;
		Vector4 _color;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

private:

	inline RayElement& getRay(int rayId)
	{
		return _rayData.getElement(rayId);
	}
	
	inline const RayElement& getRay(int rayId) const
	{
		return _rayData.getElement(rayId);
	}

	typedef ConcurrentStorage<RayElement,RAYS_PER_BLOCK> RayDataType;


	struct StackLevel
	{
		int _firstRay;
		int _endRay;
	};

	struct ThreadData
	{
		int _nextIntersect;
	};

	int											_nextIntersect;
	const int									_maxRaysPerStack;

	RayDataType::MultiAccessIterator			_rayIterator;
	RayDataType									_rayData;
	std::vector<StackLevel>						_rayStack;
	std::vector<ThreadData>						_threadData;

};

}
#endif