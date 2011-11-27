/********************************************************/
// FILE: WhittedShader.h
// DESCRIPTION: Raytracer Whitted Shader
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

#ifndef RAYTRACE_WHITTED_SHADER_GUARD
#define RAYTRACE_WHITTED_SHADER_GUARD

#include <RaytraceCommon.h>
#include "EngineBase.h"
#include "ConcurrentStorage.h"
#include "SampleData.h"


namespace Raytrace {

template<class _SampleData,class _RayData> struct WhittedShaderEngine;

struct WhittedShader
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef WhittedShaderEngine<_SampleData,_RayData> type;
	};
	typedef SimpleSampleData<> ExternalData;
	typedef EngineMultiThreaded ThreadingMode;
	typedef EngineSampler EngineType;
};


template<class _SampleData,class _RayData> struct WhittedShaderEngine
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;
	
	typedef typename SampleData::IntersectionIdType IntersectionIdType;
	typedef typename SampleData::SampleIdType SampleIdType;
	typedef typename SampleData::ShadowIdType ShadowIdType;
	typedef typename RayData::RayIdType RayIdType;

	inline WhittedShaderEngine(const WhittedShader& shader,_SampleData& sampleData,RayData& rayData,const SceneReader& r) :
		_sampleData(sampleData),_rayData(rayData)
	{

		Triangle dummy;

		_triangles.resize(r.getNumTriangles());
		_materials.resize(r.getNumMaterials());
		

		for(size_t i = 0; i < _materials.size(); ++i)
		{
			Material mat = r.getMaterial(i);

			_materials[i]._color = mat->GetColor().head<3>();
		}

		for(size_t i = 0; i < _triangles.size(); ++i)
		{
			Triangle tri;
			r.getTriangle((int)i,tri,_triangles[i]._material);
			assert( _triangles[i]._material < (int)_materials.size() );
			
			_triangles[i]._location = tri.point(0);
			_triangles[i]._baryUDir = tri.point(1) - tri.point(0);
			_triangles[i]._baryVDir = tri.point(2) - tri.point(0);

			_triangles[i]._normal = _triangles[i]._baryUDir.cross(_triangles[i]._baryVDir).normalized();
		}
	}
	inline void prepare(int numThreads) {}
	inline void threadEnterSubsample(int threadId) 
	{
		static const Vector3 lightSource(0.0f,0.0f,1.5f);

		IntersectionIdType count = 1;
		IntersectionIdType intersectionId = _sampleData.getNextIntersections(threadId,count);
		while(count)
		{
			int triangleId;
			Vector2 barycentric;
			Vector3 parentDirection;

			_sampleData.getIntersectionResult(intersectionId,triangleId,barycentric,parentDirection);
			
			if(triangleId != -1)
			{
				const TriData& triangle = _triangles[triangleId];

				Vector3 location = triangle._baryUDir*barycentric.x() + triangle._baryVDir*barycentric.y() + triangle._location;

				auto subsampleIds = _sampleData.allocateSubsamples(threadId,intersectionId,0,1);
				RayIdType rayId = _rayData.allocateRays<RayData::AnyHitRay>(threadId,1);

				Ray ray(location,(lightSource-location).normalized(),(lightSource-location).norm());

				_rayData.setRayInitialData<RayData::AnyHitRay>(rayId, ray, subsampleIds.second);
			}
			else
				auto subsampleIds = _sampleData.allocateSubsamples(threadId,intersectionId,0,0);

			count = 1;
			intersectionId = _sampleData.getNextIntersections(threadId,count);
		}
		_sampleData.threadCompleteSubsampling(threadId,intersectionId);
	}
	inline void threadEnterShade(int threadId) 
	{
		static const Vector3 lightSource(0.0f,0.0f,1.5f);

		IntersectionIdType count = 1;
		IntersectionIdType intersectionId = _sampleData.getNextIntersections(threadId,count);
		while(count)
		{
			int triangleId;
			SampleIdType parentSample;
			Vector2 barycentric;
			Vector3 parentDirection;
			SampleIdType firstSubsample;
			SampleIdType numSubsamples;
			ShadowIdType firstShadow;
			ShadowIdType numShadows;

			_sampleData.getIntersectionResult(intersectionId,triangleId,barycentric,parentDirection);
			_sampleData.getSubsampleInfo(intersectionId,parentSample,firstSubsample,numSubsamples,firstShadow,numShadows);

			if(triangleId != -1)
			{
				int materialId = _triangles[triangleId]._material;
				const TriData& triangle = _triangles[triangleId];

				Vector3 location = triangle._baryUDir*barycentric.x() + triangle._baryVDir*barycentric.y() + triangle._location;

				Vector3 lightDir = (lightSource-location);

				Real diffuseFactor = lightDir.normalized().dot( triangle._normal );

				if(diffuseFactor < 0.0f)
					diffuseFactor = 0.0f;
				if(diffuseFactor > 1.0f)
					diffuseFactor = 1.0f;

				bool inShadow = false;

				if(numShadows)
					_sampleData.getIntersectionResult(firstShadow,inShadow);

				if(materialId < (int)_materials.size() && !inShadow )
				{
					//float dist = location.norm();
					//Vector3 color(barycentric.x(),barycentric.y(),0.0f);
					const Vector3& color = _materials[materialId]._color;
					_sampleData.setSampleIntensity(parentSample,color*diffuseFactor);
				}
				else
				{
					_sampleData.setSampleIntensity(parentSample,Vector3(0.0f,0.0f,0.0f));
				}
			}
			else
				_sampleData.setSampleIntensity(parentSample,Vector3(1.0f,1.0f,1.0f));

			count = 1;
			intersectionId = _sampleData.getNextIntersections(threadId,count);

		}
		_sampleData.threadCompleteShading(threadId,intersectionId);
	}

	SampleData& _sampleData;
	RayData& _rayData;

	struct TriData
	{
		int			_material;
		Vector3		_normal;
		Vector3		_location;
		Vector3		_baryUDir;
		Vector3		_baryVDir;
	};

	std::vector<TriData>	_triangles;

	struct MaterialSettings
	{
		Vector3	_color;
	};

	std::vector<MaterialSettings> _materials;
};

}
#endif