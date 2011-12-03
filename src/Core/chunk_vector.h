/********************************************************/
// FILE: chunk_vector.h
// DESCRIPTION: vector that is an 'array' of 'arrays'
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

#ifndef RAYTRACE_CHUNK_VECTOR_GUARD
#define RAYTRACE_CHUNK_VECTOR_GUARD

#include <array>

namespace Raytrace {
	
	// contains _Size elements in _ChunkSize chunks

	template<class _Element, int _Size, int _ChunkSize> struct static_chunked_vector
	{
		typedef _Element value_type;
		typedef size_t size_type;
		typedef _Element& reference;
		typedef const _Element& const_reference;

		// too lazy for this, not so important
		/*
		typedef typename Base::iterator iterator;
		typedef typename Base::const_iterator const_iterator;
		*/

		typedef std::array<_Element,_ChunkSize> chunk_type;
		static const size_t MaxSize = _Size;
		static const size_t ChunkSize = _ChunkSize;
		static const size_t NumChunks = (MaxSize - 1) / _ChunkSize + 1;

		static_chunked_vector() :_size (0)
		{
			for(size_t i = 0; i < NumChunks; ++i)
				_chunks[i] = nullptr;
		}

		~static_chunked_vector()
		{
			for(size_t i = 0; i < NumChunks; ++i)
				if(_chunks[i])
					delete _chunks[i];
		}

		inline bool empty() const
		{
			return _size == 0;
		}

		inline size_type size() const
		{
			return _size;
		}

		inline reference operator[](size_type t)
		{
			return (*_chunks[t/ChunkSize])[t%ChunkSize];
		}
	
		inline const_reference operator[](size_type t) const
		{
			return (*_chunks[t/ChunkSize])[t%ChunkSize];
		}

		inline const_reference front() const
		{
			return operator[](0);
		}
			
		inline reference front()
		{
			return operator[](0);
		}
			
		inline const_reference back() const
		{
			return operator[](size()-1);
		}
			
		inline reference back()
		{
			return operator[](size()-1);
		}

		inline void push_back()
		{
			++_size;
			if(_chunks[(_size-1)/ChunkSize] == nullptr)
				_chunks[(_size-1)/ChunkSize] = new chunk_type;
		}

		inline void push_back(const_reference element)
		{
			push_back();
			back() = element;
		}
			
		inline void pop_back()
		{
			--_size;
		}

		inline void clear()
		{
			_size = 0;
		}

		size_t _size;
		std::array<chunk_type*,NumChunks> _chunks;
	};

	template<class _Element, int _ChunkSize> struct chunked_vector
	{
		typedef _Element value_type;
		typedef size_t size_type;
		typedef _Element& reference;
		typedef const _Element& const_reference;

		// too lazy for this, not so important
		/*
		typedef typename Base::iterator iterator;
		typedef typename Base::const_iterator const_iterator;
		*/

		typedef std::array<_Element,_ChunkSize> chunk_type;
		static const size_t ChunkSize = _ChunkSize;

		chunked_vector() :_size (0)
		{
		}

		~chunked_vector()
		{
			for(size_t i = 0; i < _chunks.size(); ++i)
				if(_chunks[i])
					delete _chunks[i];
		}

		inline bool empty() const
		{
			return _size == 0;
		}

		inline size_type size() const
		{
			return _size;
		}

		inline reference operator[](size_type t)
		{
			return (*_chunks[t/ChunkSize])[t%ChunkSize];
		}
	
		inline const_reference operator[](size_type t) const
		{
			return (*_chunks[t/ChunkSize])[t%ChunkSize];
		}

		inline const_reference front() const
		{
			return operator[](0);
		}
			
		inline reference front()
		{
			return operator[](0);
		}
			
		inline const_reference back() const
		{
			return operator[](size()-1);
		}
			
		inline reference back()
		{
			return operator[](size()-1);
		}

		inline void push_back()
		{
			++_size;
			if((_size-1)/ChunkSize >= _chunks.size())
				_chunks.push_back(new chunk_type);
		}

		inline void push_back(const_reference element)
		{
			push_back();
			back() = element;
		}
			
		inline void pop_back()
		{
			--_size;
		}

		inline void clear()
		{
			_size = 0;
		}

		size_t _size;
		std::vector<chunk_type*> _chunks;
	};
}

#endif