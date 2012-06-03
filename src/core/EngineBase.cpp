#include "headers.h"
#include <RaytraceCommon.h>
#include "InterlockedFunctions.h"
#include "EngineBase.h"
#include "RayData.h"
#include "SampleData.h"
#include "Engines.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <sstream>

#ifdef TARGET_WINDOWS

#include <Windows.h>

namespace OS
{
	inline void setThreadAffinity(size_t i)
	{
		SetThreadIdealProcessor( GetCurrentThread(), (DWORD)i );
	}

	inline void setPriorityLow()
	{
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_LOWEST);
	}
}

#else

namespace OS
{
	inline void setThreadAffinity(size_t i)
	{
	}

	inline void setPriorityLow()
	{
	}
}

#endif

namespace Raytrace {
	
template<class _Callee>
struct threadStartFunctor
{
	inline void operator()()
	{
		return _parent->ThreadEnter();
	}
	_Callee*		_parent;
};

template<class _RayData,class _SampleData, class _SceneReader>
const std::array<String,7> BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::ModeName = { "Idle", "Startup", "Sample", "Integrate", "Intersect", "Complete", "Overhead" };

template<class _RayData,class _SampleData,class _SceneReader>
BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::BasicRaytraceEngine(
	const IntersectorType& intersector,
	const SamplerType& sampler,
	const IntegratorType& integrator,
	const SceneReader& sceneReader) :
	_intersector(intersector),
	_sampler(sampler),
	_integrator(integrator),
	_sceneReader(sceneReader),
	_mode(IDLE),
	_threadTerminateCounter(-1),
	_threadCompletionCounter(0),
	_numThreads(0),
	_beginTime(0),
	_totalBeginTime(0),
	_totalEndTime(0),
	_timeFactor(0),
	_totalFirstHitRays(0),
	_totalAnyHitRays(0),
	_totalSamples(0),
	_refcount(0),
	_initComplete(false),
	_progress(0.0f)
{

	_numThreads = boost::thread::hardware_concurrency();

	_modeSwitchBarrier.reset( new boost::barrier(_numThreads) );
	_startupBarrier.reset( new boost::barrier(_numThreads + 1) );
	_threads.resize(_numThreads);

	threadStartFunctor<ThisType> startFunc;
	startFunc._parent = this;
	
	_threadCompletionCounter = _numThreads;
	//start threads, whee!
	for(int i = 0; i < _numThreads; ++i)
		_threads[i]._threadPointer = _threadGroup.create_thread(startFunc);
	_startupBarrier->wait();
}

template<class _RayData,class _SampleData,class _SceneReader>
BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::~BasicRaytraceEngine() 
{
	_threads.clear();
}

	
template<class _RayData,class _SampleData,class _SceneReader>
Result BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::Initialize(const SceneReader& scene)
{
	return Result::NotImplemented;
}

template<class _RayData,class _SampleData,class _SceneReader>
Result BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::Begin()
{
	if(_mode != IDLE)
		return Result::Failed;
	

	//time stuff
	_totalBeginTime = _beginTime = boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds();
	_timeFactor = 1000000;

	for(auto it = _timeByMode.begin(); it != _timeByMode.end(); ++it)
		*it = 0;

	_startupBarrier->wait();

	return Result::Succeeded;
}

template<class _RayData,class _SampleData,class _SceneReader>
Result BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::Pause()
{
	return Result::NotImplemented;
}

template<class _RayData,class _SampleData,class _SceneReader>
Result BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::Resume()
{
	return Result::NotImplemented;
}

template<class _RayData,class _SampleData,class _SceneReader>
Result BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::GetStatus(const String& status_type,String& status_out) const
{
	
	if(status_type == String("info") && (size_t)_mode >= 0 && (size_t)_mode <= ModeName.size())
	{
		std::ostringstream text;

		text.precision(2);
		
		u64 timeElapsed = 0;
		u64 samplesSec = 0;
		u64 raysSec = 0;

		if(_totalEndTime == 0)
			 timeElapsed = boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds() - _totalBeginTime;
		else
			timeElapsed = _totalEndTime - _totalBeginTime;
		
		if(timeElapsed > 0)
		{
			samplesSec = _totalSamples * _timeFactor / timeElapsed;
			if( _timeByMode[INTERSECT] > 0)
				raysSec = (_totalFirstHitRays + _totalAnyHitRays) * _timeFactor / _timeByMode[INTERSECT];
		}

		if(_progress < 1.0f)
		{
			text << ModeName[_mode] << String("-") << std::fixed << _progress*100 << String("%:");
		}
		else
		{
			for(size_t i = 0; i < NUM_MODES; ++i)
				text << ModeName[i] << ": " << _timeByMode[i]*1000/_timeFactor << " ms  ";
		}

		text << samplesSec << String(" samples/second ");
		text << raysSec << String(" rays/second ");

		status_out = text.str();

		if(_progress < 1.0f)
			return Result::RenderingInProgress;
		else
			return Result::RenderingComplete;
	}

	return Result::Failed;
}

template<class _RayData,class _SampleData,class _SceneReader>
Result BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::GatherPreview(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const
{
	if(_initComplete)
	{
		return _sampler->GatherPreview(format,xRes,yRes,pDataOut);
	}
	else
		return Result::Failed;

}

template<class _RayData,class _SampleData,class _SceneReader>
Result BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::GetImage(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const
{
	if(_mode == COMPLETE)
	{
		_sampler->GetImage(format,xRes,yRes,pDataOut);
		return Result::Succeeded;
	}
	else
		return Result::Failed;
}

template<class _RayData,class _SampleData,class _SceneReader>
void BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::AddRef()
{
	InterlockedIncrement(_refcount);
}

template<class _RayData,class _SampleData,class _SceneReader>
void BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::Release()
{
	u32 count = InterlockedDecrement(_refcount);

	if(count == 1)
		TerminateThreads();
	else if(count == 0)
		delete this;
}

template<class _RayData,class _SampleData,class _SceneReader>
void BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::TerminateThreads()
{
	_threadTerminateCounter = _numThreads;
}
	
template<class _RayData,class _SampleData,class _SceneReader>
void BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::ThreadEnter()
{
	AddRef();
	_startupBarrier->wait();

	int id = -1;

	for(auto it = _threads.begin(); it != _threads.end(); ++it)
		if(it->_threadPointer->get_id() ==  boost::this_thread::get_id())
		{
			id = (int)(it - _threads.begin());
			break;
		}

	OS::setThreadAffinity(id);
	OS::setPriorityLow();

	MODE myMode;
	bool leader = false;

	_startupBarrier->wait();

	if(id == 0)
	{
		setMode(OVERHEAD);
		_nextMode = STARTUP;
		EnterModeST(_nextMode);
		_threadCompletionCounter = (u32)_threads.size();
	}


	if(_modeSwitchBarrier->wait())
	{
		assert(_nextMode == STARTUP);
		_initComplete = true;
		setMode(_nextMode);
	}
	
	myMode = _nextMode;

	assert(myMode == STARTUP);
	assert(_nextMode == STARTUP);
	
	while(_nextMode != COMPLETE)
	{
		EnterModeMT(_nextMode,id);

		u32 count = InterlockedDecrement(_threadCompletionCounter);
		if(count == _numThreads -1)
			setMode(OVERHEAD);
		else if(count == 0)
		{
			LeaveModeST(myMode);
			_nextMode = getNextMode(myMode);
			EnterModeST(_nextMode);
			_threadCompletionCounter = (u32)_threads.size();
		}
			
		if(_modeSwitchBarrier->wait())
			setMode(_nextMode);
		
		myMode = _nextMode;
	}

	Release();

}

template<class _RayData,class _SampleData,class _SceneReader>
void BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::EnterModeST(MODE mode)
{
	switch(mode)
	{
	case STARTUP:
		_sampleData.InitializeST(_numThreads);
		_rayData.InitializeST(_numThreads);

		_sampler->InitializePrepareST(_numThreads,_sceneReader,_sampleData);
		_intersector->InitializePrepareST(_numThreads,_sceneReader,_rayData);
		_integrator->InitializePrepareST(_numThreads,_sceneReader,_sampleData,_rayData);
		break;
	case SAMPLE:
		_sampleData.PrepareSampleST();

		_totalSamples += _sampleData.getNumCompletedSamples();
		_sampler->GeneratePrepareST();
		break;
	case INTEGRATE:
		_sampleData.PrepareIntegrateST();
		_rayData.PrepareIntegrateST();

		_integrator->IntegratePrepareST();
		break;
	case INTERSECT:
		_rayData.PrepareIntersectST();

		_totalAnyHitRays += _rayData.getNumAnyHitRays();
		_totalFirstHitRays += _rayData.getNumFirstHitRays();
		_intersector->IntersectPrepareST();
		break;
	case COMPLETE:
		break;
	default:
		assert(!"Error: Illegal Mode!");
		break;
	}
}

template<class _RayData,class _SampleData,class _SceneReader>
void BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::EnterModeMT(MODE mode,size_t threadId)
{
	switch(mode)
	{
	case STARTUP:
		_sampler->InitializeMT(threadId);
		_intersector->InitializeMT(threadId);
		_integrator->InitializeMT(threadId);
		break;
	case SAMPLE:
		_sampler->GenerateMT(threadId);
		break;
	case INTEGRATE:
		_integrator->IntegrateMT(threadId);
		break;
	case INTERSECT:
		_intersector->IntersectMT(threadId);
		break;
	default:
		assert(!"Error: Illegal Mode!");
		break;
	}
}

template<class _RayData,class _SampleData,class _SceneReader>
void BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::LeaveModeST(MODE mode)
{
	switch(mode)
	{
	case STARTUP:
		_sampler->InitializeCompleteST();
		_intersector->InitializeCompleteST();
		_integrator->InitializeCompleteST();
		break;
	case SAMPLE:
		_sampleData.CompleteSampleST();
		_progress = _sampler->GenerateCompleteST();
		break;
	case INTEGRATE:
		_sampleData.CompleteIntegrateST();
		_rayData.CompleteIntegrateST();
		_integrator->IntegrateCompleteST();
		break;
	case INTERSECT:
		_rayData.CompleteIntersectST();
		_intersector->IntersectCompleteST();
		break;
	default:
		assert(!"Error: Illegal Mode!");
		break;
	}
}

template<class _RayData,class _SampleData,class _SceneReader>
typename BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::MODE BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::getNextMode(MODE prevMode)
{
	if(_threadTerminateCounter > 0)
		return COMPLETE;
	MODE nextMode;
	switch(prevMode)
	{
	case STARTUP:
		nextMode = SAMPLE;
		break;
	case SAMPLE:
		if(_sampleData.getNumActiveSamples() > 0)
			nextMode = INTEGRATE;
		else
		{
			_totalEndTime = boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds();
			nextMode = COMPLETE;
			_progress = 1.0f;
		}
		break;
	case INTEGRATE:
		if(_rayData.getNumAnyHitRays() + _rayData.getNumFirstHitRays() > 0)
			nextMode = INTERSECT;
		else
			nextMode = SAMPLE;
		break;
	case INTERSECT:
		nextMode = INTEGRATE;
		break;
	default:
		assert(!"Error: Illegal Mode!");
		break;
	}

	return nextMode;
}

template<
	class _SampleData,
	class _RayData, 
	class _SceneReader>
	ObjectPointer<IEngine<_SceneReader>> CreateBasicRaytraceEngine(
		const boost::shared_ptr<IIntersector<_RayData,_SceneReader>>& intersector,
		const boost::shared_ptr<ISampler<_SampleData,_SceneReader>>& sampler,
		const boost::shared_ptr<IIntegrator<_SampleData,_RayData,_SceneReader>>& integrator,
		const _SceneReader& scene)
	{
		return ObjectPointer<IEngine<_SceneReader>>(new BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>(intersector,sampler,integrator,scene));
	}

//template class BasicRaytraceEngine<typename DefaultEngine::RayData,typename DefaultEngine::SampleData,typename DefaultEngine::SceneReader>;

template ObjectPointer<IEngine<DefaultEngine::SceneReader>>
	CreateBasicRaytraceEngine<DefaultEngine::SampleData,DefaultEngine::RayData,DefaultEngine::SceneReader>(
		const boost::shared_ptr<IIntersector<DefaultEngine::RayData,DefaultEngine::SceneReader>>& intersector,
		const boost::shared_ptr<ISampler<DefaultEngine::SampleData,DefaultEngine::SceneReader>>& sampler,
		const boost::shared_ptr<IIntegrator<DefaultEngine::SampleData,DefaultEngine::RayData,DefaultEngine::SceneReader>>& integrator,
		const DefaultEngine::SceneReader& scene);
	

	/*
template<class _RayData,class _SampleData,class _SceneReader>
	Result BasicRaytraceEngine<_RayData,_SampleData,_SceneReader>::GetLastFrameTime(int mode,int& time)
	{
		if(_timeFactor == 0)
			return Result::Failed;
		else
		{
			time = (int)((_timeByMode[mode]*1000)/_timeFactor);
			return Result::Succeeded;
		}
	}

	Result RaytraceEngineBase::Gather(void* pOut,size_t nOut)
	{
		if(_timingMode == STARTUP || _timingMode == IDLE)
			return Result::Failed;

		MODE mode = _timingMode;

		GatherInternal(pOut,nOut);

		if(mode == COMPLETE)
			return Result::RenderingComplete;
		else
			return Result::RenderingInProgress;
	}

	Result RaytraceEngineBase::Begin(int numThreads)
	{
	}

	Result RaytraceEngineBase::Stop()
	{
		_bTerminate = true;

		return Result::Succeeded;
	}

	void RaytraceEngineBase::TerminateThreads()
	{
		_bTerminate = true;
		_threadGroup.join_all();
		_threads.clear();
	}

	void RaytraceEngineBase::ThreadEnter()
	{
	}*/
}