/********************************************************/
// FILE: Aligned.h
// DESCRIPTION: Alignment template base class
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

#ifndef RAYTRACE_ALIGNED_GUARD
#define RAYTRACE_ALIGNED_GUARD

#include <RaytraceCommon.h>
#include <stddef.h>  // Required for size_t and ptrdiff_t and NULL
#include <new>       // Required for placement new and std::bad_alloc
#include <stdexcept> // Required for std::length_error
 
// The following headers contain stuff that AlignedAllocator uses.
#include <stdlib.h>  // For malloc() and free()

namespace Raytrace {

	namespace detail
	{

		template<int _Alignment> struct aligned_base;
		
		#ifdef COMPILER_MSVC
			template<> struct __declspec(align(1)) aligned_base<1>{};
			template<> struct __declspec(align(2)) aligned_base<2>{};
			template<> struct __declspec(align(4)) aligned_base<4>{};
			template<> struct __declspec(align(8)) aligned_base<8>{};
			template<> struct __declspec(align(16)) aligned_base<16>{};
			template<> struct __declspec(align(32)) aligned_base<32>{};
			template<> struct __declspec(align(64)) aligned_base<64>{};
		#endif
	}

	
	template<int _Alignment> struct Aligned : public detail::aligned_base<_Alignment>
	{
	};

	template <typename T> class AlignedAllocator {
	public:

		typedef T * pointer;
		typedef const T * const_pointer;
		typedef T& reference;
		typedef const T& const_reference;
		typedef T value_type;
		typedef size_t size_type;
		typedef ptrdiff_t difference_type;
 
		T * address(T& r) const {
			return &r;
		}
 
		const T * address(const T& s) const {
			return &s;
		}
 
		size_t max_size() const {
			return (static_cast<size_t>(0) - static_cast<size_t>(1)) / sizeof(T);
		}
 
 
		// The following must be the same for all allocators.
		template <typename U> struct rebind {
			typedef AlignedAllocator<U> other;
		};
 
		bool operator!=(const AlignedAllocator& other) const {
			return !(*this == other);
		}
 
		void construct(T * const p, const T& t) const {
			void * const pv = static_cast<void *>(p);
 
			new (pv) T(t);
		}
 
		void destroy(T * const p) const
		{
			p->~T();
		}
 
 
		bool operator==(const AlignedAllocator& other) const {
			return true;
		}
 
 
		// Default constructor, copy constructor, rebinding constructor, and destructor.
		// Empty for stateless allocators.
		AlignedAllocator() { }
 
		AlignedAllocator(const AlignedAllocator&) { }
 
		template <typename U> AlignedAllocator(const AlignedAllocator<U>&) { }
 
		~AlignedAllocator() { }
 
 
		// The following will be different for each allocator.
		T * allocate(const size_t n) const {

			if (n == 0) {
				return nullptr;
			}

			if (n > max_size()) {
				throw std::length_error("AlignedAllocator<T>::allocate() - Integer overflow.");
			}
 
			void* pv = nullptr;
			#ifdef COMPILER_MSVC
			pv = _aligned_malloc(n*sizeof(T), std::alignment_of<T>::value);
			#elif (defined COMPILER_GCC)
			pv = _mm_malloc(n*sizeof(T), std::alignment_of<T>::value);
			#endif
 
			// Allocators should throw std::bad_alloc in the case of memory allocation failure.
			if (pv == nullptr) {
				throw std::bad_alloc();
			}
 
			return static_cast<T *>(pv);
		}
 
		void deallocate(T * const p, const size_t n) const {
			// AlignedAllocator wraps free().
			#ifdef COMPILER_MSVC
			_aligned_free(p);
			#elif (defined COMPILER_GCC)
			_mm_free(p);
			#endif
		}
 
 
		// The following will be the same for all allocators that ignore hints.
		template <typename U> T * allocate(const size_t n, const U * /* const hint */) const {
			return allocate(n);
		}
 
 
		// Allocators are not required to be assignable, so
		// all allocators should have a private unimplemented
		// assignment operator. Note that this will trigger the
		// off-by-default (enabled under /Wall) warning C4626
		// "assignment operator could not be generated because a
		// base class assignment operator is inaccessible" within
		// the STL headers, but that warning is useless.
	private:
		AlignedAllocator& operator=(const AlignedAllocator&);
	};


}

#endif