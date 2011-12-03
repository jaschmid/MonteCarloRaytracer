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
#include "MathHelper.h"
#include "RayData.h"
#include "RayAABBIntersection.h"
#include "RayTriangleIntersection.h"
#include "IIntersector.h"
#include "SceneReader.h"
#include "Aligned.h"

namespace Raytrace {

template<class _RayData,class _SceneReader,int _SimdWidth> struct SimpleIntersector : public IIntersector<_RayData,_SceneReader>
{
	typedef _RayData RayData;
	typedef _SceneReader SceneReader;
	static const int SimdWidth = _SimdWidth;

	typedef typename RayData::RayType BaseRayType;
	typedef typename RayData::PrimitiveType BasePrimitiveType;

	typedef SimdType<int,SimdWidth>	Scalari_T;
	typedef SimdType<float,SimdWidth>	Scalar_T;
	typedef typename Vector2v<SimdWidth>::type Vector2_T;
	typedef typename Vector3v<SimdWidth>::type Vector3_T;

	typedef mpl::vector<
				RaySignModePrecompute,
				RayLengthModeNone,
				RayInvDirModePrecompute,
				RayScalarType<Scalar_T>,
				RayDimensions<BaseRayType::Dimensions> > RayOptions;
	typedef mpl::vector<
				TriangleBaseModeBarycentricPlane,
				TriangleScalarType<Scalar_T>,
				TriangleDimensions<BasePrimitiveType::Dimensions>,
				TriangleUserDataType<Scalari_T>	> PrimitiveOptions;

	typedef mpl::vector< TriangleUserDataType<int>	> PrimitiveUserOptions;

	typedef typename BaseRayType::adapt<RayOptions>::type		RayType;

	typedef typename BasePrimitiveType::adapt<PrimitiveUserOptions>::type UserPrimitiveType;

	typedef typename BasePrimitiveType::adapt<PrimitiveOptions>::type	PrimitiveType;
	
	template<class _RayType> struct RayTypeInfo
	{

	};

	template<> struct RayTypeInfo<AnyHitRay>
	{
		typedef typename SimpleIntersector::RayType type;
		typedef typename Intersector<boost::mpl::vector< 
				Raytrace::RayType<typename SimpleIntersector::RayType>,
				Raytrace::PrimitiveType<typename SimpleIntersector::PrimitiveType>, 
				Raytrace::RayCount<1>, 
				Raytrace::PrimitiveCount<1>,
				Raytrace::RayModeConstant
			>>::type intersector;
	};
	
	template<> struct RayTypeInfo<FirstHitRay>
	{
		typedef typename SimpleIntersector::RayType type;
		typedef typename Intersector<boost::mpl::vector< 
				Raytrace::RayType<typename SimpleIntersector::RayType>,
				Raytrace::PrimitiveType<typename SimpleIntersector::PrimitiveType>, 
				Raytrace::RayCount<1>, 
				Raytrace::PrimitiveCount<1>,
				Raytrace::RayModeUpdateMinimum
			>>::type intersector;
	};

	
	void InitializePrepareST(size_t numThreads,const SceneReader& scene,RayData& rayData) 
	{
		_rayData = &rayData;
		int num = scene->getNumPrimitives();
		if(num > 0)
		{
			_sceneData.resize( (num -1) / SimdWidth + 1);
			for(int i = 0; i < (int)_sceneData.size(); ++i)
			{
				std::array<UserPrimitiveType,SimdWidth> primitives;

				for(int j = 0; j < SimdWidth; ++j)
				{
					int material;
					int index = i*SimdWidth + j;
					if(index < num)
					{
						BasePrimitiveType base;
						scene->getPrimitive(index,base,material);
						primitives[j] = base;
						primitives[j].setUser( index + 1 );
					}
					else
					{
						primitives[j].setPoint(0,Vector3(0.0f,0.0f,0.0f));
						primitives[j].setPoint(1,Vector3(0.0f,0.0f,0.0f));
						primitives[j].setPoint(2,Vector3(0.0f,0.0f,0.0f));
						primitives[j].setUser( index + 1 );
					}
				}

				_sceneData[i] = PrimitiveType(ConstArrayWrapper<std::array<UserPrimitiveType,SimdWidth>>(primitives));
			}
		}
	}
	void InitializeMT(size_t threadId) 
	{
	}
	void InitializeCompleteST() 
	{
	}

	void IntersectPrepareST() 
	{
	}
	void IntersectMT(size_t threadId) 
	{
		doIntersections<AnyHitRay>(threadId);
		doIntersections<FirstHitRay>(threadId);
	}
	void IntersectCompleteST() 
	{
	}
	

	template<class _RayType> inline void doIntersections(int threadId)
	{
		typename RayData::Element<_RayType> element;
		while(_rayData->popRay<_RayType>(threadId,element))
			processRay<_RayType>(element);
	}
		
	template<class _RayType> void processRay(const typename RayData::Element<_RayType>& it);

	template<> void processRay<AnyHitRay>(const typename RayData::Element<AnyHitRay>& element)
	{
		const BaseRayType& rayBase = element.ray;

		std::array<BaseRayType,SimdWidth> rayArray;

		for(int i = 0; i < SimdWidth; ++i)
			rayArray[i] = rayBase;

		RayTypeInfo<AnyHitRay>::type ray(rayArray);

		Scalar_T tTemp(rayBase.length() > .0f ? rayBase.length() : std::numeric_limits<Real>::infinity());
		
		bool found = false;

		for(auto it = _sceneData.begin(); it != _sceneData.end() ; ++it)
			if( RayTypeInfo<AnyHitRay>::intersector()(ray,*it,tTemp) )
			{
				found = true;
				break;
			}

		if(found)
			(*element.resultOut) = 1;
		else
			(*element.resultOut) = 0;
	}

	template<> void processRay<FirstHitRay>(const typename RayData::Element<FirstHitRay>& element)
	{
		const BaseRayType& rayBase = element.ray;

		std::array<BaseRayType,SimdWidth> rayArray;

		for(int i = 0; i < SimdWidth; ++i)
			rayArray[i] = rayBase;

		RayTypeInfo<FirstHitRay>::type ray(rayArray);

		Scalar_T tTemp(rayBase.length() > .0f ? rayBase.length() : std::numeric_limits<Real>::infinity());
		Vector2_T baryTemp;
		Scalari_T triIds(0);

		for(auto it = _sceneData.begin(); it != _sceneData.end() ; ++it)
			RayTypeInfo<FirstHitRay>::intersector()(ray,*it,tTemp,baryTemp,triIds);
		
		Real t = std::numeric_limits<Real>::infinity();
		Vector2 bary;
		int triId = 0;

		for(int i = 0; i < SimdWidth; ++i)
			if(triIds[i] != 0)
			{
				t = tTemp[i];
				bary.x() = baryTemp.x()[i];
				bary.y() = baryTemp.y()[i];
				triId = triIds[i];
			}
		
		if(triId != 0)
		{
			if(element.absoluteIntersectionLocation)(*element.absoluteIntersectionLocation) = rayBase.origin()+ rayBase.direction()*t;
			if(element.rayRelativeIntersectionLocation)(*element.rayRelativeIntersectionLocation) = t;
			if(element.primitiveRelativeIntersectionLocation)(*element.primitiveRelativeIntersectionLocation) = bary;
			if(element.primitiveIdentifier)(*element.primitiveIdentifier) = triId - 1;
		}
		else
		{
			if(element.absoluteIntersectionLocation)(*element.absoluteIntersectionLocation) = Vector3(0.0f,0.0f,0.0f);
			if(element.rayRelativeIntersectionLocation)(*element.rayRelativeIntersectionLocation) = -1.0f;
			if(element.primitiveRelativeIntersectionLocation)(*element.primitiveRelativeIntersectionLocation) = Vector2(0.0f,0.0f);
			if(element.primitiveIdentifier)(*element.primitiveIdentifier) = -1;
		}
	}

	std::vector<PrimitiveType,AlignedAllocator<PrimitiveType>>	_sceneData;

	RayData* _rayData;

};

}
#endif