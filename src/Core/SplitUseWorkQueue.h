/********************************************************/
// FILE: SplitUseWorkQueue.h
// DESCRIPTION: A simple split use work queue, supports concurrent write or read but no mixing and no element removal except clearing (single threaded)
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

#ifndef SPLIT_WORK_QUEUE_GUARD
#define SPLIT_WORK_QUEUE_GUARD

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "InterlockedFunctions.h"
#include <deque>
#include <array>
#include <vector>
#include <bitset>
#include "chunk_vector.h"


namespace Raytrace {

// storage specifically for sample and ray data
// assumes the same block will never be read and written at the same time
namespace SplitUseWorkQueueStuff
{
	namespace detail
	{
		template<class _Element,int _Size> struct DefaultElementContainer : public std::array<_Element,_Size>
		{
			typedef _Element& reference;
			typedef const _Element& const_reference;
		};

		template<int _Size> struct DefaultElementContainer<bool,_Size> : public std::bitset<_Size>
		{
			typedef typename std::bitset<_Size>::reference reference;
			typedef bool const_reference;
		};
	}


	template<class _Element,int _Size> struct DefaultElementContainer
	{
		typedef detail::DefaultElementContainer<_Element,_Size> type;
	};
}

template<class _Element,int _BlockSize,class _BlockData = typename SplitUseWorkQueueStuff::DefaultElementContainer<_Element,_BlockSize>::type> class SplitUseWorkQueue
{
public:

	typedef size_t IdType;
	typedef  _BlockData BlockDataContainer;

	typedef typename BlockDataContainer::reference reference;
	typedef typename BlockDataContainer::const_reference const_reference;

	static const IdType INVALID_ID = (IdType) -1;
	static const IdType BlockSize = (IdType) _BlockSize;

	static const size_t MaxBlocks = 1024*1024;
	static const size_t ChunkSize = 1024;

	inline SplitUseWorkQueue() : 
		_numAllocatedBlocks(0),
		_numReadBlocks(0),
		_numWrittenBlocks(0)
	{
		static_assert( _BlockSize > 0 , "Block Size needs to be larger than zero.");
	}

	inline ~SplitUseWorkQueue()
	{
	}

	inline void clear()
	{
		_numWrittenBlocks = 0;
		_numReadBlocks = 0;

		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			it->_currentReadItem = INVALID_ID;
			it->_currentReadBlock = INVALID_ID;
			it->_currentWriteBlock = INVALID_ID;
			it->_numElements = 0;
		}
	}
	
	inline void reset()
	{
		_numReadBlocks = 0;

		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			it->_currentReadItem = INVALID_ID;
			it->_currentReadBlock = INVALID_ID;
		}
	}

	// writing function

	inline void pushElement(const _Element& element,size_t threadId)
	{
		IdType currentBlock = _threadData[threadId]._currentWriteBlock;
		if(currentBlock == INVALID_ID || 1 > (_BlockSize - _allocatedBlocks[currentBlock]._numUsed) )
			currentBlock = _threadData[threadId]._currentWriteBlock = getNewBlock();

		_allocatedBlocks[currentBlock]._data[_allocatedBlocks[currentBlock]._numUsed]  = element;

		_allocatedBlocks[currentBlock]._numUsed ++;
		_threadData[threadId]._numElements ++;
	}
	
	inline _Element* lastWriteElement(size_t threadId)
	{
		IdType currentBlock = _threadData[threadId]._currentWriteBlock;
		return &_allocatedBlocks[currentBlock]._data[_allocatedBlocks[currentBlock]._numUsed-1];
	}

	// reading function

	inline _Element* currElement(size_t threadId)
	{
		if(_threadData[threadId]._currentReadBlock == INVALID_ID)
			return nullptr;
		else
			return &_allocatedBlocks[_threadData[threadId]._currentReadBlock]._data[_threadData[threadId]._currentReadItem];
	}

	inline void advanceElement(size_t threadId)
	{
		_threadData[threadId]._currentReadItem++;
		if( _threadData[threadId]._currentReadBlock == INVALID_ID ||
			_threadData[threadId]._currentReadItem >= _allocatedBlocks[_threadData[threadId]._currentReadBlock]._numUsed )
		{
			_threadData[threadId]._currentReadBlock = getNextBlock();
			_threadData[threadId]._currentReadItem = 0;
		}
	}

	inline void prepare(size_t numThreads)
	{
		_threadData.resize(numThreads);
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			it->_currentReadItem = INVALID_ID;
			it->_currentReadBlock = INVALID_ID;
			it->_currentWriteBlock = INVALID_ID;
			it->_numElements = 0;
		}
	}

	inline size_t  size() const
	{
		size_t total = 0;
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			total += it->_numElements;
		return total;
	}

private:

	
	IdType getNextBlock()
	{
		while(true)
		{
			IdType nextBlock = _numReadBlocks;
			if(nextBlock < _numWrittenBlocks)
			{
				if(InterlockedCompareExchange(_numReadBlocks,nextBlock+1,nextBlock) == nextBlock)
					return nextBlock;
				else
					continue;
			}
			else
				return INVALID_ID;
		}
	}

	IdType getNewBlock()
	{
		while(true)
		{
			IdType newBlock = _numWrittenBlocks;
			if(_numAllocatedBlocks <= newBlock)
			{
				boost::mutex::scoped_lock l(_allocationMutex);
				_allocatedBlocks.push_back();
				++_numAllocatedBlocks;
				continue;
			}
			if(InterlockedCompareExchange(_numWrittenBlocks,newBlock+1,newBlock) == newBlock)
			{
				_allocatedBlocks[newBlock]._numUsed = 0;
				return newBlock;
			}
		}
	}

	struct ThreadData
	{
		IdType _currentReadItem; // INVALID_ID for none
		IdType _currentReadBlock; // INVALID_ID for none
		IdType _currentWriteBlock; // INVALID_ID for none

		size_t _numElements;
	};

	struct Block
	{
		BlockDataContainer					_data;
		IdType								_numUsed;
	};
	
	volatile IdType						_numWrittenBlocks;
	volatile IdType						_numReadBlocks;

	IdType								_numAllocatedBlocks;
	boost::mutex						_allocationMutex;
	
	static_chunked_vector<Block,MaxBlocks,ChunkSize>	_allocatedBlocks;

	std::vector<ThreadData>				_threadData;
};

}

#endif