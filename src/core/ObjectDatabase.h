/********************************************************/
// FILE: ObjectDatabase.h
// DESCRIPTION: For implementing a Database of objects utilizing ObjectTypes
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

#ifndef RAYTRACE_OBJECTS_DATABASE_GUARD
#define RAYTRACE_OBJECTS_DATABASE_GUARD

#include <RaytraceCommon.h>
#include <boost/mpl/set.hpp>
#include <boost/intrusive/avl_set.hpp>

namespace Raytrace {

	template<class _PairSet> class ObjectDatabase
	{
	public:

		RResult InsertObject(const Object& object);
		RResult RemoveObject(const Object& object);
		
		RResult RemoveAllObjects();
		int GetNumObjects() const;
		const Object& GetFirstObject() const;
		const Object& GetNextObject(const Object& last) const;
		const Object& GetObject(const String& name) const;

		template<class _ObjectType> void RemoveAllObjectsTyped();
		template<class _ObjectType> int GetNumObjectsTyped() const;
		template<class _ObjectType> const boost::shared_ptr<_ObjectType>& GetFirstObjectTyped() const;
		template<class _ObjectType> const boost::shared_ptr<_ObjectType>& GetNextObjectTyped(const boost::shared_ptr<_ObjectType>& last) const;
		template<class _ObjectType> const boost::shared_ptr<_ObjectType>& GetObjectTyped(const String& name) const;

	private:

	};

};

#endif