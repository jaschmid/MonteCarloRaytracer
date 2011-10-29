/********************************************************/
// FILE: ObjectImp.h
// DESCRIPTION: Raytracer Object Implementation
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

#ifndef RAYTRACE_OBJECT_IMP_GUARD
#define RAYTRACE_OBJECT_IMP_GUARD

#include <RaytracePropertySet.h>
#include <RaytraceObject.h>
#include <boost/weak_ptr.hpp>
#include "PropertySetImp.h"
#include "ObjectContainerImp.h"

namespace Raytrace {
	
	template<class _A,class _B> struct DoubleInherit : public _A,public _B
	{
	};
	
	template<class _Final,class _Base> class ObjectImp : public PropertySetImp<_Final,DoubleInherit<_Base,InsertableObject>>
	{
	public:

		ObjectImp(const String& name) : _name(name)
		{
		}
		
		// detach from parent
		virtual Result Remove()
		{
			return Result::Failed;
		}

		// get object name
		virtual const String& GetName() const
		{
			return _name;
		}

		// get object type
		virtual RObjectType GetType() const
		{
			return _Final::ObjectType;
		}


	protected:
		const String					_name;
	};

}

#endif