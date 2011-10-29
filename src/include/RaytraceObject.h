/********************************************************/
// FILE: RaytraceObject.h
// DESCRIPTION: Raytracer exported Object interface
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

#ifndef RAYTRACE_OBJECT_GUARD
#define RAYTRACE_OBJECT_GUARD

#include <RaytraceCommon.h>
#include <RaytracePropertySet.h>

namespace Raytrace {
	
	struct ObjectType
	{
		static const ObjectType Camera;
		static const ObjectType Output;
		static const ObjectType TriMesh;
		static const ObjectType Material;

		explicit ObjectType(int v,const String& name) : _value(v),_name(name) { }

		// explicit // if you have it!
		   operator int() const { return _value; }
		   operator String() const { return _name; }

	private:

		const int _value;
		const String& _name;
	};

	typedef const ObjectType& RObjectType;

	class IObjectContainer : public virtual IBase
	{
	public:
		virtual Result InsertObject(const Object& object) = 0;
		virtual Result RemoveObject(const Object& object) = 0;
		virtual Result RemoveAllObjects() = 0;

		virtual Object GetFirstObject() const = 0;
		virtual Object GetFirstObject(RObjectType type) const = 0;
		virtual Object GetNextObject(const Object& last) const = 0;
		virtual Object GetNextObject(const Object& last,RObjectType type) const = 0;

		virtual Object GetObject(const String& name) const = 0;
	};

	class IObject : public virtual IPropertySet
	{
	public:
		// detach from parent
		virtual Result Remove() = 0;

		// get object name
		virtual const String& GetName() const = 0;

		// get object type
		virtual RObjectType GetType() const = 0;
	};

}

#endif