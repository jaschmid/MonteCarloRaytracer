/********************************************************/
// FILE: static_vector.h
// DESCRIPTION: stl compatible static_vector, vector with a bounded maximum size
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

#ifndef RAYTRACE_STATIC_VECTOR_GUARD
#define RAYTRACE_STATIC_VECTOR_GUARD

#include <RaytraceCommon.h>
#include <array>

namespace Raytrace {
	
	template<class _base, int _size> struct static_vector : private std::array<_base,_size>
	{
		typedef std::array<_base,_size> Base;
		typedef typename Base::value_type value_type;
		typedef typename Base::size_type size_type;
		typedef typename Base::reference reference;
		typedef typename Base::const_reference const_reference;
		typedef typename Base::iterator iterator;
		typedef typename Base::const_iterator const_iterator;

		static_vector() :_size (0)
		{
		}

		inline iterator begin()
		{
			return Base::begin();
		}
			
		inline iterator end()
		{
			return Base::begin() + (_size);
		}
			
		inline const_iterator begin() const
		{
			return Base::begin();
		}
			
		inline const_iterator end() const
		{
			return Base::begin() + (_size);
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
			return Base::operator[](t);
		}
	
		inline const_reference operator[](size_type t) const
		{
			return Base::operator[](t);
		}

		inline const_reference front() const
		{
			return Base::operator[](0);
		}
			
		inline reference front()
		{
			return Base::operator[](0);
		}
			
		inline const_reference back() const
		{
			return Base::operator[](_size-1);
		}
			
		inline reference back()
		{
			return Base::operator[](_size-1);
		}

		inline void push_back(const_reference element)
		{
			Base::operator[](_size) = element;
			++_size;
		}

		inline void conditional_push_back(bool condition,const_reference element)
		{
			Base::operator[](_size) = element;
			_size = condition ? _size +1 : _size;
		}

		inline void resize(size_t size) 
		{
			_size = size;;
		}
			
		inline void pop_back()
		{
			--_size;
		}

		size_t _size;
	};
}

#endif