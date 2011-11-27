/********************************************************/
// FILE: IntersectorBase.h
// DESCRIPTION: Base Intersector resolution struct
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

#ifndef RAYTRACE_INTERSECTOR_BASE_GUARD
#define RAYTRACE_INTERSECTOR_BASE_GUARD

#include <RaytraceCommon.h>
#include "TemplateOptions.h"

namespace Raytrace {
	
	template<class _Type> struct PrimitiveType;
	template<class _Type> struct RayType;
	template<int _RayCount> struct RayCount;
	template<int _PrimitiveCount>struct  PrimitiveCount;
	struct RayModeConstant;
	struct RayModeUpdateMinimum;

	struct Tag_PrimitiveType
	{
		typedef PrimitiveType<void> DefaultValue;
	};
	
	struct Tag_RayType
	{
		typedef RayType<void> DefaultValue;
	};
	
	struct Tag_PrimitiveCount
	{
		typedef PrimitiveCount<1> DefaultValue;
	};
	
	struct Tag_RayCount
	{
		typedef RayCount<1> DefaultValue;
	};
	
	struct Tag_RayMode
	{
		typedef RayModeUpdateMinimum DefaultValue;
	};

	template<class _Type> struct PrimitiveType
	{
		typedef _Type type;
		typedef typename _Type::PrimitiveClass primitiveClass;
		typedef Tag_PrimitiveType Tag;
	};
	
	template<> struct PrimitiveType<void>
	{
		typedef void type;
		typedef void primitiveClass;
		typedef Tag_PrimitiveType Tag;
	};
	template<class _Type> struct RayType
	{
		typedef _Type type;
		typedef Tag_RayType Tag;
	};
	
	template<> struct RayType<void>
	{
		typedef void type;
		typedef Tag_RayType Tag;
	};
	template<int _RayCount> struct RayCount
	{
		static const int value = _RayCount;
		typedef Tag_RayCount Tag;
	};

	template<int _PrimitiveCount>struct  PrimitiveCount
	{
		static const int value = _PrimitiveCount;
		typedef Tag_PrimitiveCount Tag;
	};
	
	struct RayModeConstant
	{
		typedef RayModeConstant type;
		typedef Tag_RayMode Tag;
	};

	struct RayModeUpdateMinimum
	{
		typedef RayModeUpdateMinimum type;
		typedef Tag_RayMode Tag;
	};

	namespace detail{

		template<class _Options,class PrimitiveType> struct IntersectorResolver
		{
			IntersectorResolver()
			{
				static_assert(false,"Unknown Primitive Type");
			}

			typedef void type;
		};
		
		template<class _Options> struct IntersectorResolver<_Options,void>
		{
			IntersectorResolver()
			{
				static_assert(false,"Specify a Primitive Type");
			}

			typedef void type;
		};
	}

	template<class _Options> struct Intersector
	{
		typedef typename detail::IntersectorResolver< _Options, typename getOptionByTag<_Options,Tag_PrimitiveType>::type::primitiveClass>::type type;
	};
	
	struct PrimitiveClassRay {};
	struct PrimitiveClassAxisAlignedBox {};
	struct PrimitiveClassTriangle {};
	struct PrimitiveClassSphere {};

}

#endif
