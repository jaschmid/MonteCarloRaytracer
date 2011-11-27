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
#include "RayAABBIntersection.h"
#include "RayTriangleIntersection.h"

namespace Raytrace {

template<class _SampleData,class _RayData,int _SimdWidth> struct SimpleIntersectorEngine;

struct SimpleIntersector
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef SimpleIntersectorEngine<_SampleData,_RayData,4> type;
	};

	typedef DualRayData<> ExternalData;
	typedef EngineMultiThreaded ThreadingMode;
	typedef EngineIntersector EngineType;
};

template<class _SampleData,class _RayData,int _SimdWidth> struct SimpleIntersectorEngine
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;

	typedef typename _RayData::RayIdType RayIdType;
	static const int SimdWidth = _SimdWidth;

	typedef SimdType<int,SimdWidth>	Scalari_T;
	typedef SimdType<float,SimdWidth>	Scalar_T;
	typedef typename Vector2v<SimdWidth>::type Vector2_T;
	typedef typename Vector3v<SimdWidth>::type Vector3_T;

	typedef TriAccel<SimdWidth>		PrimitiveType;

	typedef typename RayData::AnyHitRay AnyHitRay;
	typedef typename RayData::FirstHitRay FirstHitRay;
	
	template<class _RayType> struct RayTypeInfo
	{

	};

	template<> struct RayTypeInfo<AnyHitRay>
	{
		typedef RayAccel<SimdType<f32,SimdWidth>,3,SignModePrecompute,InvDirModePrecompute> type;
		typedef typename Intersector<boost::mpl::vector< 
				Raytrace::RayType<type>,
				Raytrace::PrimitiveType<PrimitiveType>, 
				Raytrace::RayCount<1>, 
				Raytrace::PrimitiveCount<1>,
				Raytrace::RayModeConstant
			>>::type intersector;
	};
	
	template<> struct RayTypeInfo<FirstHitRay>
	{
		typedef RayAccel<SimdType<f32,SimdWidth>,3,SignModePrecompute,InvDirModePrecompute> type;
		typedef typename Intersector<boost::mpl::vector< 
				Raytrace::RayType<type>,
				Raytrace::PrimitiveType<PrimitiveType>, 
				Raytrace::RayCount<1>, 
				Raytrace::PrimitiveCount<1>,
				Raytrace::RayModeUpdateMinimum
			>>::type intersector;
	};

	inline SimpleIntersectorEngine(const SimpleIntersector& shader,_SampleData& sampleData,RayData& rayData,const SceneReader& r) : _rayData(rayData),_sampleData(sampleData)
	{
		int num = r.getNumTriangles();
		if(num > 0)
		{
			_sceneData.resize( (num -1) / SimdWidth + 1);
			for(int i = 0; i < (int)_sceneData.size(); ++i)
			{
				std::array<PrimitiveType::Minimum,SimdWidth> primitives;

				for(int j = 0; j < SimdWidth; ++j)
				{
					int material;
					int index = i*SimdWidth + j;
					if(index < num)
					{
						r.getTriangle(index,primitives[j],material);
						primitives[j]._user = index + 1;
					}
					else
					{
						primitives[j].setPoint(0,Vector3(0.0f,0.0f,0.0f));
						primitives[j].setPoint(1,Vector3(0.0f,0.0f,0.0f));
						primitives[j].setPoint(2,Vector3(0.0f,0.0f,0.0f));
						primitives[j]._user = index + 1;
					}
				}

				_sceneData[i] = PrimitiveType(primitives);
			}
		}
	}
	inline void prepare(int numThreads) 
	{
	}
	inline void threadEnter(int threadId) 
	{
		doIntersections<typename RayData::AnyHitRay>(threadId);
		doIntersections<typename RayData::FirstHitRay>(threadId);
	}

	template<class _RayType> void doIntersections(int threadId)
	{
		RayIdType count = 1;
		RayIdType rayId = _rayData.getNextRays<_RayType>(threadId,count);
		while(count)
		{
			Ray rayBase;
			typename RayData::OutputIdType<_RayType>::type outputSample;
			_rayData.getRayInfo<_RayType>(rayId,rayBase,outputSample);

			processRay<_RayType>(threadId,rayBase,outputSample);

			count = 1;
			rayId = _rayData.getNextRays<_RayType>(threadId,count);
		}
		_rayData.threadCompleteIntersecting<_RayType>(threadId,rayId);
	}
	
	template<class _RayType> void processRay(int threadId,const Ray& ray,const typename RayData::OutputIdType<_RayType>::type& outputSample)
	{
		static_assert(false,"Unsupported Ray Type")
	}

	template<> void processRay<typename RayData::AnyHitRay>(int threadId,const Ray& rayBase,const typename RayData::OutputIdType<typename RayData::AnyHitRay>::type& outputSample)
	{
		typedef RayData::AnyHitRay RayType;
		std::array<Ray,SimdWidth> rayArray;
		std::array<Real,SimdWidth> tArray;

		for(int i = 0; i < SimdWidth; ++i)
		{
			rayArray[i] = rayBase;
			tArray[i] = rayBase.length();
			if(tArray[i] < .0f)
				tArray[i] = std::numeric_limits<Real>::infinity();
		}

		RayTypeInfo<RayType>::type ray(rayArray);
		RayTypeInfo<RayType>::type::Scalar_T tTemp(tArray);

		bool found = false;
		
		for(auto it = _sceneData.begin(); it != _sceneData.end() ; ++it)
			if( RayTypeInfo<RayType>::intersector()(ray,*it,tTemp) )
			{
				_sampleData.setIntersectionResult(outputSample,true);
				return;
			}

		_sampleData.setIntersectionResult(outputSample,false);
	}

	template<> void processRay<typename RayData::FirstHitRay>(int threadId,const Ray& rayBase,const typename RayData::OutputIdType<typename RayData::FirstHitRay>::type& outputSample)
	{
		typedef RayData::FirstHitRay RayType;
		std::array<Ray,SimdWidth> rayArray;

		for(int i = 0; i < SimdWidth; ++i)
			rayArray[i] = rayBase;

		RayTypeInfo<RayType>::type ray(rayArray);

		Scalar_T tTemp(rayBase.length() > .0f ? rayBase.length() : std::numeric_limits<Real>::infinity());
		Vector2_T baryTemp;
		Scalari_T triIds(-1);

		for(auto it = _sceneData.begin(); it != _sceneData.end() ; ++it)
			RayTypeInfo<RayType>::intersector()(ray,*it,tTemp,baryTemp,triIds);
		
		Real t = std::numeric_limits<Real>::infinity();
		Vector2 bary;
		int triId = 0;

		for(int i = 0; i < SimdWidth; ++i)
			if(triIds[i] != 0 && tTemp[i] < t)
			{
				t = tTemp[i];
				bary.x() = baryTemp.x()[i];
				bary.y() = baryTemp.y()[i];
				triId = triIds[i];
			}
		
		if(triId != 0)
			_sampleData.setIntersectionResult(threadId,outputSample,triId - 1,bary,rayBase.direction());
		else
			_sampleData.setIntersectionResult(threadId,outputSample,-1,Vector2(0.0f,0.0f),rayBase.direction());
	}

	std::vector<PrimitiveType,Eigen::aligned_allocator<PrimitiveType>>	_sceneData;

	SampleData& _sampleData;
	RayData& _rayData;
};

}
#endif