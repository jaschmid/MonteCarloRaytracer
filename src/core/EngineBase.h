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


namespace Raytrace {

//raytrace engine base class, some common features for all raytrace engines to increase code reuse

class RaytraceEngineBase
{
public:

	RaytraceEngineBase();

	virtual ~RaytraceEngineBase() ;

	//sync calls, updates the output surface
	Result Gather();

	//async, begins generation of a frame, ignored if engine currently running
	Result Begin(int numThreads = 0);

	Result GetLastFrameTime(int& time);

protected:

	void TerminateThreads();

	void ThreadEnter();

	friend struct threadStartFunctor;
	struct threadStartFunctor
	{
		void operator()()
		{
			_startupBarrier->wait();
			return _parent->ThreadEnter();
		}
		RaytraceEngineBase*		_parent;
		boost::shared_ptr<boost::barrier>			_startupBarrier;
	};

	// prepare internal engines for number of threads
	virtual void Prepare(int numThreads) = 0;

	// generates a final image, no longer has to check that startup has passed
	virtual int GatherInternal() = 0;

	//returns number of rays generated
	virtual int ThreadEnterStartup(int iThread) = 0;

	//returns number of samples generated
	virtual int ThreadEnterIntersect(int iThread) = 0;

	//returns number of rays generated
	virtual int ThreadEnterShade(int iThread) = 0;

	enum mode
	{
		IDLE,
		STARTUP,
		SHADER,
		INTERSECT,
		COMPLETE
	};

	struct ThreadData
	{
		boost::thread*	_threadPointer;
		bool			_idle;
	};

	// constant while threads operating
	std::auto_ptr<boost::barrier>	_modeSwitchBarrier;
	std::vector<ThreadData>			_threads;
	boost::thread_group				_threadGroup;
	int								_numThreads;

	::Math::u64						_beginTime;
	::Math::u64						_endTime;
	::Math::u64						_timeFactor;

	// modified by main thread, read by all worker threads
	bool							_bTerminate;

	// modified by worker thread leader, read by main thread
	mode						_mode;

};

/***************************************************************************************************************/
// Raytrace Engine:
/***************************************************************************************************************/
//		CameraRayGenerator is responsible for generating initial rays and gathering rays to generate an image
//		Intersector is responsible for determining locations where ray interact with scene objects
//		Shader is responsible for generating secondary rays and/or determine the color of intersections
/***************************************************************************************************************/

template<
	class _SampleGenerator,
	class _Intersector, 
	class _Shader>
class RaytraceEngine : public RaytraceEngineBase
{
public:

	typedef typename _Shader::ExternalData SampleData;
	typedef typename _Intersector::ExternalData RayData;
	
	typedef typename _SampleGenerator::Engine<SampleData,RayData>::type SampleGenerator;
	typedef typename _Intersector::Engine<SampleData,RayData>::type Intersector;
	typedef typename _Shader::Engine<SampleData,RayData>::type Shader;

	RaytraceEngine(
		const _SampleGenerator& sampleGenerator,
		const _Intersector& intersector,
		const _Shader& shader) :
			_sampleData(shader),
			_rayData(intersector),
			_sampleGenerator(sampleGenerator,_sampleData,_rayData),
			_intersector(intersector,_sampleData,_rayData),
			_shader(shader,_sampleData,_rayData)
		{
		}

	~RaytraceEngine()
	{
		TerminateThreads();
	}
	
private:

	void Prepare(int numThreads)
	{
		_sampleData.prepare(numThreads);
		_rayData.prepare(numThreads);

		_sampleGenerator.prepare(numThreads);
		_intersector.prepare(numThreads);
		_shader.prepare(numThreads);
	}

	int GatherInternal()
	{
		return _sampleGenerator.gatherOutput();
	}

	int ThreadEnterStartup(int iThread)
	{
		return _sampleGenerator.threadEnter(iThread);
	}

	int ThreadEnterIntersect(int iThread)
	{
		return _intersector.threadEnter(iThread);
	}

	int ThreadEnterShade(int iThread)
	{
		return _shader.threadEnter(iThread);
	}
	
	SampleData		_sampleData;
	RayData			_rayData;

	SampleGenerator	_sampleGenerator;
	Intersector		_intersector;
	Shader			_shader;
};

template<class _Generator,class _Intersector,class _Sampler> 
	inline boost::shared_ptr<RaytraceEngineBase> CreateRaytraceEngine(
		const _Generator& sampleGenerator,
		const _Intersector& intersector,
		const _Sampler& shader)
	{
		return boost::shared_ptr<RaytraceEngineBase>(new RaytraceEngine<_Generator,_Intersector,_Sampler>(sampleGenerator,intersector,shader));
	}

}

#endif