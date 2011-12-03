/********************************************************/
// FILE: EngineBase.h
// DESCRIPTION: Raytracer Engine
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

#ifndef RAYTRACE_ENGINE_BASE_GUARD
#define RAYTRACE_ENGINE_BASE_GUARD

#include <RaytraceCommon.h>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "IEngine.h"
#include "IIntersector.h"
#include "ISampler.h"
#include "IIntegrator.h"


namespace Raytrace {

/***************************************************************************************************************/
// Raytrace Engine:
/***************************************************************************************************************/
//		CameraRayGenerator is responsible for generating initial rays and gathering rays to generate an image
//		Intersector is responsible for determining locations where ray interact with scene objects
//		Shader is responsible for generating secondary rays and/or determine the color of intersections
/***************************************************************************************************************/
	
template<class _Callee>
struct threadStartFunctor;

template<
	class _RayData,
	class _SampleData, 
	class _SceneReader>
class BasicRaytraceEngine : public IEngine<_SceneReader>
{
public:

	template<class _Callee> friend struct threadStartFunctor;

	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef _SceneReader SceneReader;

	typedef IIntersector<RayData,SceneReader> IIntersector;
	typedef ISampler<SampleData,SceneReader> ISampler;
	typedef IIntegrator<SampleData,RayData,SceneReader> IIntegrator;

	typedef boost::shared_ptr<IIntersector> Intersector;
	typedef boost::shared_ptr<ISampler> Sampler;
	typedef boost::shared_ptr<IIntegrator> Integrator;

	typedef BasicRaytraceEngine<RayData,SampleData,SceneReader> ThisType;

	BasicRaytraceEngine(
		const Intersector& intersector,
		const Sampler& sampler,
		const Integrator& integrator,
		const SceneReader& sceneReader) ;

	~BasicRaytraceEngine();

	Result Initialize(const SceneReader& scene);

	Result Begin();
	Result Pause();
	Result Resume();

	Result GetStatus(const String& status_type,String& status_out) const;
	
	Result GatherPreview(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const;

	Result GetImage(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const;

	void AddRef();
	void Release();
	
private:

	enum MODE
	{
		IDLE = 0,
		STARTUP = 1,
		SAMPLE = 2,
		INTEGRATE = 3,
		INTERSECT = 4,
		COMPLETE = 5,
		OVERHEAD = 6
	};


	static const size_t NUM_MODES = 7;
	static const std::array<String,NUM_MODES> ModeName;// = { "Idle", "Startup", "Sample", "Integrate", "Intersect", "Complete", "Overhead" };

	void TerminateThreads();

	void ThreadEnter();

	struct ThreadData
	{
		boost::thread*	_threadPointer;
	};

	inline void setMode(MODE mode)
	{
		if(mode == _mode)
			return;

		u64 time = 	boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds();
		
		_timeByMode[_mode] += time - _beginTime;

		_beginTime = time;

		_mode = mode;
	}

	void EnterModeST(MODE mode);
	void EnterModeMT(MODE mode,size_t threadId);
	void LeaveModeST(MODE mode);
	MODE getNextMode(MODE prevMode);
	
	SampleData		_sampleData;
	RayData			_rayData;

	Sampler			_sampler;
	Intersector		_intersector;
	Integrator		_integrator;
	SceneReader		_sceneReader;

	float							_progress;

	// modified by worker thread leader, read by main thread
	MODE			_mode,_nextMode;

	// -1 for running, set to numThreads to begin terminating, when it reaches 0 terminated
	volatile i32					_threadTerminateCounter;
	volatile u32					_threadCompletionCounter;

	// constant while threads operating
	std::auto_ptr<boost::barrier>	_modeSwitchBarrier;
	std::auto_ptr<boost::barrier>	_startupBarrier;
	std::vector<ThreadData>			_threads;
	boost::thread_group				_threadGroup;
	i32								_numThreads;

	// timing related
	std::array<u64,NUM_MODES>		_timeByMode;
	u64								_beginTime;
	u64								_timeFactor;
	u64								_totalBeginTime;
	u64								_totalEndTime;

	// some statistics

	u64								_totalFirstHitRays;
	u64								_totalAnyHitRays;
	u64								_totalSamples;

	bool							_initComplete;

	// refcount!

	volatile u32					_refcount;

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}

#endif