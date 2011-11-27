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
#include <bitset>
#include <ppl.h>
#include <concurrent_vector.h>


namespace Raytrace {

// storage specifically for sample and ray data
// assumes the same block will never be read and written at the same time
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
		typedef typename bool const_reference;
	};
}


template<class _Element,int _Size> struct DefaultElementContainer
{
	typedef detail::DefaultElementContainer<_Element,_Size> type;
};

template<class _Element,int _BlockSize,class _IdType = u32,class _BlockData = typename DefaultElementContainer<_Element,_BlockSize>::type> class ConcurrentStorage
{
public:

	typedef _IdType IdType;
	typedef  _BlockData BlockDataContainer;

	typedef typename BlockDataContainer::reference reference;
	typedef typename BlockDataContainer::const_reference const_reference;

	static const IdType INVALID_ID = (IdType) -1;
	static const IdType BlockSize = (IdType) _BlockSize;

	class MultiAccessIterator
	{
	public:
		MultiAccessIterator() : _storage(nullptr), _nextBlock(INVALID_ID),_threadData(0)
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

		MultiAccessIterator(ConcurrentStorage& storage,IdType firstBlock,IdType endElement) : _storage(&storage), _nextBlock(firstBlock),_endBlock(endElement?((endElement-1)/_BlockSize+1):0),_threadData(storage._threadData.size()),_endElement(endElement)
		{
			for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			{
				it->_currentBlock = INVALID_ID;
				it->_nextElement = INVALID_ID;
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

		inline IdType getNextElement(IdType nThreadId)
		{
			int num = 1;
			return getNextElements(nThreadId,num);
		}

		inline IdType getNextElements(int nThreadId, IdType& numElements)
		{
			ThreadData& data = _threadData[nThreadId];
			if(data._currentBlock == INVALID_ID || data._nextElement >= _storage->_allocatedBlocks[data._currentBlock]._numUsed)
			{
				if(_nextBlock >= _endBlock)
				{
					numElements = 0;
					return _endElement;
				}
				data._currentBlock = InterlockedIncrement(_nextBlock) - 1;
				data._nextElement = INVALID_ID;
				if(data._currentBlock >= _endBlock)
				{
					numElements = 0;
					return _endElement;
				}
			}

			Block& block = _storage->_allocatedBlocks[data._currentBlock];

			IdType element;

			if(data._nextElement == INVALID_ID)
			{
				element = data._currentBlock*_BlockSize;
				numElements = std::min<IdType>(block._numUsed,numElements);
				data._nextElement = numElements;
			}
			else
			{
				element = data._currentBlock*_BlockSize + data._nextElement;
				numElements = std::min<IdType>(block._numUsed - data._nextElement,numElements);
				data._nextElement += numElements;
			}

			if(numElements == 0)
				return _endElement;

			return element;
		}

	private:
		
		struct ThreadData
		{
			IdType _currentBlock;
			IdType _nextElement;
		};

		IdType						_endBlock;
		IdType						_endElement;
		volatile IdType				_nextBlock;

		ConcurrentStorage* _storage;

		std::vector<ThreadData> _threadData;
	};

	inline ConcurrentStorage() : _numAllocatedBlocks(0),_numUsedBlocks(0),_maxBlocks(0x7fffffff),_maxElement(0x7fffffff)
	{
		static_assert( _BlockSize > 0 , "Block Size needs to be larger than zero.");
	}

	inline ~ConcurrentStorage()
	{
	}

	inline MultiAccessIterator begin(IdType firstElement = 0,IdType endElement = INVALID_ID)
	{
		if(firstElement == INVALID_ID)
			firstElement = 0;

		int firstBlock = firstElement/_BlockSize;

		if(endElement < firstElement || endElement == INVALID_ID)
			endElement  = endElementId();

		return MultiAccessIterator(*this, firstBlock, endElement);
	}

	inline void setMaxElement(IdType maxElement = 0x7fffffff)
	{
		_maxElement = maxElement;
	}

	inline void popElements(IdType idNewEnd)
	{
		if(idNewEnd > 0)
		{
			_numUsedBlocks = (idNewEnd-1)/_BlockSize + 1;

			IdType lastBlock = _numUsedBlocks - 1;
			IdType lastElement = (idNewEnd-1)%_BlockSize;

			_allocatedBlocks[lastBlock]._numUsed = lastElement + 1;
		}
		else
			_numUsedBlocks = 0;

		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			it->_currentWriteBlock = INVALID_ID;
	}

	inline IdType endElementId() const
	{
		if(_numUsedBlocks == 0)
			return 0;
		else
			return (_numUsedBlocks-1)*_BlockSize + _allocatedBlocks[_numUsedBlocks-1]._numUsed;
	}

	inline void finishWritingBlock()
	{
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			it->_currentWriteBlock = INVALID_ID;
	}

	inline typename BlockDataContainer::reference getElement(IdType id)
	{
		return _allocatedBlocks[id/_BlockSize]._data[id%_BlockSize];
	}
	inline typename BlockDataContainer::const_reference getElement(IdType id) const
	{
		return _allocatedBlocks[id/_BlockSize]._data[id%_BlockSize];
	}

	inline IdType pushElements(int threadId,IdType count)
	{
		if(count > _BlockSize || count <= 0)
			return INVALID_ID;
		int currentBlock = _threadData[threadId]._currentWriteBlock;
		if(currentBlock == -1 || count > (_BlockSize - _allocatedBlocks[currentBlock]._numUsed) )
			currentBlock = _threadData[threadId]._currentWriteBlock = getNewBlock();
		if(currentBlock == -1)
			return INVALID_ID;

		int element = _allocatedBlocks[currentBlock]._numUsed + currentBlock*_BlockSize;
		_allocatedBlocks[currentBlock]._numUsed += count;

		return element;
	}

	inline void prepare(int numThreads)
	{
		_threadData.resize(numThreads);
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
			it->_currentWriteBlock = INVALID_ID;
	}

private:

	ConcurrentStorage(const ConcurrentStorage& other) {}

	friend class MultiAccessIterator;

	int getNewBlock()
	{
		while(true)
		{
			IdType newBlock = _numUsedBlocks;
			if(newBlock >= _maxBlocks || newBlock >= (_maxElement-1)/_BlockSize+1)
				return INVALID_ID;
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
		IdType _currentWriteBlock; // -1 for none
	};

	struct Block
	{
		BlockDataContainer					_data;
		IdType								_numUsed;
	};
	
	volatile IdType						_numUsedBlocks;
	const IdType						_maxBlocks;
	IdType								_maxElement;

	IdType								_numAllocatedBlocks;
	boost::mutex						_allocationMutex;
	Concurrency::concurrent_vector<Block,Eigen::aligned_allocator<Block>>		_allocatedBlocks;

	std::vector<ThreadData>				_threadData;
};

}

#endif