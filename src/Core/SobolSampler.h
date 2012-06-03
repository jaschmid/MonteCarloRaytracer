/********************************************************/
// FILE: SobolSampler.h
// DESCRIPTION: Sampler based on Low Discrepancy Sobol sequences
// AUTHOR: Jan Schmid (jaschmid@eml.cc)    
/********************************************************/
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc/3.0/ or send 
// a letter to Creative Commons, 444 Castro Street, 
// Suite 900, Mountain View, California, 94041, USA.
/********************************************************/


#if defined(_VSC_VER) && (_VSC_VER >= 1020)
#pragma once
#endif

#ifndef RAYTRACE_SOBOL_SAMPLER_GUARD
#define RAYTRACE_SOBOL_SAMPLER_GUARD

#include <RaytraceCommon.h>
#include "SamplerBase.h"
#include "PoissonDiscSampler.h"
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <strstream>

extern const char* __data_sobolBasis[];

namespace Raytrace {

template<class _SampleData,class _SceneReader> struct SobolSampler : public SamplerBase<_SampleData,_SceneReader>
{
	typedef _SampleData SampleData;
	typedef _SceneReader SceneReader;

	// dimensions after this will return random monte carlo numbers
	const static size_t NumDimensions = 512;
	const static size_t MaxBits	= 16;
	
	const static size_t NumJitterValues = 1024;

	struct SobolSequence;
	typedef SamplerBase<_SampleData,_SceneReader> Base;


	inline SobolSampler()
	{
	}

	inline ~SobolSampler()
	{
	}
	
	void InitializePrepareST(size_t numThreads,const _SceneReader& scene,_SampleData& sampleData)
	{
		InitializeSampler(numThreads,scene,sampleData,scene->getMultisampleCount());

		LoadSobolMatrices();

		_sampleData->setSampleGenerator(this);
				
		_threadData.resize(numThreads);

		_lastSequenceIdx = 0;
		_currentSequenceIdx = 1;
		_sampleIdx = 1;
		GenerateInitialSobolSequence(_sequences[_lastSequenceIdx]);
		NextSobolSequence(_sequences[_lastSequenceIdx],_sequences[_currentSequenceIdx]);
		/*
		while(true)
		{
			NextSobolSequence(_sequences[_lastSequenceIdx],_sequences[_currentSequenceIdx]);
			size_t temp = _lastSequenceIdx;
			_lastSequenceIdx = _currentSequenceIdx;
			_currentSequenceIdx = temp;
		}*/
		
		boost::random::mt11213b							randomGenerator; 
		boost::random::uniform_int_distribution<u32>	randomUniformDistribution;

		for(size_t i = 0; i < NumJitterValues; ++i)
			_jitters[i] = randomUniformDistribution(randomGenerator);
	}
	
	f32 GenerateCompleteST() 
	{
		f32 stat = Base::GenerateCompleteST();

		if(_numGeneratedSamples/_finalImage.size() > _sampleIdx)
		{
			//generate next idx
			_sampleIdx++;
			size_t temp = _lastSequenceIdx;
			_lastSequenceIdx = _currentSequenceIdx;
			_currentSequenceIdx = temp;
			NextSobolSequence(_sequences[_lastSequenceIdx],_sequences[_currentSequenceIdx]);
		}
		assert(_numGeneratedSamples/_finalImage.size() <= _sampleIdx);

		return stat;
	}

	static inline Real frac(Real in)
	{
		Real dummy;
		return modf(in,&dummy);
	}

	inline float mixup(u32 baseRandom,u32 mixin)
	{
		/*
		u32 jitter1 = _jitters[mixin%NumJitterValues];
		u32 jitter2 = _jitters[(mixin/NumJitterValues)%NumJitterValues];
		
		
		jitter1 = ( jitter1 << 16) | ( jitter1 >> 16);
		jitter1 = ((jitter1 & 0x00ff00ff) << 8)	| ((jitter1 & 0xff00ff00) >> 8);
		jitter1 = ((jitter1 & 0x0f0f0f0f) << 4)	| ((jitter1 & 0xf0f0f0f0) >> 4);
		jitter1 = ((jitter1 & 0x33333333) << 2)	| ((jitter1 & 0xcccccccc) >> 2);
		jitter1 = ((jitter1 & 0x55555555) << 1)	| ((jitter1 & 0xaaaaaaaa) >> 1);
		
		baseRandom ^= jitter1;
		baseRandom ^= jitter2;*/

		for(u32 v = 1<<31; mixin; mixin >>= 1, v |= v>>1)
			if(mixin & 1)
				baseRandom ^= v;
		

		Real result =  frac( (Real)((double) baseRandom / (double) 0x100000000LL));

		assert(result >= 0.0f && result < 1.0f);
		return result;
	}

	inline Vector2 getRandomValue2D(size_t sampleIndex,size_t randomIndex)
	{
		return Vector2(getRandomValue(sampleIndex,NumDimensions/2 + (randomIndex*2)+0),getRandomValue(sampleIndex,NumDimensions/2 + (randomIndex*2)+1));
	}

	inline static u32 hash( u32 a)
	{
		a = (a ^ 61) ^ (a >> 16);
		a = a + (a << 3);
		a = a ^ (a >> 4);
		a = a * 0x27d4eb2d;
		a = a ^ (a >> 15);
		return a;
	}

	inline Real getRandomValue(size_t sampleIndex,size_t randomIndex)
	{
		size_t sequenceIndex = (u32)(sampleIndex / _finalImage.size());
		if(sequenceIndex == _sampleIdx)
			sequenceIndex = _currentSequenceIdx;
		else
		{
			assert(sequenceIndex == _sampleIdx -1);
			sequenceIndex = _lastSequenceIdx;
		}
		sampleIndex %= _finalImage.size();
		u32 jitter1=_jitters[hash((u32)sampleIndex)%NumJitterValues];
		
		u32 dimension_index = (u32)randomIndex;
		
		for(u32 v = 1<<31; jitter1; jitter1 >>= 1, v |= v>>1)
			if(jitter1 & 1)
				dimension_index ^= v;

		u32 jitter2=_jitters[hash( (u32)(sampleIndex+randomIndex))%NumJitterValues];

		return mixup(_sequences[sequenceIndex]._X[ dimension_index %NumDimensions], jitter2 );
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

			Vector2 jitter = getRandomValue2D(sample._index,random_index);

			Vector2 location = current + (jitter.array() * _pixelSize.array()).matrix() + _pixelSize*.5f;

			return location;
		}
		else
		{
			/*
			return Vector2(	_threadData[threadId]._randomUniformDistribution(_threadData[threadId]._randomGenerator),
					_threadData[threadId]._randomUniformDistribution(_threadData[threadId]._randomGenerator));*/
			return getRandomValue2D(sample._index,random_index);
		}
	}

	typename SampleData::SampleValueType getSampleLocation(const typename SampleData::SampleInput& sample,typename SampleData::SampleIndexType random_index,size_t threadId)
	{
		return getRandomValue(sample._index,random_index);
	}

	inline void GenerateInitialSobolSequence(SobolSequence& sequence)
	{
		boost::random::uniform_int_distribution<u32>	randomUniformDistribution;
		boost::random::mt11213b					randomGenerator; 

		sequence._i = 0;
		sequence._C = 1;

		for(size_t iD = 0; iD < NumDimensions; iD++)
		{
			sequence._X[iD] = 0;//randomUniformDistribution(randomGenerator);
		}
	}

	inline void NextSobolSequence(const SobolSequence& sequenceIn,SobolSequence& sequence)
	{
		sequence._i = sequenceIn._i +1;

		// generate C

		sequence._C = 1;
		u32 val = sequence._i;
		while(val & 1)
		{
			val >>= 1;
			sequence._C++;
		}

		// generate X's and values

		for(size_t iD = 0; iD < NumDimensions; iD++)
		{
			/*
			u32 i = sequence._i;
			sequence._X[iD] = 0;

			for(u32 j = 0; i; i >>= 1, j++)
				if(i & 1)
					sequence._X[iD] ^= _sobolMatrices[iD]._V[j];
					*/
			sequence._X[iD] = sequenceIn._X[iD] ^ _sobolMatrices[iD]._V[sequenceIn._C];
		}
	}

	inline void LoadSobolMatrices()
	{

		_sobolMatrices[0]._d = 0; //?
		_sobolMatrices[0]._s = 0; //?
		_sobolMatrices[0]._a = 0; //?
		for(int iB = 0; iB < MaxBits; iB++)
			_sobolMatrices[0]._V[iB] = 1 << (32 - iB);

		for(int iD = 1; iD < NumDimensions; iD++)
		{
			std::istrstream infile(__data_sobolBasis[iD-1]);
			infile >> _sobolMatrices[iD]._d >> _sobolMatrices[iD]._s >> _sobolMatrices[iD]._a;
			size_t iB = 1;
			//load basic dimensions
			for(; iB <= _sobolMatrices[iD]._s; iB++)
			{
				u32 m;
				infile >> m;
				_sobolMatrices[iD]._V[iB] = m << (32 - iB);
			}
			//generate additional dimensions that might be lacking
			for(; iB < MaxBits+1; iB++)
			{
				size_t i_Vin_s = iB - _sobolMatrices[iD]._s;
				_sobolMatrices[iD]._V[iB] = _sobolMatrices[iD]._V[i_Vin_s] ^ (_sobolMatrices[iD]._V[i_Vin_s] >> _sobolMatrices[iD]._s);
				
				for(size_t k = 1; k < _sobolMatrices[iD]._s; ++k)
					_sobolMatrices[iD]._V[iB] ^= ((( _sobolMatrices[iD]._a >> (_sobolMatrices[iD]._s-1-k)) & 1) * _sobolMatrices[iD]._V[iB-k]); 
			}
		}
	}

	struct ThreadData
	{
		boost::random::uniform_01<Real,Real>	_randomUniformDistribution;
		boost::random::mt11213b					_randomGenerator;  
	};

	
	struct SobolMatrix
	{
		u32 _d;
		u32 _s;
		u32 _a;
		std::array<u32,MaxBits+1>	_V;
	};

	struct SobolSequence
	{
		u32								_i;
		u32								_C;
		std::array<u32,NumDimensions>	_X;
		std::array<Real,NumDimensions>	_value;
	};

	std::array<SobolMatrix,NumDimensions>	_sobolMatrices;
	std::array<SobolSequence,2>				_sequences;
	std::array<u32,NumJitterValues>			_jitters;

	size_t									_sampleIdx;

	size_t									_lastSequenceIdx;
	size_t									_currentSequenceIdx;

	std::vector<ThreadData>			_threadData;
};

}
#endif