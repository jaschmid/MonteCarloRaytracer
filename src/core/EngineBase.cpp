#include "headers.h"
#include <RaytraceCommon.h>
#include "EngineBase.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace Raytrace {

	RaytraceEngineBase::RaytraceEngineBase() :
	  _mode(IDLE),
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
			::Math::u64 diff = _endTime - _beginTime;
			time = (int)((diff*1000)/_timeFactor);
			return Result::Succeeded;
		}
	}

	Result RaytraceEngineBase::Gather()
	{
		if(_mode == STARTUP || _mode == IDLE)
			return Result::Failed;

		GatherInternal();

		if(_mode == COMPLETE)
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
		_mode = STARTUP;

		_numThreads = numThreads;

		Prepare(numThreads);
		
		threadStartFunctor startFunc;
		startFunc._parent = this;
		startFunc._startupBarrier = startupBarrier;

		//start threads, whee!
		for(int i = 0; i < numThreads; ++i)
		{
			_threads[i]._idle = false;
			_threads[i]._threadPointer = _threadGroup.create_thread(startFunc);
		}

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
		mode myMode = STARTUP;
		bool leader = false;
		
		int id = -1;

		for(auto it = _threads.begin(); it != _threads.end(); ++it)
			if(it->_threadPointer->get_id() ==  boost::this_thread::get_id())
			{
				id = it - _threads.begin();
				break;
			}

		leader = _modeSwitchBarrier->wait();
		while(myMode != COMPLETE)
		{
			int numResults = 0;
			switch(myMode)
			{
			case STARTUP:
				numResults = ThreadEnterStartup(id);
				myMode = INTERSECT;
				break;
			case SHADER:
				numResults = ThreadEnterShade(id);
				myMode = INTERSECT;
				break;
			case INTERSECT:
				numResults = ThreadEnterIntersect(id);
				myMode = SHADER;
				break;
			}

			if(numResults && !_bTerminate)
				_threads[id]._idle = false;
			else
				_threads[id]._idle = true;

			leader = _modeSwitchBarrier->wait();

			bool anyBusy = false;
			for(auto it = _threads.begin(); it != _threads.end(); ++it)
				if(!it->_idle)
				{
					anyBusy = true;
					break;
				}
			if(!anyBusy)
				myMode = COMPLETE;
			if(leader)
			{
				_mode = myMode;
				if(myMode == COMPLETE)
				{
					_endTime = boost::posix_time::microsec_clock::local_time().time_of_day().total_microseconds();
					_timeFactor = 1000000L;
				}
			}
		}
		_modeSwitchBarrier->wait();
	}
}