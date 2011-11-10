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
#include "Ray.h"
#include "SceneReader.h"
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
		inline void operator()()
		{
			_startupBarrier->wait();
			return _parent->ThreadEnter();
		}
		RaytraceEngineBase*		_parent;
		boost::shared_ptr<boost::barrier>			_startupBarrier;
	};
	
	enum MODE
	{
		IDLE,
		STARTUP,
		SUBSAMPLE,
		SHADE,
		INTERSECT,
		GENERATE,
		COMPLETE
	};

	// generates a final image, no longer has to check that startup has passed
	virtual void GatherInternal() = 0;

	// change mode, called only by 1 thread
	virtual MODE ChangeMode(bool bTerminate) = 0;

	virtual void ThreadEnter(int iThread) = 0;

	struct ThreadData
	{
		boost::thread*	_threadPointer;
	};

	// constant while threads operating
	std::auto_ptr<boost::barrier>	_modeSwitchBarrier;
	std::vector<ThreadData>			_threads;
	boost::thread_group				_threadGroup;
	int								_numThreads;

	u64								_beginTime;
	u64								_endTime;
	u64								_timeFactor;

	// modified by main thread, read by all worker threads
	bool							_bTerminate;

private:

	MODE							_baseMode;

};

struct EngineSingleThreaded;
struct EngineMultiThreaded;

struct EngineGenerator;
struct EngineSampler;
struct EngineIntersector;
/*
template<class _System,class _SampleData,class _RayData> struct EngineThreadingWrapper
{
	typedef _System System;
	typedef _SampleData BaseSampleData;
	typedef _RayData BaseRayData;

	struct SimpleWrapper
	{
		struct RayDataSTWrapper
		{
			inline RayDataSTWrapper(BaseRayData& data,int threadId) :
				_threadId(threadId),
				_mtRayData(data),
				_nextRay(0)
			{
			}

			
			inline int getNextRays(int& count)
			{
				int res = _mtRayData.getNextRays(_threadId,count);
				_nextRay = res + count;
				return res;
			}

			inline void shadeRay(int rayId,const Vector4& color)
			{
				return _mtRayData.shadeRay(rayId,color);
			}

			inline void getRayInfo(int rayId,Ray& ray) const
			{
				return _mtRayData.getRayInfo(rayId,ray);
			}

			inline void threadCompleteIntersecting()
			{
				_mtRayData.threadCompleteIntersecting(_threadId,_nextRay);
			}

			int				_nextRay;
			int				_threadId;
			BaseRayData&		_mtRayData;
		};
		struct SampleDataSTWrapper
		{
			inline SampleDataSTWrapper(BaseSampleData& data,int threadId) :
				_threadId(threadId),
				_mtSampleData(data)
			{
			}

			int					_threadId;
			BaseSampleData&		_mtSampleData;
		};

		typedef typename RayDataSTWrapper RayData;
		typedef typename SampleDataSTWrapper SampleData;
		typedef typename System::Engine<SampleData,RayData>::type Engine;
		typedef EngineMultiThreaded ThreadingMode;
		typedef typename System::EngineType EngineType;

		struct Implementation
		{
			struct EngineComponent
			{
				inline EngineComponent(const System& system,BaseSampleData& baseSample,BaseRayData& baseRay,int threadId) :
					_rayData(baseRay,threadId),
					_sampleData(baseSample,threadId),
					_system(system),
					_engine(_system,_sampleData,_rayData)
				{

				}

				inline EngineComponent(const EngineComponent& other) :
					_rayData(other._rayData),
					_sampleData(other._sampleData),
					_system(other._system),
					_engine(_system,_sampleData,_rayData)
				{
				}

				RayData		_rayData;
				SampleData	_sampleData;
				const System& _system;
				Engine		_engine;
			};

			Implementation(const System& system,BaseSampleData& samples,BaseRayData& rays) : _system(system),_samples(samples),_rays(rays)
			{
			}
			~Implementation()
			{
			}

			inline void prepare(int numThreads)
			{
				if(_engines.size() != 0)
					return;
				_engines.reserve(numThreads);
				for(int i = 0; i < numThreads; ++i)
					_engines.push_back(EngineComponent(_system,_samples,_rays,i));
				for(auto it = _engines.begin(); it != _engines.end(); ++it)
					it->_engine.prepare();
			}

			inline void threadEnter(int threadId)
			{
				_engines[threadId]._engine.enter();
				_engines[threadId]._rayData.threadCompleteIntersecting();
			}
			inline void threadEnterSubsample(int threadId)
			{
				_engines[threadId]._engine.enterShade();
			}
			inline void threadEnterShade(int threadId)
			{
				_engines[threadId]._engine.enterSubsample();
			}

			BaseSampleData& _samples;
			BaseRayData& _rays;
			System					_system;
			std::vector<EngineComponent>	_engines;
		};

		typedef Implementation type;
	};

	struct GeneratorWrapper 
	{
		struct RayDataGeneratorWrapper
		{
			inline RayDataGeneratorWrapper(BaseRayData& data) :
				_mtRayData(data)
			{
			}
				
			inline void setRayInitialData(int rayId,const Ray& ray,float contribution)
			{
				return _mtRayData.setRayInitialData(rayId,ray,contribution);
			}
			inline void getRayShadingInfo(int rayId,Vector4& color,float& contribution)
			{
				return _mtRayData.getRayShadingInfo(rayId,color,contribution);
			}
			inline int allocateRays(int count)
			{
				return _mtRayData.allocateRays(0,count);
			}

			inline void pushRay(const Vector3& start,const Vector3& dir,int ResultSampleId)
			{
				return _mtRayData.pushRay(start,dir,0,ResultSampleId);
			}

			BaseRayData&		_mtRayData;
		};

		typedef typename RayDataGeneratorWrapper RayData;
		typedef typename BaseSampleData SampleData;
		typedef typename System::Engine<SampleData,RayData>::type Engine;
		typedef EngineMultiThreaded ThreadingMode;
		typedef typename System::EngineType EngineType;

		struct Implementation
		{

			Implementation(const System& system,BaseSampleData& samples,BaseRayData& rays) : _rayData(rays),_engine(system,samples,_rayData)
			{
			}
			~Implementation()
			{
			}

			inline void prepare(int numThreads)
			{
				_engine.prepare();
			}

			inline int gatherOutput()
			{
				return _engine.gatherOutput();
			}

			inline int threadEnter(int thread)
			{
				if(thread == 0)
					return _engine.enter();
				else
					return 0;
			}

			inline void pushGenerate()
			{
				_engine.pushGenerate();
			}
			
			RayData		_rayData;
			Engine		_engine;

		private:
			Implementation(const Implementation& other) {}
		};

		typedef Implementation type;
	};

	struct FlatWrapper
	{
		typedef BaseRayData RayData;
		typedef BaseSampleData SampleData;
		typedef typename System::Engine<SampleData,RayData>::type Engine;
		typedef typename System::EngineType EngineType;
		
		typedef Engine type;
	};

	template<class _ThreadingMode,class _EngineType> struct WrapperSelector
	{
	};

	template<class _EngineType> struct WrapperSelector<EngineMultiThreaded,_EngineType>
	{
		typedef typename FlatWrapper::type type;
	};
	template<class _EngineType> struct WrapperSelector<EngineSingleThreaded,_EngineType>
	{
		typedef typename SimpleWrapper::type type;
	};
	template<> struct WrapperSelector<EngineSingleThreaded,EngineGenerator>
	{
		typedef typename GeneratorWrapper::type type;
	};

	typedef typename WrapperSelector<typename System::ThreadingMode,typename System::EngineType>::type type;
};
*/
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

	inline RaytraceEngine(
		const _SampleGenerator& sampleGenerator,
		const _Intersector& intersector,
		const _Shader& shader,
		const SceneReader& scene) :
			_sampleGenerator(sampleGenerator,_sampleData,_rayData),
			_intersector(intersector,_sampleData,_rayData,scene),
			_shader(shader,_sampleData,_rayData,scene),
			_mode(IDLE)
		{
		}

	inline ~RaytraceEngine()
	{
		TerminateThreads();
	}
	
private:

	void GatherInternal()
	{
		_sampleGenerator.gatherOutput();
	}

	MODE ChangeMode(bool bTerminate)
	{

		MODE newMode;

		if(bTerminate)
		{
			newMode = COMPLETE;
			_mode = newMode;
			return newMode;
		}

		switch(_mode)
		{
		case INTERSECT:
			_rayData.completeIntersecting();
			_sampleData.completePushingSamples();
			assert(!_rayData.hasRaysToIntersect());
			if(_sampleData.hasSamplesToSubsample())
			{
				_rayData.pushStack();
				newMode = SUBSAMPLE;
				break;
			}
			else if(_sampleData.emptyStack())
			{
				newMode = GENERATE;
				break;
			}
			else
			{
				_sampleData.popStack();
				newMode = SHADE;
				break;
			}
			break;
		case SHADE:
			_sampleData.completeShading();
			_sampleData.popShaded();
			
			do
			{
				if(_sampleData.hasSamplesToShade())
				{
					newMode = SHADE;
					break;
				}
				else if(_sampleData.hasSamplesToSubsample())
				{
					_rayData.popStack();
					_rayData.pushStack();
					newMode = SUBSAMPLE;
					break;
				}
				else if(_sampleData.emptyStack())
				{
					newMode = GENERATE;
					break;
				}
				else
				{
					_rayData.popStack();
					_sampleData.popStack();
				}
			}
			while(true);
			break;
		case STARTUP:
		case GENERATE:
			_rayData.completeAllocatingRays();
			if(_rayData.hasRaysToIntersect())
				newMode = INTERSECT;
			else
				newMode = COMPLETE;
			break;
		case SUBSAMPLE:
			_sampleData.completeSubsampling();
			_rayData.completeAllocatingRays();
			if(_rayData.hasRaysToIntersect())
			{
				_sampleData.pushStack();
				newMode = INTERSECT;
			}
			else
				newMode = SHADE;
			break;
		case IDLE:
			_sampleData.prepare(_numThreads);
			_rayData.prepare(_numThreads);

			_sampleGenerator.prepare(_numThreads);
			_intersector.prepare(_numThreads);
			_shader.prepare(_numThreads);
			newMode = STARTUP;
			break;
		}

		switch(newMode)
		{
		case SHADE:
			_sampleData.prepareShading();
			break;
		case SUBSAMPLE:
			_rayData.prepareAllocatingRays();
			_sampleData.prepareSubsampling();
			break;
		case INTERSECT:
			_sampleData.preparePushingSamples();
			_rayData.prepareIntersecting();
			break;
		case GENERATE:
			_sampleGenerator.pushGenerate();
		case STARTUP:
			_rayData.prepareAllocatingRays();
			break;
		}

		_mode = newMode;

		return newMode;
	}
	
	void ThreadEnter(int iThread)
	{
		switch(_mode)
		{
		case SUBSAMPLE:
			_shader.threadEnterSubsample(iThread);
			break;
		case SHADE:
			_shader.threadEnterShade(iThread);
			break;
		case STARTUP:
		case GENERATE:
			_sampleGenerator.threadEnter(iThread);
			break;
		case INTERSECT:
			_intersector.threadEnter(iThread);
			break;
		}
	}
	
	SampleData		_sampleData;
	RayData			_rayData;

	SampleGenerator	_sampleGenerator;
	Intersector		_intersector;
	Shader			_shader;

	// modified by worker thread leader, read by main thread
	MODE			_mode;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

template<class _Generator,class _Intersector,class _Sampler> 
	inline boost::shared_ptr<RaytraceEngineBase> CreateRaytraceEngine(
		const _Generator& sampleGenerator,
		const _Intersector& intersector,
		const _Sampler& shader,
		const SceneReader& scene)
	{
		return boost::shared_ptr<RaytraceEngineBase>(new RaytraceEngine<_Generator,_Intersector,_Sampler>(sampleGenerator,intersector,shader,scene));
	}

}

#endif