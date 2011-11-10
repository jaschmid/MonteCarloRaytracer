#include "headers.h"
#include <RaytraceCommon.h>
#include "EngineBase.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Raytrace {

	RaytraceEngineBase::RaytraceEngineBase() :
	  _baseMode(IDLE),
	  _numThreads(-1),
	  _beginTime(0),
	  _endTime(0),
	  _timeFactor(0),
	  _bTerminate(false)
	{
	}

	RaytraceEngineBase::~RaytraceEngineBase() 
	{
	}
	
	Result RaytraceEngineBase::GetLastFrameTime(int& time)
	{
		if(_timeFactor == 0)
			return Result::Failed;
		else
		{
			u64 diff = _endTime - _beginTime;
			time = (int)((diff*1000)/_timeFactor);
			return Result::Succeeded;
		}
	}

	Result RaytraceEngineBase::Gather()
	{
		if(_baseMode == STARTUP || _baseMode == IDLE)
			return Result::Failed;

		MODE mode = _baseMode;

		GatherInternal();

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
		_beginTime = boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds();
		_baseMode = STARTUP;

		_numThreads = numThreads;
		
		threadStartFunctor startFunc;
		startFunc._parent = this;
		startFunc._startupBarrier = startupBarrier;

		//start threads, whee!
		for(int i = 0; i < numThreads; ++i)
			_threads[i]._threadPointer = _threadGroup.create_thread(startFunc);

		startupBarrier->wait();

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
				id = it - _threads.begin();
				break;
			}

		if(id == 0)
			_baseMode = ChangeMode(_bTerminate);

		bool leader = _modeSwitchBarrier->wait();
		MODE myMode = _baseMode;

		while(myMode != COMPLETE)
		{
			ThreadEnter(id);

			leader = _modeSwitchBarrier->wait();

			if(leader)
			{

				MODE newMode = ChangeMode(_bTerminate);
				
				if(newMode == COMPLETE)
				{
					_endTime = boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds();
					_timeFactor = 1000000L;
				}

				_baseMode = newMode;
			}
			
			leader = _modeSwitchBarrier->wait();

			myMode = _baseMode;
		}
		_modeSwitchBarrier->wait();
	}
}