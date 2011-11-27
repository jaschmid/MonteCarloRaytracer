/********************************************************/
// FILE: ConcurrentStorage.h
// DESCRIPTION: A concurrent storage class
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

#ifndef CONCURRENT_STORAGE_GUARD
#define CONCURRENT_STORAGE_GUARD

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include "InterlockedFunctions.h"
#include <deque>
#include <array>
#include <vector>
#include <ppl.h>
#include <concurrent_vector.h>


namespace Raytrace {

// storage specifically for sample and ray data
// assumes the same block will never be read and written at the same time

template<class _Element,int _BlockSize> class ConcurrentStorage
{
public:

	class MultiAccessIterator
	{
	public:
		MultiAccessIterator() : _storage(nullptr), _nextBlock(-1),_threadData(0)
		{
		}

		MultiAccessIterator(const MultiAccessIterator& other) : _storage(other.storage), _nextBlock(other._nextBlock),_endBlock(other._endBlock),_threadData(other._threadData.size()),_endElement(other._endElement)
		{
			for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			{
				it->_currentBlock = other._threadData[it - _threadData.begin()]._currentBlock;
				it->_currentElement = other._threadData[it - _threadData.begin()]._currentElement;
			}
		}

		MultiAccessIterator(ConcurrentStorage& storage,int firstBlock,int endElement) : _storage(&storage), _nextBlock(firstBlock),_endBlock((endElement-1)/_BlockSize+1),_threadData(storage._threadData.size()),_endElement(endElement)
		{
			for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			{
				it->_currentBlock = -1;
				it->_nextElement = -1;
			}
		}

		MultiAccessIterator& operator = (const MultiAccessIterator& other)
		{
			_endElement = other._endElement;
			_storage = other._storage;
			_nextBlock = other._nextBlock;
			_endBlock = other._endBlock;
			_threadData.resize(other._threadData.size());
			for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			{
				it->_currentBlock = other._threadData[it - _threadData.begin()]._currentBlock;
				it->_nextElement = other._threadData[it - _threadData.begin()]._nextElement;
			}
			return *this;
		}

		~MultiAccessIterator()
		{
		}

		inline int getNextElement(int nThreadId)
		{
			int num = 1;
			return getNextElements(nThreadId,num);
		}

		inline int getNextElements(int nThreadId, int& numElements)
		{
			ThreadData& data = _threadData[nThreadId];
			if(data._currentBlock == -1 || data._nextElement >= _storage->_allocatedBlocks[data._currentBlock]._numUsed)
			{
				if(_nextBlock >= _endBlock)
				{
					numElements = 0;
					return _endElement;
				}
				data._currentBlock = InterlockedIncrement(_nextBlock) - 1;
				data._nextElement = -1;
				if(data._currentBlock >= _endBlock)
				{
					numElements = 0;
					return _endElement;
				}
			}

			Block& block = _storage->_allocatedBlocks[data._currentBlock];

			int element;

			if(data._nextElement == -1)
			{
				element = data._currentBlock*_BlockSize;
				numElements = std::min<int>(block._numUsed,numElements);
				data._nextElement = numElements;
			}
			else
			{
				element = data._currentBlock*_BlockSize + data._nextElement;
				numElements = std::min<int>(block._numUsed - data._nextElement,numElements);
				data._nextElement += numElements;
			}

			if(numElements == 0)
				return _endElement;

			return element;
		}

	private:
		
		struct ThreadData
		{
			int _currentBlock;
			int _nextElement;
		};

		int						_endBlock;
		int						_endElement;
		volatile i32			_nextBlock;

		ConcurrentStorage* _storage;

		std::vector<ThreadData> _threadData;
	};

	inline ConcurrentStorage() : _numAllocatedBlocks(0),_numUsedBlocks(0),_maxBlocks(0x7fffffff),_maxElement(0x7fffffff)
	{
	}

	inline ~ConcurrentStorage()
	{
	}

	inline MultiAccessIterator begin(int firstElement = 0,int endElement = -1)
	{
		if(firstElement < 0)
			firstElement = 0;

		int firstBlock = firstElement/_BlockSize;

		if(endElement < firstElement)
			endElement  = endElementId();

		return MultiAccessIterator(*this, firstBlock, endElement);
	}

	inline void setMaxElement(int maxElement = 0x7fffffff)
	{
		_maxElement = maxElement;
	}

	inline void popElements(int idNewEnd)
	{
		_numUsedBlocks = (idNewEnd-1)/_BlockSize + 1;
		if(idNewEnd > 0)
		{
			int lastBlock = _numUsedBlocks - 1;
			int lastElement = (idNewEnd-1)%_BlockSize;

			_allocatedBlocks[lastBlock]._numUsed = lastElement + 1;
		}

		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			it->_currentWriteBlock = -1;
	}

	inline int endElementId() const
	{
		if(_numUsedBlocks == 0)
			return 0;
		else
			return (_numUsedBlocks-1)*_BlockSize + _allocatedBlocks[_numUsedBlocks-1]._numUsed;
	}

	inline void finishWritingBlock()
	{
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			it->_currentWriteBlock = -1;
	}

	inline _Element& getElement(int id)
	{
		return _allocatedBlocks[id/_BlockSize]._data[id%_BlockSize];
	}
	inline const _Element& getElement(int id) const
	{
		return _allocatedBlocks[id/_BlockSize]._data[id%_BlockSize];
	}

	inline int pushElements(int threadId,int count)
	{
		if(count > _BlockSize || count <= 0)
			return -1;
		int currentBlock = _threadData[threadId]._currentWriteBlock;
		if(currentBlock == -1 || count > (_BlockSize - _allocatedBlocks[currentBlock]._numUsed) )
			currentBlock = _threadData[threadId]._currentWriteBlock = getNewBlock();
		if(currentBlock == -1)
			return -1;

		int element = _allocatedBlocks[currentBlock]._numUsed + currentBlock*_BlockSize;
		_allocatedBlocks[currentBlock]._numUsed += count;

		return element;
	}

	inline void prepare(int numThreads)
	{
		_threadData.resize(numThreads);
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			it->_currentWriteBlock = -1;
	}

private:

	ConcurrentStorage(const ConcurrentStorage& other) {}

	friend class MultiAccessIterator;

	int getNewBlock()
	{
		while(true)
		{
			i32 newBlock = _numUsedBlocks;
			if(newBlock >= _maxBlocks || newBlock >= (_maxElement-1)/_BlockSize+1)
				return -1;
			if(_numAllocatedBlocks <= newBlock)
			{
				boost::mutex::scoped_lock l(_allocationMutex);
				_allocatedBlocks.push_back(Block());
				++_numAllocatedBlocks;
				continue;
			}
			if(InterlockedCompareExchange(_numUsedBlocks,newBlock+1,newBlock) == newBlock)
			{
				_allocatedBlocks[newBlock]._numUsed = 0;
				return newBlock;
			}
		}
	}

	struct ThreadData
	{
		int _currentWriteBlock; // -1 for none
	};

	struct Block
	{
		std::array<_Element,_BlockSize>		_data;
		int									_numUsed;
	};
	
	volatile i32				_numUsedBlocks;
	const int							_maxBlocks;
	int									_maxElement;

	int									_numAllocatedBlocks;
	boost::mutex						_allocationMutex;
	Concurrency::concurrent_vector<Block,Eigen::aligned_allocator<Block>>		_allocatedBlocks;

	std::vector<ThreadData>				_threadData;
};

}

#endif