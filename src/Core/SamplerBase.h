/********************************************************/
// FILE: SamplerBase.h
// DESCRIPTION: Some common sampler functions
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

#ifndef RAYTRACE_SAMPLER_BASE_GUARD
#define RAYTRACE_SAMPLER_BASE_GUARD

#include <RaytraceCommon.h>
#include "InterlockedFunctions.h"
#include "MathHelper.h"
#include "SampleData.h"
#include "ISampler.h"
#include "SceneReader.h"


namespace Raytrace {

template<class _SampleData,class _SceneReader> struct SamplerBase : public ISampler<_SampleData,_SceneReader>
{
	typedef _SampleData SampleData;
	typedef _SceneReader SceneReader;

#ifdef _DEBUG
	static const size_t MaxGeneratedSamples = 16*_SampleData::NumSamplesPerBlock;
#else
	static const size_t MaxGeneratedSamples = 32*1024*_SampleData::NumSamplesPerBlock;
#endif
	
	void InitializeSampler(size_t numThreads,const _SceneReader& scene,_SampleData& sampleData,size_t multisampleCount)
	{
		_imageSize = scene->getResolution();
		
		_maxGenerateSamples = std::min<size_t>(MaxGeneratedSamples,_imageSize.x()*_imageSize.y());
		size_t size = _imageSize.x()*_imageSize.y();
		_finalImage.resize(size);
		
		_numGeneratedSamples = 0;
		_numCompletedSamples = 0;
		_numDrawnSamples = 0;
		_nextGenerateSamples = 0;

		_nextGenerateBlock = 0;

		_pixelSize = Vector2( 1.0f / (float) _imageSize.x(), 1.0f / (float)_imageSize.y());

		_generateBarrier.reset( new boost::barrier((unsigned int)numThreads) );

		_sampleData= &sampleData;
				
		_threadStats.resize(numThreads);

		_numDesiredSamples = _finalImage.size()*multisampleCount;
	}

	virtual void InitializeMT(size_t threadId) 
	{
	}
	virtual void InitializeCompleteST() 
	{
	}

	virtual void GeneratePrepareST() 
	{

		up numDesiredSamples = _numDesiredSamples - _numGeneratedSamples;
		up numEffectiveSamples = std::min<up>(numDesiredSamples,_maxGenerateSamples);

		_nextGenerateSamples = numEffectiveSamples;

		for(auto it = _threadStats.begin(); it != _threadStats.end(); ++it)
		{
			it->_numGenerated = 0;
			it->_numCompleted = 0;
		}
	}

	virtual void GenerateMT(size_t threadId) 
	{
		const Vector2u size(_imageSize);

		//read completed
		typename SampleData::SampleOutput completed;

		while(_sampleData->popCompletedSample(threadId,completed))
		{
			size_t imageIndex = completed._index % _finalImage.size();

			_finalImage[imageIndex].pushData(completed._result);
			_threadStats[threadId]._numCompleted++;
		}

		//write new

		while(true)
		{
			const up block = InterlockedIncrement(_nextGenerateBlock) - 1;

			const up firstSample = std::max<up>(_numGeneratedSamples,block*_SampleData::NumSamplesPerBlock);
			const up endSample = std::min<up>(_numGeneratedSamples+_nextGenerateSamples,(block+1)*_SampleData::NumSamplesPerBlock);

			if(firstSample >= endSample)
				break;
				

			typename SampleData::SampleInput generated;
			const up numSamples = endSample-firstSample;

			for(up i = firstSample; i < endSample; ++i)
			{
				generated._index = i;
				_sampleData->pushGeneratedSample(threadId,generated);
				_threadStats[threadId]._numGenerated++;
			}
		}
	}

	virtual f32 GenerateCompleteST() 
	{
		for(auto it = _threadStats.begin(); it != _threadStats.end(); ++it)
			_numCompletedSamples += it->_numCompleted;

		//assert(_numCompletedSamples == _numGeneratedSamples);

		for(auto it = _threadStats.begin(); it != _threadStats.end(); ++it)
			_numGeneratedSamples += it->_numGenerated;

		_nextGenerateBlock = _numGeneratedSamples/_SampleData::NumSamplesPerBlock;
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
			Vector4 color = _finalImage[i].Mean();

			color.head<3>() = (color.head<3>().array() / (color.head<3>().array() + Vector3(1.0f,1.0f,1.0f).array())).matrix();

			out( (u32)(i % _imageSize.x()) , (u32)(i / _imageSize.x()) ) = Pixel<_Format>(Pixel<RGBA_FLOAT32>(
					std::min<f32>(1.0f,std::max<f32>(0.0f,color.x())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,color.y())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,color.z())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,color.w()))
				));
		}
		for(size_t i = begin2; i < end2; ++i)
		{
			Vector4 color = _finalImage[i].Mean();

			color.head<3>() = (color.head<3>().array() / (color.head<3>().array() + Vector3(1.0f,1.0f,1.0f).array())).matrix();

			out( (u32)(i % _imageSize.x()) , (u32)(i / _imageSize.x()) ) = Pixel<_Format>(Pixel<RGBA_FLOAT32>(
					std::min<f32>(1.0f,std::max<f32>(0.0f,color.x())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,color.y())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,color.z())),
					std::min<f32>(1.0f,std::max<f32>(0.0f,color.w()))
				));
		}
	}
	
	template<> inline void Gather<RGBA_FLOAT32>(ImageRect<RGBA_FLOAT32> out) const
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
			Vector4 color = _finalImage[i].Mean();
			color.w() = std::min<f32>(1.0f,std::max<f32>(0.0f,color.w()));
			out( (u32)(i % _imageSize.x()) , (u32)(i / _imageSize.x()) ) = Pixel<RGBA_FLOAT32>(color);
		}
		for(size_t i = begin2; i < end2; ++i)
		{
			Vector4 color = _finalImage[i].Mean();
			color.w() = std::min<f32>(1.0f,std::max<f32>(0.0f,color.w()));
			out( (u32)(i % _imageSize.x()) , (u32)(i / _imageSize.x()) ) = Pixel<RGBA_FLOAT32>(color);
		}
	}

	virtual Result GatherPreview(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const 
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
	virtual void GetImage(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const 
	{
		GatherPreview(format,xRes,yRes,pDataOut);
	}

	struct FinalImageElement
	{
		
		Vector4				_mean;
		Vector4				_s;
		size_t				_numSamples;

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
			Vector4 variance = (_numSamples > 1) ? Vector4(_s/(Real)(_numSamples - 1) ) : Vector4(0.0f,0.0f,0.0f,0.0f);
			return variance.x() + variance.y() + variance.z() + variance.w();
		}

		inline Vector4 Mean() const
		{
			return _mean;
		}

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	struct ThreadStats
	{
		size_t						_numGenerated;
		size_t						_numCompleted;
	};

	std::auto_ptr<boost::barrier>	_generateBarrier;

	Vector2u						_imageSize;
	std::vector<FinalImageElement>	_finalImage;
	std::vector<ThreadStats>			_threadStats;
	
	size_t					_maxGenerateSamples;

	size_t					_nextGenerateSamples;
	size_t					_numGeneratedSamples;
	size_t					_numCompletedSamples;
	size_t					_numDesiredSamples;
	mutable size_t			_numDrawnSamples;

	volatile up				_nextGenerateBlock;

	Vector2					_pixelSize;

	SampleData*				_sampleData;
};

}
#endif