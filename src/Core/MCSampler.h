/********************************************************/
// FILE: SimpleGenerator.h
// DESCRIPTION: Simple Ray Generator
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

#ifndef RAYTRACE_SIMPLE_GENERATOR_GUARD
#define RAYTRACE_SIMPLE_GENERATOR_GUARD

#include <RaytraceCommon.h>
#include <Math/InterlockedFunctions.h>
#include "MathHelper.h"
#include "SampleData.h"
#include "PoissonDiscSampler.h"
#include "ISampler.h"
#include "SceneReader.h"

namespace Raytrace {

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
};

template<class _SampleData,class _SceneReader> struct MCSampler : public ISampler<_SampleData,_SceneReader>
{
	typedef _SampleData SampleData;
	typedef _SceneReader SceneReader;
	typedef MultisampleGenerator SamplerType;

#ifdef _DEBUG
	static const size_t MaxGeneratedBlocks = 16;
#else
	static const size_t MaxGeneratedBlocks = 32*1024;
#endif

	static const size_t SampleBlockSize = _SampleData::NumSamplesPerBlock;

	inline MCSampler()
	{
	}

	inline ~MCSampler()
	{
	}
	
	void InitializePrepareST(size_t numThreads,const _SceneReader& scene,_SampleData& sampleData)
	{
		_imageSize = scene->getResolution();
		
		_maxGenerateSamples = std::min<size_t>(MaxGeneratedBlocks*SampleBlockSize,_imageSize.x()*_imageSize.y());
		size_t size = _imageSize.x()*_imageSize.y();
		_finalImage.resize(size);
		
		_numGeneratedSamples = 0;
		_numCompletedSamples = 0;
		_numDrawnSamples = 0;
		_nextGenerateSamples = 0;

		_nextGenerateBlock = 0;

		_pixelSize = Vector2( 1.0f / (float) _imageSize.x(), 1.0f / (float)_imageSize.y());
		_sampler.Initialize(0.25f);

		_generateBarrier.reset( new boost::barrier((unsigned int)numThreads) );

		_sampleData= &sampleData;

		_sampleData->resizeSampleBuffer(_maxGenerateSamples);
		
		_threadData.resize(numThreads);

		_numDesiredSamples = _finalImage.size()*_sampler.count();
	}
	void InitializeMT(size_t threadId) 
	{
	}
	void InitializeCompleteST() 
	{
	}

	void GeneratePrepareST() 
	{

		up numDesiredSamples = _numDesiredSamples - _numGeneratedSamples;
		up numEffectiveSamples = std::min<up>(numDesiredSamples,_maxGenerateSamples);

		_nextGenerateSamples = numEffectiveSamples;

		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			it->_numGenerated = 0;
			it->_numCompleted = 0;
		}
	}
	void GenerateMT(size_t threadId) 
	{
		const Vector2u size(_imageSize);

		//read completed
		{
			auto it = _sampleData->beginCompleted(threadId);
			auto end = _sampleData->endCompleted(threadId);

			while(it != end)
			{
				Vector2i currentPixel((int)(it->getX()* (f32)size.x()), (int)(it->getY()* (f32)size.y()));
				size_t imageIndex = currentPixel.y() * size.x() + currentPixel.x();

				_finalImage[imageIndex].pushData(Vector3(it->getResult()));
				_threadData[threadId]._numCompleted++;

				it.deactivateSample();
			}
		}

		_generateBarrier->wait();

		//write new
		{
			auto it = _sampleData->beginInactive(threadId);
			auto end = _sampleData->endInactive(threadId);

			while(it != end)
			{
				const up block = InterlockedIncrement(_nextGenerateBlock) - 1;

				const up firstSample = std::max<up>(_numGeneratedSamples,block*SampleBlockSize);
				const up endSample = std::min<up>(_numGeneratedSamples+_nextGenerateSamples,(block+1)*SampleBlockSize);

				if(firstSample >= endSample)
					break;
				
				const up numSamples = endSample-firstSample;

				for(up i = firstSample; i < endSample && it != end; ++i)
				{
					size_t multisampleIndex = (size_t)(i  / (up)_finalImage.size());
					size_t imageIndex = (size_t)(i  % (up)_finalImage.size());
					//SampleIdType sampleIndex = (SampleIdType)(i - _numCompletedSamples);
				
					Vector2i currentPixel((u32)(imageIndex % size.x()), (u32)(imageIndex / size.x()));
					Vector2 current ( (float)(currentPixel.x()) * (float)_pixelSize.x(), (float)(currentPixel.y()) * (float)_pixelSize.y());
					current+=_pixelSize*.5f;

					Vector2 jitter = _sampler(multisampleIndex);

					Vector2 location = current + (jitter.array() * _pixelSize.array()).matrix() - _pixelSize*.5f;
					assert(location.x() >= 0.0f && location.x() <= 1.0f);
					assert(location.y() >= 0.0f && location.y() <= 1.0f);

					it->setX(location.x());
					it->setY(location.y());

					float x = location.x(),y = location.y(),z = (float)multisampleIndex / (float)_sampler.count();

					it->setResult( Vector3( x,y,z) );

					it.activateSample();
					_threadData[threadId]._numGenerated++;
				}
			}
		}
	}

	f32 GenerateCompleteST() 
	{
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			_numGeneratedSamples += it->_numGenerated;
			_numCompletedSamples += it->_numCompleted;
		}

		_nextGenerateBlock = _numGeneratedSamples/SampleBlockSize;
		return (float)_numCompletedSamples / (float)_numDesiredSamples;
	}

	template<IMAGE_FORMAT _Format> inline void Gather(ImageRect<_Format> out) const
	{	
		up completed = _numCompletedSamples;

		if(_numDrawnSamples >= completed)
			return;

		size_t range = (size_t)std::min<up>(completed - _numDrawnSamples,(up)_finalImage.size());

		size_t begin1,end1;
		size_t begin2,end2;

		begin1 = (size_t)(_numDrawnSamples%(up)_finalImage.size());
		end1 = begin1 + range;
		if(end1 > _finalImage.size())
		{
			begin2 = 0;
			end2 = end1 - _finalImage.size();
			end1 = _finalImage.size();
		}
		else
		{
			begin2 = end2 = end1;
		}

		_numDrawnSamples = completed;

		for(size_t i = begin1; i < end1; ++i)
		{
			out( (u32)(i % _imageSize.x()) , (u32)(i / _imageSize.x()) ) = Pixel<_Format>(Pixel<RGBA_FLOAT32>(
					std::min<f32>(1.0f,std::max<f32>(0.0f,_finalImage[i]._mean.x())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,_finalImage[i]._mean.y())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,_finalImage[i]._mean.z())),
					1.0f
				));
		}
		for(size_t i = begin2; i < end2; ++i)
		{
			out( (u32)(i % _imageSize.x()) , (u32)(i / _imageSize.x()) ) = Pixel<_Format>(Pixel<RGBA_FLOAT32>(
					std::min<f32>(1.0f,std::max<f32>(0.0f,_finalImage[i]._mean.x())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,_finalImage[i]._mean.y())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,_finalImage[i]._mean.z())),
					1.0f
				));
		}
	}

	Result GatherPreview(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const 
	{
		assert(xRes == _imageSize.x());
		assert(yRes == _imageSize.y());

		switch(format)
		{
		case A8R8G8B8:
			Gather(ImageRect<A8R8G8B8>(pDataOut,Vector2u((u32)xRes,(u32)yRes),(u32)xRes));
			return Result::Succeeded;
		case RGBA_FLOAT32:
			Gather(ImageRect<RGBA_FLOAT32>(pDataOut,Vector2u((u32)xRes,(u32)yRes),(u32)xRes));
			return Result::Succeeded;
		}
		return Result::Failed;
	}
	void GetImage(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const 
	{
		assert(xRes == _imageSize.x());
		assert(yRes == _imageSize.y());

		switch(format)
		{
		case A8R8G8B8:
			Gather(ImageRect<A8R8G8B8>(pDataOut,Vector2u((u32)xRes,(u32)yRes),(u32)xRes*sizeof(Pixel<A8R8G8B8>)));
			return;
		case RGBA_FLOAT32:
			Gather(ImageRect<RGBA_FLOAT32>(pDataOut,Vector2u((u32)xRes,(u32)yRes),(u32)xRes*sizeof(Pixel<RGBA_FLOAT32>)));
			return;
		}
	}

	struct FinalImageElement
	{
		Vector3				_mean;
		Vector3				_s;
		size_t				_numSamples;

		inline FinalImageElement() : _numSamples(0) {}

		inline void pushData(const Vector3& element)
		{
			if(_numSamples)
			{
				++_numSamples;
				Vector3 newMean = _mean + (element - _mean)/(Real)_numSamples;
				_s += ((element - _mean).array()/(element - newMean).array()).matrix();
				_mean = newMean;
			}
			else
			{
				_numSamples = 1;
				_mean = element;
				_s = Vector3(0.0f,0.0f,0.0f);
			}
		}

		inline Real Variance() const
		{
			Vector4 variance = (_numSamples > 1) ? (_s/(Real)(_numSamples - 1) ) : (Vector3(0.0f,0.0f,0.0f));
			return variance.x() + variance.y() + variance.z() + variance.w();
		}

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	struct ThreadData
	{
		size_t _numGenerated;
		size_t _numCompleted;
	};

	std::auto_ptr<boost::barrier>	_generateBarrier;

	Vector2u						_imageSize;
	std::vector<FinalImageElement>	_finalImage;
	std::vector<ThreadData>			_threadData;
	
	size_t					_maxGenerateSamples;

	size_t					_nextGenerateSamples;
	size_t					_numGeneratedSamples;
	size_t					_numCompletedSamples;
	size_t					_numDesiredSamples;
	mutable size_t			_numDrawnSamples;

	volatile up				_nextGenerateBlock;

	Vector2					_pixelSize;

	SamplerType				_sampler;

	SampleData*				_sampleData;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
#endif