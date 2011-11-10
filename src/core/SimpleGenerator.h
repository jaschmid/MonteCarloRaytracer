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
#include "EngineBase.h"
#include "MathHelper.h"

namespace Raytrace {

template<class _OutputImageRect,class _SampleData,class _RayData> struct SimpleSampleGeneratorEngine;

template<class _ImageRect> struct SimpleSampleGenerator
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef SimpleSampleGeneratorEngine<_ImageRect,_SampleData,_RayData> type;
	};

	SimpleSampleGenerator(const _ImageRect& rect) : _outputImageRect(rect) {}

	typedef _ImageRect OutputImageRect;
	typedef EngineSingleThreaded ThreadingMode;
	typedef EngineGenerator EngineType;

	OutputImageRect			_outputImageRect;
	Camera					_camera;
	Output					_output;
};

template<class _OutputImageRect,class _SampleData,class _RayData> struct SimpleSampleGeneratorEngine
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef _OutputImageRect OutputImageRect;

	inline SimpleSampleGeneratorEngine(const SimpleSampleGenerator<_OutputImageRect>& shader,SampleData& samples,RayData& rayData) : 
		_outputImageRect(shader._outputImageRect),
		_rays(rayData),_samples(samples),
		_maxGenerated(_outputImageRect.XSize()*_outputImageRect.YSize()),
		_numCompleted(0),_numDrawn(0)
	{
		_fov.x() = shader._camera->GetFov();
		_fov.y() = _fov.x() / shader._camera->GetAspect();

		Vector3 from = shader._camera->GetFrom();
		Vector3 to = shader._camera->GetTo();
		Vector3 up = (shader._camera->GetUp() - from).normalized();

		_viewMatrix = FromLookAt(from,to,up).inverse();
		_completedSamples.resize(_maxGenerated);
	}
	inline void prepare(int numThreads) 
	{
		_threadNumGeneratedSamples.resize(numThreads);
		for(auto it = _threadNumGeneratedSamples.begin(); it != _threadNumGeneratedSamples.end(); ++it)
			*it = it - _threadNumGeneratedSamples.begin();
	}
	inline void gatherOutput() 
	{
		if(_numDrawn >= _numCompleted)
			return;

		const Vector2u size(_outputImageRect.XSize(),_outputImageRect.YSize());
		_numDrawn = _numCompleted;
		for(int i = 0; i < _numDrawn; ++i)
		{
			Vector4 color;
			float contribution;
			_rays.getRayShadingInfo(_completedSamples[i],color,contribution);

			_outputImageRect( i % size.x() , i / size.x() ) = Pixel<RGBA_FLOAT32>(
					color.x()*contribution,
					color.y()*contribution,
					color.z()*contribution,
					color.w()*contribution
				);
		}

		return;
	}
	inline void threadEnter(int threadId) 
	{
		const Vector2u size(_outputImageRect.XSize(),_outputImageRect.YSize());
		const Vector2 angleStep(_fov.x()*.5f / (float)size.x(),_fov.y()*.5f / (float)size.y());
		int numSamples = 0;
		bool bExit = false;

		int sample = _threadNumGeneratedSamples[threadId];

		while(sample < _maxGenerated)
		{
			Vector2i current ( sample % size.x() - size.x()/2, sample / size.x() - size.y()/2);

			Vector2 rayAngle( angleStep.x() * current.x() , angleStep.y() * current.y() );
			Ray ray( Vector3(0.0f, 0.0f, 0.0f),Vector3(-sinf(rayAngle.x()), -sinf(rayAngle.y()), cosf(rayAngle.x())*cosf(rayAngle.y()) ).normalized() , -1.0f);
			ray *= _viewMatrix;
			int rayId = _rays.allocateRays(threadId,1);
			if(rayId == -1)
			{
				bExit = true;
				break;
			}
			_rays.setRayInitialData(rayId, ray , 1.0f);
			_completedSamples[sample] = rayId;
			sample += _threadNumGeneratedSamples.size();
		}

		_threadNumGeneratedSamples[threadId]=std::min<int>(sample,_maxGenerated);
	}

	inline void pushGenerate()
	{
		int min = _maxGenerated;
		for(auto it = _threadNumGeneratedSamples.begin(); it != _threadNumGeneratedSamples.end(); ++it)
			if(*it < min)
				min = *it;
		_numCompleted = min;
	}

	SampleData&				_samples;
	RayData&				_rays;
	OutputImageRect			_outputImageRect;
	std::vector<int>		_threadNumGeneratedSamples;
	std::vector<int>		_completedSamples;
	int						_numCompleted;
	int						_numDrawn;
	const int				_maxGenerated;
	Matrix4					_viewMatrix;
	Vector2					_fov;
};

}
#endif