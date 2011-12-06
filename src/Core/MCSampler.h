/********************************************************/
// FILE: MCSampler.h
// DESCRIPTION: Purely random Monte Carlo Sampler
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

#ifndef RAYTRACE_MC_SAMPLER_GUARD
#define RAYTRACE_MC_SAMPLER_GUARD

#include <RaytraceCommon.h>
#include "SamplerBase.h"
//#include "PoissonDiscSampler.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>


namespace Raytrace {
	/*
struct MultisampleGenerator
{
	MultisampleGenerator()
	{
	}

	inline void Initialize(f32 sampleSize)
	{
		_sampleSize = sampleSize;
		FastPoissonDiskSampling(sampleSize/2.0f,_sampleLocations);
	}

	inline size_t count() const
	{
		return _sampleLocations.size();
	}

	inline Vector2 operator()(size_t sample)
	{
		return _sampleLocations[sample];
	}

	f32						_sampleSize;
	std::vector<Vector2>	_sampleLocations;
};*/

template<class _SampleData,class _SceneReader> struct MCSampler : public SamplerBase<_SampleData,_SceneReader>
{
	typedef _SampleData SampleData;
	typedef _SceneReader SceneReader;
	//typedef MultisampleGenerator SamplerType;

	inline MCSampler()
	{
	}

	inline ~MCSampler()
	{
	}
	
	void InitializePrepareST(size_t numThreads,const _SceneReader& scene,_SampleData& sampleData)
	{
		//_sampler.Initialize(1.0f/32.0f);

		InitializeSampler(numThreads,scene,sampleData,64);

		_sampleData->setSampleGenerator(this);
				
		_threadData.resize(numThreads);
	}
	
	typename SampleData::SampleValue2DType getSampleLocation2D(const typename SampleData::SampleInput& sample,typename SampleData::SampleIndexType random_index,size_t threadId)
	{
		if(random_index == SampleData::SampleIndex2DImageXY)
		{
			size_t multisampleIndex = (size_t)(sample._index  / (up)_finalImage.size());
			size_t imageIndex = (size_t)(sample._index  % (up)_finalImage.size());
			//SampleIdType sampleIndex = (SampleIdType)(i - _numCompletedSamples);
				
			Vector2i currentPixel((u32)(imageIndex % _imageSize.x()), (u32)(imageIndex / _imageSize.x()));
			Vector2 current ( (float)(currentPixel.x()) * (float)_pixelSize.x(), (float)(currentPixel.y()) * (float)_pixelSize.y());
			current+=_pixelSize*.5f;
			
			Vector2 jitter = Vector2(	_threadData[threadId]._randomUniformDistribution(_threadData[threadId]._randomGenerator),
					_threadData[threadId]._randomUniformDistribution(_threadData[threadId]._randomGenerator));

			Vector2 location = current + (jitter.array() * _pixelSize.array()).matrix() - _pixelSize*.5f;

			return location;
		}
		else
		{
			return Vector2(	_threadData[threadId]._randomUniformDistribution(_threadData[threadId]._randomGenerator),
					_threadData[threadId]._randomUniformDistribution(_threadData[threadId]._randomGenerator));
		}
	}

	typename SampleData::SampleValueType getSampleLocation(const typename SampleData::SampleInput& sample,typename SampleData::SampleIndexType random_index,size_t threadId)
	{
		return _threadData[threadId]._randomUniformDistribution(_threadData[threadId]._randomGenerator);
	}

	struct ThreadData
	{
		boost::random::uniform_01<Real,Real>	_randomUniformDistribution;
		boost::random::mt11213b					_randomGenerator;  
	};

	std::vector<ThreadData>			_threadData;
	//SamplerType						_sampler;
};

}
#endif