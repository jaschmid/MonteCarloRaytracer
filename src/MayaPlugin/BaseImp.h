/********************************************************/
// FILE: BaseImp.h
// DESCRIPTION: Raytracer Base Reference Counting Implementation
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

#ifndef RAYTRACE_BASE_IMP_GUARD
#define RAYTRACE_BASE_IMP_GUARD

#include <RaytraceCommon.h>

namespace Raytrace {
	
	template<class _Final,class _Base> class BaseImp : public _Base
	{
	public:

		BaseImp() : _count(0)
		{
		}
		
		virtual ~BaseImp()
		{
		}

		inline void AddRef()
		{
			++_count;
		}
		inline void Release()
		{
			if(--_count == 0)
				delete this;
		}

	protected:
		unsigned int					_count;
	};

}

#endif