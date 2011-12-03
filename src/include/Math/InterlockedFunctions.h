/********************************************************/
// FILE: InterlockedFunctions.h                                         
// DESCRIPTION: Inline Wrappers to OS/CPU dependant Interlocked Functions
// AUTHOR: Jan Schmid (jaschmid@eml.cc)    
/********************************************************/
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc/3.0/ or send 
// a letter to Creative Commons, 444 Castro Street, 
// Suite 900, Mountain View, California, 94041, USA.
/********************************************************/

#ifndef RAYTRACE_INTERLOCKED_FUNCTIONS_H
#define RAYTRACE_INTERLOCKED_FUNCTIONS_H

#ifdef COMPILER_MSVC
#pragma once
#endif

#if defined(COMPILER_MSVC)

#include <Windows.h>
#include <intrin.h>

namespace Raytrace {

/*interlocked increment*/
	
#ifdef TARGET_X86
inline size_t InterlockedIncrement(volatile size_t& p)
{
    return (size_t)::_InterlockedIncrement((volatile i32*)&p);
}
#endif

inline u16 InterlockedIncrement(volatile u16& p)
{
    return (u16)::_InterlockedIncrement16((volatile i16*)&p);
}

inline u32 InterlockedIncrement(volatile u32& p)
{
    return (u32)::_InterlockedIncrement((volatile i32*)&p);
}

#ifdef TARGET_X64
inline u64 InterlockedIncrement(volatile u64& p)
{
    return (u64)::_InterlockedIncrement64((volatile i64*)&p);
}
#endif

inline i16 InterlockedIncrement(volatile i16& p)
{
    return ::_InterlockedIncrement16(&p);
}

inline i32 InterlockedIncrement(volatile i32& p)
{
    return ::_InterlockedIncrement(&p);
}

#ifdef TARGET_X64
inline i64 InterlockedIncrement(volatile i64& p)
{
    return ::_InterlockedIncrement64(&p);
}
#endif

/*interlocked decrement*/

#ifdef TARGET_X86
inline size_t InterlockedDecrement(volatile size_t& p)
{
    return (size_t)::_InterlockedDecrement((volatile i32*)&p);
}
#endif

inline u32 InterlockedDecrement(volatile u32& p)
{
    return (u32)::_InterlockedDecrement((volatile i32*)&p);
}

#ifdef TARGET_X64
inline u64 InterlockedDecrement(volatile u64& p)
{
    return (u64)::_InterlockedDecrement64((volatile i64*)&p);
}
#endif

inline u16 InterlockedDecrement(volatile u16& p)
{
    return (u16)::_InterlockedDecrement16((volatile i16*)&p);
}

inline i32 InterlockedDecrement(volatile i32& p)
{
    return ::_InterlockedDecrement(&p);
}

#ifdef TARGET_X64
inline i64 InterlockedDecrement(volatile i64& p)
{
    return ::_InterlockedDecrement64(&p);
}
#endif

inline i16 InterlockedDecrement(volatile i16& p)
{
    return ::_InterlockedDecrement16(&p);
}

/*interlocked compare exchange*/

inline unsigned int InterlockedCompareExchange(volatile unsigned int& p,u32 exchange, unsigned int compare)
{
    return (unsigned int)::InterlockedCompareExchange((volatile i32*)&p,(i32)exchange,(i32)compare);
}

inline u32 InterlockedCompareExchange(volatile u32& p,u32 exchange, u32 compare)
{
    return (u32)::InterlockedCompareExchange((volatile i32*)&p,(i32)exchange,(i32)compare);
}

inline u64 InterlockedCompareExchange(volatile u64& p,u64 exchange, u64 compare)
{
    return (u64)::InterlockedCompareExchange64((volatile i64*)&p,(i64)exchange,(i64)compare);
}

inline i32 InterlockedCompareExchange(volatile i32& p,i32 exchange, i32 compare)
{
    return ::InterlockedCompareExchange(&p,exchange,compare);
}

inline i64 InterlockedCompareExchange(volatile i64& p,i64 exchange, i64 compare)
{
    return ::InterlockedCompareExchange64(&p,exchange,compare);
}

template<class _C> inline _C* InterlockedCompareExchange(_C* volatile & p,const _C* exchange,const _C* compare)
{
    return (_C*)::InterlockedCompareExchangePointer((PVOID volatile*)&p,(PVOID)exchange,(PVOID)compare);
}

struct _DoubleWordCompareExchange
{
	up low;
	up high;

	bool operator == (const _DoubleWordCompareExchange& other)
	{
		return low == other.low && high == other.high;
	}
	bool operator != (const _DoubleWordCompareExchange& other)
	{
		return low != other.low || high != other.high;
	}
};

typedef __declspec( align( 16 ) ) _DoubleWordCompareExchange DoubleWordCompareExchange;

inline DoubleWordCompareExchange InterlockedCompareExchange(volatile DoubleWordCompareExchange& p,const DoubleWordCompareExchange& exchange, const DoubleWordCompareExchange& compare)
{
#ifdef TARGET_X64
	DoubleWordCompareExchange result = compare;
    if(::_InterlockedCompareExchange128((i64 volatile*)&p,exchange.high,exchange.low,(i64*)&result))
		return result;
	else
		return result;
#else
	i64 result = InterlockedCompareExchange(*(i64 volatile*)&p,*(i64*)&exchange,*(i64*)&compare);
	return *(DoubleWordCompareExchange*)&result;
#endif
}

/*interlocked exchange*/

inline u32 InterlockedExchange(volatile u32& p,u32 exchange)
{
    return (u32)::InterlockedExchange((volatile i32*)&p,(i32)exchange);
}

inline u64 InterlockedExchange(volatile u64& p,u64 exchange)
{
    return (u64)::InterlockedExchange64((volatile i64*)&p,(i64)exchange);
}

inline i32 InterlockedExchange(volatile i32& p,i32 exchange)
{
    return ::InterlockedExchange(&p,exchange);
}

inline i64 InterlockedExchange(volatile i64& p,i64 exchange)
{
    return ::InterlockedExchange64(&p,exchange);
}

template<class _C> inline _C* InterlockedExchange(_C* volatile & p,const _C* exchange)
{
    return (_C*)::InterlockedExchangePointer((PVOID volatile*)&p,(PVOID)exchange);
}

}
#elif defined(COMPILER_GCC)

namespace Math {

inline u32 InterlockedIncrement(volatile u32* p)
{
    return __sync_add_and_fetch(p,1);
}

inline u64 InterlockedIncrement(volatile u64* p)
{
    return __sync_add_and_fetch(p,1);
}

inline i32 InterlockedIncrement(volatile i32* p)
{
    return __sync_add_and_fetch(p,1);
}

inline i64 InterlockedIncrement(volatile i64* p)
{
    return __sync_add_and_fetch(p,1);
}

inline u32 InterlockedDecrement(volatile u32* p)
{
    return __sync_sub_and_fetch(p,1);
}

inline u64 InterlockedDecrement(volatile u64* p)
{
    return __sync_sub_and_fetch(p,1);
}

inline i32 InterlockedDecrement(volatile i32* p)
{
    return __sync_sub_and_fetch(p,1);
}

inline i64 InterlockedDecrement(volatile i64* p)
{
    return __sync_sub_and_fetch(p,1);
}

}

#else
#error UNSUPPORTED COMPILER
#endif

#endif