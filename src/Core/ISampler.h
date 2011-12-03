/********************************************************/
// FILE: ISampler.h
// DESCRIPTION: Secifies interface for all Samplers (note it's still a template)
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

#ifndef RAYTRACE_ISAMPLER_GUARD
#define RAYTRACE_ISAMPLER_GUARD

#include <RaytraceCommon.h>

namespace Raytrace
{
	template<class _SampleData> struct ISampleGenerator
	{
		virtual typename _SampleData::SampleValue2DType getSampleLocation2D(const typename _SampleData::SampleInput& sample,typename _SampleData::SampleIndexType random_index,size_t threadId) 
		{
			return typename _SampleData::SampleValue2DType();
		}
		virtual typename _SampleData::SampleValueType getSampleLocation(const typename _SampleData::SampleInput& sample,typename _SampleData::SampleIndexType random_index,size_t threadId) 
		{
			return typename _SampleData::SampleValueType();
		}
	};

	template<class _SampleData,class _SceneReader> struct ISampler : public ISampleGenerator<_SampleData>
	{
		virtual void InitializePrepareST(size_t numThreads,const _SceneReader& scene,_SampleData& sampleData) {}
		virtual void InitializeMT(size_t threadId) {}
		virtual void InitializeCompleteST() {}

		virtual void GeneratePrepareST() {}
		virtual void GenerateMT(size_t threadId) {}
		virtual f32 GenerateCompleteST() {return 1.0f;}

		virtual Result GatherPreview(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const {return Result::NotImplemented;}

		virtual void GetImage(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const {}
		
		virtual ~ISampler() {}
	};
}

#endif