/********************************************************/
// FILE: SimpleIntersector.h
// DESCRIPTION: Simple Ray Intersector using list of triangles
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

#ifndef RAYTRACE_SIMPLE_INTERSECTOR_GUARD
#define RAYTRACE_SIMPLE_INTERSECTOR_GUARD

#include <RaytraceCommon.h>
#include "Ray.h"
#include "EngineBase.h"
#include "ConcurrentStorage.h"
#include "MathHelper.h"
#include "RayData.h"

namespace Raytrace {

template<class _SampleData,class _RayData> struct SimpleIntersectorEngine;

struct SimpleIntersector
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef SimpleIntersectorEngine<_SampleData,_RayData> type;
	};
	typedef SimpleRayData ExternalData;
	typedef EngineMultiThreaded ThreadingMode;
	typedef EngineIntersector EngineType;
};

template<class _SampleData,class _RayData> struct SimpleIntersectorEngine
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;

	inline SimpleIntersectorEngine(const SimpleIntersector& shader,_SampleData& sampleData,RayData& rayData,const SceneReader& r) : _rayData(rayData),_sampleData(sampleData)
	{
		int num = r.getNumTriangles();
		_sceneData.resize(num);
		for(int i = 0; i< num; ++i)
		{
			Triangle tri;
			int material;
			r.getTriangle(i,tri,material);
			_sceneData[i]._triangle = TriAccel(tri,i,material);
		}
	}
	inline void prepare(int numThreads) 
	{
	}
	inline void threadEnter(int threadId) 
	{
		int count = 1;
		int rayId = _rayData.getNextRays(threadId,count);
		while(count)
		{
			Ray rayBase;
			_rayData.getRayInfo(rayId,rayBase);
			RayAccel ray(rayBase);

			float t= 1000.0f,tTemp = 1000.0f;
			Vector2 bary,baryTemp;
			int triId = -1;

			for(auto it = _sceneData.begin(); it != _sceneData.end() ; ++it)
				if(Intersect(ray,it->_triangle,tTemp,baryTemp) && (triId == -1 || tTemp < t) )
				{
					t = tTemp;
					bary = baryTemp;
					triId = it->_triangle._user1;
				}

			if(triId != -1)
				_sampleData.pushSample(threadId,rayId,triId,bary);
			else
				_sampleData.pushSample(threadId,rayId,-1,Vector2(0.0f,0.0f));

			count = 1;
			rayId = _rayData.getNextRays(threadId,count);
		}
		_rayData.threadCompleteIntersecting(threadId,rayId);
	}

	struct TriangleElement
	{
		TriAccel	_triangle;
	};

	std::vector<TriangleElement,Eigen::aligned_allocator<TriangleElement>>	_sceneData;

	SampleData& _sampleData;
	RayData& _rayData;
};

}
#endif