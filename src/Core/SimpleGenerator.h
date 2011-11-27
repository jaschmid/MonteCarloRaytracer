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
#include "EngineBase.h"
#include "MathHelper.h"
#include "PoissonDiscSampler.h"

namespace Raytrace {

template<class _OutputImageRect,class _SampleData,class _RayData,class _Camera,class _Sampler> struct SimpleSampleGeneratorEngine;

struct FisheyeCamera
{
	FisheyeCamera(const Camera& camera)
	{
		_fov.x() = camera->GetFov();
		_fov.y() = _fov.x() / camera->GetAspect();

		_angleStep = Vector2(_fov.x()*.25f,_fov.y()*.25f);
	}

	inline Ray operator()(const Vector2& location)
	{
		Vector2 rayAngle( _angleStep.x() * location.x() , _angleStep.y() * location.y() );
		Ray ray( Vector3(0.0f, 0.0f, 0.0f),Vector3(-sinf(rayAngle.x()), -sinf(rayAngle.y()), cosf(rayAngle.x())*cosf(rayAngle.y()) ).normalized() , -1.0f);

		return ray;
	}

	Vector2					_fov;
	Vector2					_angleStep;
};

struct MultisampleGenerator
{
	MultisampleGenerator(f32 sampleSize) : _sampleSize(sampleSize)
	{
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

	const f32				_sampleSize;
	std::vector<Vector2>	_sampleLocations;
};

template<class _ImageRect> struct SimpleSampleGenerator
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef SimpleSampleGeneratorEngine<_ImageRect,_SampleData,_RayData,FisheyeCamera,MultisampleGenerator> type;
	};

	SimpleSampleGenerator(size_t xRes,size_t yRes) : _xResolution(xRes),_yResolution(yRes) {}

	typedef _ImageRect OutputImageRect;
	typedef EngineSingleThreaded ThreadingMode;
	typedef EngineGenerator EngineType;

	size_t					_xResolution;
	size_t					_yResolution;
	Camera					_camera;
	Output					_output;
};



#ifdef _DEBUG
static const int MAX_GENERATED_BLOCKS	= 256;
#else
static const int MAX_GENERATED_BLOCKS	= 4*1024;
#endif

template<class _OutputImageRect,class _SampleData,class _RayData,class _CameraType,class _Multisampler> struct SimpleSampleGeneratorEngine
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef _OutputImageRect OutputImageRect;
	typedef _CameraType CameraType;
	typedef _Multisampler SamplerType;

	typedef typename SampleData::SampleIdType SampleIdType;
	typedef typename RayData::RayIdType RayIdType;
	static const SampleIdType SampleBlockSize = (SampleIdType) SampleData::SampleBlockSize;

	inline SimpleSampleGeneratorEngine(const SimpleSampleGenerator<_OutputImageRect>& shader,SampleData& samples,RayData& rayData) : 
		_imageSize(shader._xResolution,shader._yResolution),
		_rays(rayData),_samples(samples),
		_maxGenerateSamples(std::min<SampleIdType>(MAX_GENERATED_BLOCKS*SampleBlockSize,_imageSize.x()*_imageSize.y())),
		_camera(shader._camera),
		_sampler(0.25f)
	{
		size_t size = _imageSize.x()*_imageSize.y();
		_completedSamples.resize(_maxGenerateSamples);
		_finalImage.resize(size);
		
		_numGeneratedSamples = 0;
		_numCompletedSamples = 0;
		_numDrawnSamples = 0;
		_numOutputSamples = 0;

		_nextGenerateBlock = 0;

		_pixelSize = Vector2( 1.0f / (float) _imageSize.x(), 1.0f / (float)_imageSize.y());
	}
	inline void prepare(int numThreads) 
	{
	}
	inline void gatherOutput(void* pOut,size_t nOut) 
	{

		_OutputImageRect out(pOut,Vector2u(_imageSize.x(),_imageSize.y()),(u32)(nOut/_imageSize.y()/sizeof(Pixel<R8G8B8A8>)));

		up drawn = _numDrawnSamples;

		if(_numOutputSamples >= drawn)
			return;

		size_t range = (size_t)std::min<up>(drawn - _numOutputSamples,(up)_finalImage.size());

		size_t begin1,end1;
		size_t begin2,end2;

		begin1 = (size_t)(_numOutputSamples%(up)_finalImage.size());
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

		_numOutputSamples = drawn;

		for(size_t i = begin1; i < end1; ++i)
		{
			out( (u32)(i % _imageSize.x()) , (u32)(i / _imageSize.x()) ) = Pixel<RGBA_FLOAT32>(
					_finalImage[i]._mean.x(),
					_finalImage[i]._mean.y(),
					_finalImage[i]._mean.z(),
					1.0f
				);
		}
		for(size_t i = begin2; i < end2; ++i)
		{
			out( (u32)(i % _imageSize.x()) , (u32)(i / _imageSize.x()) ) = Pixel<RGBA_FLOAT32>(
					_finalImage[i]._mean.x(),
					_finalImage[i]._mean.y(),
					_finalImage[i]._mean.z(),
					1.0f
				);
		}

		return;
	}
	inline void prepareGenerating()
	{
		up numDesiredSamples = _finalImage.size()*_sampler.count() - _numGeneratedSamples;
		up numEffectiveSamples = std::min<up>(numDesiredSamples,_maxGenerateSamples);

		_numGeneratedSamples += numEffectiveSamples;
	}
	inline void threadEnter(int threadId) 
	{
		const Vector2u size(_imageSize);

		while(true)
		{

			const up block = InterlockedIncrement(_nextGenerateBlock) - 1;

			const up firstSample = std::max<up>(_numCompletedSamples,block*SampleBlockSize);
			const up endSample = std::min<up>(_numGeneratedSamples,(block+1)*SampleBlockSize);

			if(firstSample >= endSample)
				break;

			const up numSamples = endSample-firstSample;

			for(up i = firstSample; i < endSample; ++i)
			{
				size_t multisampleIndex = (size_t)(i  / (up)_finalImage.size());
				size_t imageIndex = (size_t)(i  % (up)_finalImage.size());
				SampleIdType sampleIndex = (SampleIdType)(i - _numCompletedSamples);
				
				Vector2i currentPixel(imageIndex % size.x(), imageIndex / size.x());
				Vector2 current ( (float)(currentPixel.x()) / (float)size.x(), (float)(currentPixel.y()) / (float)size.y());
				current*=2.0f;
				current-=Vector2(1.0f,1.0f);

				Vector2 jitter = (_sampler(multisampleIndex).array() * _pixelSize.array()).matrix()*2.0f - _pixelSize*.5f;

				Ray ray = _camera( current + jitter );

				SampleIdType sample = _samples.allocateSamples(threadId, 1);
				RayIdType rayId = _rays.allocateRays<RayData::FirstHitRay>(threadId,1);

				_rays.setRayInitialData<RayData::FirstHitRay>(rayId, ray, sample);

				_completedSamples[sampleIndex] = sample;
			}
		}

	}

	inline void completeGenerating()
	{
		_numDrawnSamples = _numCompletedSamples;
		_numCompletedSamples = _numGeneratedSamples;
		_nextGenerateBlock = _numCompletedSamples/SampleBlockSize;
	}

	inline void pushCompleted()
	{
		
		for(up i = 0; i < (_numCompletedSamples-_numDrawnSamples) ; ++i)
		{
			SampleIdType sample = _completedSamples[i];
			size_t imageLoc = (size_t)((i + _numDrawnSamples) % (up)_finalImage.size());
			Vector4 data;
			_samples.getSampleInfo(sample, data);
			_finalImage[imageLoc].pushData(data);
		}

	}

	SampleData&				_samples;
	RayData&				_rays;
	
	struct FinalImageElement
	{
		Vector4				_mean;
		Vector4				_s;
		SampleIdType		_numSamples;

		inline FinalImageElement() : _numSamples(0) {}

		inline void pushData(const Vector4& element)
		{
			if(_numSamples)
			{
				++_numSamples;
				Vector4 newMean = _mean + (element - _mean)/(Real)_numSamples;
				_s += ((element - _mean).array()/(element - newMean).array()).matrix();
				_mean = newMean;
			}
			else
			{
				_numSamples = 1;
				_mean = element;
				_s = Vector4(0.0f,0.0f,0.0f,0.0f);
			}
		}

		inline Real Variance() const
		{
			Vector4 variance = (_numSamples > 1) ? (_s/(Real)(_numSamples - 1) ) : (Vector4(0.0f,0.0f,0.0f,0.0f));
			return variance.x() + variance.y() + variance.z() + variance.w();
		}

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	inline Ray GetCameraRay(const Vector2i& imageLocation) const
	{
	}

	std::vector<FinalImageElement>	_finalImage;
	

	const Vector2u					_imageSize;
	std::vector<SampleIdType>		_completedSamples;
	
	up						_numGeneratedSamples;
	up						_numCompletedSamples;
	up						_numDrawnSamples;
	up						_numOutputSamples;

	up						_multisampleCount;

	const SampleIdType		_maxGenerateSamples;

	Vector2					_pixelSize;

	volatile up				_nextGenerateBlock;

	CameraType				_camera;
	SamplerType				_sampler;
};

}
#endif