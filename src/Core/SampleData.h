/********************************************************/
// FILE: SampleData.h
// DESCRIPTION: Data container for Samples
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

#ifndef RAYTRACE_SAMPLE_DATA_GUARD
#define RAYTRACE_SAMPLE_DATA_GUARD

#include <RaytraceCommon.h>
#include "SplitUseWorkQueue.h"

namespace Raytrace {
	


template<int _NumSamplesPerBlock,class _SampleValueType,class _SampleIndexType,class _SampleInput,class _SampleOutput> struct SimpleSampleData
{
public:
	
	static const size_t NumSamplesPerBlock = _NumSamplesPerBlock;

	typedef SimpleSampleData<_NumSamplesPerBlock,_SampleValueType,_SampleIndexType,_SampleInput,_SampleOutput> ThisType;
	typedef _SampleValueType SampleValueType;
	typedef Eigen::Matrix<SampleValueType,2,1> SampleValue2DType;
	typedef ISampleGenerator<ThisType>	ISamplerType;
	typedef _SampleIndexType SampleIndexType;
	typedef _SampleOutput SampleOutput;
	typedef _SampleInput SampleInput;


	static const SampleIndexType	SampleIndex2DImageXY	= 0;
	static const SampleIndexType	SampleIndex2DCameraUV	= 1;
	static const SampleIndexType	SampleIndex2DMisc		= 2;

	static const SampleIndexType	SampleIndexTimeT		= 0;
	static const SampleIndexType	SampleIndexMisc			= 1;

	
	typedef SplitUseWorkQueue<SampleInput,NumSamplesPerBlock> GeneratedSamplesArrayType;
	typedef SplitUseWorkQueue<SampleOutput,NumSamplesPerBlock> CompletedSamplesArrayType;

	inline void InitializeST(size_t numThreads)
	{
		_generatedSamples.prepare(numThreads);
		_completedSamples.prepare(numThreads);
	}
	
	
	inline void PrepareSampleST()
	{
		_generatedSamples.clear();
		_completedSamples.reset();
	}
	
	inline void PrepareIntegrateST()
	{
		assert(_sampler);
	}
	
	inline void CompleteSampleST()
	{
		_generatedSamples.reset();
		_completedSamples.clear();
	}
	
	inline void CompleteIntegrateST()
	{
	}

	inline size_t getNumActiveSamples() const
	{
		return _generatedSamples.size();
	}
	inline size_t getNumCompletedSamples() const
	{
		return _completedSamples.size();
	}

	inline void pushCompletedSample(size_t threadId,const SampleOutput& completed)
	{
		_completedSamples.pushElement(completed,threadId);
	}

	inline bool popCompletedSample(size_t threadId,SampleOutput& completed)
	{
		_completedSamples.advanceElement(threadId);
		SampleOutput* p_completed = _completedSamples.currElement(threadId);
		if(p_completed)
		{
			completed = *p_completed;
			return true;
		}
		else
			return false;
	}
	
	inline void pushGeneratedSample(size_t threadId,const SampleInput& generated)
	{
		_generatedSamples.pushElement(generated,threadId);
	}

	inline bool popGeneratedSample(size_t threadId,SampleInput& generated)
	{
		_generatedSamples.advanceElement(threadId);
		SampleInput* p_generated = _generatedSamples.currElement(threadId);
		if(p_generated)
		{
			generated = *p_generated;
			return true;
		}
		else
			return false;
	}

	inline SampleValue2DType getSampleValueImageXY(const SampleInput& sample,size_t thread)
	{
		return _sampler->getSampleLocation2D(sample,SampleIndex2DImageXY,thread);
	}

	inline SampleValue2DType getSampleValueCameraUV(const SampleInput& sample,size_t thread)
	{
		return _sampler->getSampleLocation2D(sample,SampleIndex2DCameraUV,thread);
	}
	
	inline SampleValue2DType getSampleValueMisc2D(SampleInput& sample,size_t thread)
	{
		++sample;
		return _sampler->getSampleLocation2D(sample,SampleIndex2DMisc + sample() -1,thread);
	}
	
	inline SampleValueType getSampleValueTimeT(const SampleInput& sample,size_t thread)
	{
		return _sampler->getSampleLocation(sample,SampleIndexTimeT,thread);
	}

	inline SampleValueType getSampleValueMisc(SampleInput& sample,size_t thread)
	{
		++sample;
		return _sampler->getSampleLocation(sample,SampleIndexMisc + sample() -1,thread);
	}

	inline void setSampleGenerator(ISamplerType* sampler)
	{
		_sampler = sampler;
	}

private:

	ISamplerType*	_sampler;
	
	GeneratedSamplesArrayType _generatedSamples;
	CompletedSamplesArrayType _completedSamples;
};

}

#endif