#include "headers.h"
#include <RaytraceCommon.h>
#include <Math/InterlockedFunctions.h>
#include "EngineBase.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>

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

	RaytraceEngineBase::RaytraceEngineBase() :
	  _timingMode(IDLE),
	  _numThreads(-1),
	  _beginTime(0),
	  _timeFactor(0),
	  _bTerminate(false)
	{
	}

	RaytraceEngineBase::~RaytraceEngineBase() 
	{
		TerminateThreads();
	}
	
	Result RaytraceEngineBase::GetLastFrameTime(int mode,int& time)
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
		if(_threads.size() != 0)
			return Result::Failed;

		if(numThreads == 0)
			numThreads = boost::thread::hardware_concurrency();
		boost::shared_ptr<boost::barrier> startupBarrier(new boost::barrier(numThreads + 1));
		_modeSwitchBarrier.reset( new boost::barrier(numThreads) );
		_threads.resize(numThreads);

		//time stuff
		_beginTime = boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds();
		_timingMode = STARTUP;
		_timeFactor = 1000000L;

		for(auto it = _timeByMode.begin(); it != _timeByMode.end(); ++it)
			*it = 0;

		_numThreads = numThreads;
		_threadCount = _numThreads;
		
		threadStartFunctor startFunc;
		startFunc._parent = this;
		startFunc._startupBarrier = startupBarrier;

		//start threads, whee!
		for(int i = 0; i < numThreads; ++i)
			_threads[i]._threadPointer = _threadGroup.create_thread(startFunc);

		startupBarrier->wait();

		return Result::Succeeded;
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
		int id = -1;

		for(auto it = _threads.begin(); it != _threads.end(); ++it)
			if(it->_threadPointer->get_id() ==  boost::this_thread::get_id())
			{
				id = (int)(it - _threads.begin());
				break;
			}

		if(id == 0)
			setTimingMode(ChangeMode(_bTerminate));

		OS::setThreadAffinity(id);
		OS::setPriorityLow();

		bool leader = _modeSwitchBarrier->wait();

		while(_timingMode != COMPLETE)
		{
			ThreadEnter(id);
			
			u32 count = InterlockedDecrement(_threadCount);

			if(count == 0)
			{
				setTimingMode(IDLE);
				setTimingMode(ChangeMode(_bTerminate));
				_threadCount = _threads.size();
			}
			
			_modeSwitchBarrier->wait();
		}
		_modeSwitchBarrier->wait();
	}
}