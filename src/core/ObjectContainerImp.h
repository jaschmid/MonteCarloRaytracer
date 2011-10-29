/********************************************************/
// FILE: ObjectContainer.h
// DESCRIPTION: Raytracer Object Container
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

#ifndef RAYTRACE_OBJECT_CONTAINER_GUARD
#define RAYTRACE_OBJECT_CONTAINER_GUARD

#include <map>
#include <boost/intrusive_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/intrusive/avl_set.hpp>
#include <boost/intrusive/avl_set_hook.hpp>

namespace Raytrace {
	
	class IObjectContainer;

	class InsertableObject : public boost::intrusive::avl_set_base_hook<boost::intrusive::optimize_size<true>>, virtual public IObject
	{
	public:
		virtual ~InsertableObject()
		{
			assert(_parent == nullptr);
		}

		bool notifyAttach(IObjectContainer* parent)
		{
			if(!_parent && parent)
			{
				_parent = parent;
				return true;
			}
			else
				return false;
		}
		
		bool notifyDetach(IObjectContainer* parent)
		{
			if(_parent && _parent == parent)
			{
				_parent = nullptr;
				return true;
			}
			else
				return false;
		}

		inline const String& getName() const
		{
			return dynamic_cast<const IObject*>(this)->GetName();
		}
		
		struct comparison
		{
		   bool operator() (const InsertableObject &a, const InsertableObject &b) const
			  {  return a.getName() < b.getName();  }
	   
		   bool operator() (const String &a, const InsertableObject &b) const
			  {  return a < b.getName();  }

		   bool operator() (const InsertableObject &a, const String &b) const
			  {  return a.getName() < b;  }
		};

		friend struct comparison;

	protected:
		InsertableObject() : _parent(nullptr)
		{
		}

		IObjectContainer*			_parent;
	};

	template<class _Base> class ObjectContainer : public _Base
	{
	protected:
		template<class _1,class _2> inline ObjectContainer(_1 a,_2 b)  : _Base(a,b)
		{
		}
		template<class _1> inline ObjectContainer(_1 a)  : _Base(a)
		{
		}
		inline ObjectContainer() : _Base()
		{
		}

		inline ~ObjectContainer() 
		{
			RemoveAllObjects();
		}

		inline Object GetObject(const String& name) const
		{
			ObjectDBType::const_iterator it = _objects.find(name,InsertableObject::comparison());
			if(it != _objects.end())
				return Object(const_cast<IObject*>(dynamic_cast<const IObject*>(&*it)));
			else
				return Object();
		}
		inline Result RemoveObject(const Object& o)
		{
			InsertableObject* insert;
			try
			{
				insert = dynamic_cast<InsertableObject*>(o.get());
			}
			catch(const std::bad_cast&)
			{
				return Result::Failed;
			}

			ObjectDBType::iterator it = _objects.iterator_to(*insert);
			if(it != _objects.end())
			{
				if(!insert->notifyDetach(this))
					return Result::Failed;
				_objects.erase(it);
				insert->Release();
				return Result::Succeeded;
			}
			else
				return Result::Failed;
		}
		inline Result InsertObject(const Object& o)
		{
			InsertableObject* insert;
			try
			{
				insert = dynamic_cast<InsertableObject*>(o.get());
			}
			catch(const std::bad_cast&)
			{
				return Result::Failed;
			}

			if(!insert->notifyAttach(this))
				return Result::Failed;

			insert->AddRef();

			_objects.insert(*insert);

			return Result::Succeeded;
		}
		inline Result RemoveAllObjects()
		{
			InsertableObject* item;
			while(_objects.begin() != _objects.end())
			{
				if( (item = dynamic_cast<InsertableObject*>(&*_objects.begin()))->notifyDetach(this))
				{
					_objects.erase(_objects.begin());
					item->Release();
				}
			}
			return Result::Succeeded;
		}
		
		inline Object GetFirstObject() const
		{
			if(!_objects.empty())
				return Object(const_cast<IObject*>(dynamic_cast<const IObject*>(&*_objects.begin())));
			else
				return Object();
		}
		inline Object GetFirstObject(RObjectType type) const
		{
			if(_objects.empty())
				return Object();

			ObjectDBType::const_iterator it = _objects.begin();

			while(it != _objects.end() && it->GetType() != type)
				++it;

			if(it == _objects.end())
				return Object();
			else
				return Object(const_cast<IObject*>(dynamic_cast<const IObject*>(&*it)));
		}
		inline Object GetNextObject(const Object& last) const
		{
			InsertableObject* insert;
			try
			{
				insert = dynamic_cast<InsertableObject*>(last.get());
			}
			catch(const std::bad_cast&)
			{
				return Object();
			}

			ObjectDBType::const_iterator it = _objects.iterator_to(*insert);
			if(it != _objects.end())
			{
				++it;

				if(it != _objects.end())
					return Object(const_cast<IObject*>(dynamic_cast<const IObject*>(&*it)));
				else
					return Object();
			}
			else
				return Object();
		}
		inline Object GetNextObject(const Object& last,RObjectType type) const
		{
			InsertableObject* insert;
			try
			{
				insert = dynamic_cast<InsertableObject*>(last.get());
			}
			catch(const std::bad_cast&)
			{
				return Object();
			}

			ObjectDBType::const_iterator it = _objects.iterator_to(*insert);
			if(it != _objects.end())
			{
				++it;
				
				while(it != _objects.end() && it->GetType() != type)
					++it;

				if(it == _objects.end())
					return Object();
				else
					return Object(const_cast<IObject*>(dynamic_cast<const IObject*>(&*it)));
			}
			else
				return Object();
		}

	protected:
		
		typedef boost::intrusive::avl_multiset<InsertableObject,boost::intrusive::compare<InsertableObject::comparison>> ObjectDBType;
		ObjectDBType				_objects;
	};

}

#endif