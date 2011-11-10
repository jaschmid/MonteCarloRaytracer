/********************************************************/
// FILE: BVHIntersector.h
// DESCRIPTION: Ray Intersector using MBVH's
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

#ifndef RAYTRACE_BVH_INTERSECTOR_GUARD
#define RAYTRACE_BVH_INTERSECTOR_GUARD

#include <RaytraceCommon.h>
#include "Ray.h"
#include "EngineBase.h"
#include "ConcurrentStorage.h"
#include "MathHelper.h"
#include "RayData.h"
#include "BVH.h"

namespace Raytrace {

template<class _SampleData,class _RayData> struct BVHIntersectorEngine;

struct BVHIntersector
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef BVHIntersectorEngine<_SampleData,_RayData> type;
	};
	typedef SimpleRayData ExternalData;
	typedef EngineMultiThreaded ThreadingMode;
	typedef EngineIntersector EngineType;
};

template<class _SampleData,class _RayData> struct BVHIntersectorEngine
{
	struct TriangleElement
	{
		TriAccel	_triangle;
	};
	
	typedef BVH<TriangleElement> BVHType;
	typedef _SampleData SampleData;
	typedef _RayData RayData;

	inline BVHIntersectorEngine(const BVHIntersector& shader,_SampleData& sampleData,RayData& rayData,const SceneReader& r) : _rayData(rayData),_sampleData(sampleData)
	{
		BVHType::Constructor constructor(16);

		int num = r.getNumTriangles();
		for(int i = 0; i< num; ++i)
		{
			Triangle tri;
			int material;
			r.getTriangle(i,tri,material);

			AABB volume(tri);

			Vector3 centroid = (tri.point(0) + tri.point(1) + tri.point(2))/3.0f;

			TriangleElement item;
			item._triangle = TriAccel(tri,i,material);
			constructor.addElement( item, centroid, volume );
		}

		_sceneData.reset( new BVHType(constructor) );
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
			Ray ray;
			_rayData.getRayInfo(rayId,ray);
			int triId = -1; 
			Real t = ray.length();
			if(t < .0f)
				t = std::numeric_limits<Real>::infinity();
			Vector2 bary;
			processNode(ray, _sceneData->root() , triId, t, bary);

			/*

			float t= 1000.0f,tTemp = 1000.0f;
			Vector2 bary,baryTemp;
			int triId = -1;

			for(auto it = _sceneData.begin(); it != _sceneData.end() ; ++it)
				if(Intersect(ray,it->_triangle,tTemp,baryTemp) && (triId == -1 || tTemp < t) )
				{
					t = tTemp;
					bary = baryTemp;
					triId = it->_triangle._user1;
				}*/

			if(triId != -1)
				_sampleData.pushSample(threadId,rayId,triId,bary);
			else
				_sampleData.pushSample(threadId,rayId,-1,Vector2(0.0f,0.0f));
				
			count = 1;
			rayId = _rayData.getNextRays(threadId,count);
		}
		_rayData.threadCompleteIntersecting(threadId,rayId);
	}

	static void processNode(const RayAccel& ray,const typename BVHType::nodeIterator& it,int& triId,Real& t,Vector2& bary)
	{
		if(it.leaf())
		{
			Real tTemp = t;
			Vector2 baryTemp = bary;

			int size;
			const BVHType::LeafElement* leaf = it.leaf(size);

			for(int i = 0; i < size; ++ i)
				if( Intersect(ray,(*leaf)[i]._triangle,tTemp,baryTemp) && tTemp < t)
				{
					t = tTemp;
					triId = (*leaf)[i]._triangle._user1;
					bary = baryTemp;
				}
		}
		else
		{
			std::array<bool,BVHType::NodeSize> iChild;
			std::array<Real,BVHType::NodeSize> tChild;
			std::array<BVHType::nodeIterator,BVHType::NodeSize> child;
			size_t num;
			for(num = 0; num < BVHType::NodeSize; ++num)
			{
				if( (child[num] = it.node(num)).valid() )
					iChild[num] = Intersect( ray, child[num].volume(), tChild[num]);
				else
					break;
			}

			for(size_t i = 0; i < num; ++i)
				if(iChild[i] && tChild[i] < t)
					processNode( ray, child[i], triId, t, bary);
		}
	}

	std::auto_ptr<BVHType>	_sceneData;

	SampleData& _sampleData;
	RayData& _rayData;
};

}
#endif