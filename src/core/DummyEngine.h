/********************************************************/
// FILE: DummyEngine.h
// DESCRIPTION: Raytracer Dummy Engine that does nothing
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

#ifndef RAYTRACE_DUMMY_ENGINE_GUARD
#define RAYTRACE_DUMMY_ENGINE_GUARD

#include <RaytraceCommon.h>
#include "Ray.h"
#include "EngineBase.h"
#include "SceneReader.h"

namespace Raytrace {

template<class _OutputImageRect,class _SampleData,class _RayData> struct DummySampleGeneratorEngine;
template<class _SampleData,class _RayData> struct DummyIntersectorEngine;
template<class _SampleData,class _RayData> struct DummyShaderEngine;
struct DummySampleData;
struct DummyRayData;

template<class _ImageRect> struct DummySampleGenerator
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef DummySampleGeneratorEngine<_ImageRect,_SampleData,_RayData> type;
	};

	DummySampleGenerator(const _ImageRect& rect) : _outputImageRect(rect) {}
	
	typedef _ImageRect OutputImageRect;
	typedef EngineMultiThreaded ThreadingMode;
	typedef EngineGenerator EngineType;

	OutputImageRect			_outputImageRect;
};

struct DummyIntersector
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef DummyIntersectorEngine<_SampleData,_RayData> type;
	};
	typedef DummyRayData ExternalData;
	typedef EngineMultiThreaded ThreadingMode;
	typedef EngineIntersector EngineType;
};

struct DummyShader
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef DummyShaderEngine<_SampleData,_RayData> type;
	};
	typedef DummySampleData ExternalData;
	typedef EngineMultiThreaded ThreadingMode;
	typedef EngineSampler EngineType;
};

template<class _OutputImageRect,class _SampleData,class _RayData> struct DummySampleGeneratorEngine
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef _OutputImageRect OutputImageRect;

	inline DummySampleGeneratorEngine(const DummySampleGenerator<_OutputImageRect>& shader,SampleData& samples,RayData& rayData) : _outputImageRect(shader._outputImageRect){}
	inline void prepare(int numThreads) {}
	inline int gatherOutput() 
	{
		Pixel<A8R8G8B8> c1;
		c1.SetRed(0xff);
		c1.SetBlue(0x00);
		c1.SetGreen(0x00);
		FillRectangle(_outputImageRect, c1);
		return 0;
	}
	inline void pushGenerate() {};
	inline int threadEnter(int iThreadId) {return 0;}

	OutputImageRect			_outputImageRect;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
template<class _SampleData,class _RayData> struct DummyIntersectorEngine
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;

	inline DummyIntersectorEngine(const DummyIntersector& shader,_SampleData& sampleData,RayData& rayData,const SceneReader& scene) {}
	inline void prepare(int numThreads) {}
	inline int threadEnter(int iThreadId) {return 0;}
};
template<class _SampleData,class _RayData> struct DummyShaderEngine
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;

	inline DummyShaderEngine(const DummyShader& shader,_SampleData& sampleData,RayData& rayData,const SceneReader& scene) {}
	inline void prepare(int numThreads) {}
	inline void threadEnterSubsample(int iThreadId) {return;}
	inline void threadEnterShade(int iThreadId) {return;}
};

struct DummySampleData
{
	inline DummySampleData() {}
	inline void preparePushingSamples() {}
	inline void prepare(int numThreads) {}
	inline bool emptyStack() const { return true;}
	inline void pushStack() {}
	inline void popStack() {}
	inline void popShaded() {}
	inline bool hasSamplesToShade() { return false; }
	inline bool hasSamplesToSubsample() { return false; }
	inline void completeShading() {}
	inline void completeSubsampling() {}
	inline void completePushingSamples() {}
	inline void prepareShading() {}
	inline void prepareSubsampling() {}
};

struct DummyRayData
{
	inline DummyRayData() {}
	inline void prepareAllocatingRays() {}
	inline void completeIntersecting() {}
	inline void completeAllocatingRays() {}
	inline void prepareIntersecting() {}
	inline bool hasRaysToIntersect() { return false; }
	inline void pushRay(const Ray& dir,int ResultThreadId,int ResultSampleId) {}
	inline void pushStack() {}
	inline void popStack() {}
	inline void prepare(int numThreads) {}
};

}
#endif