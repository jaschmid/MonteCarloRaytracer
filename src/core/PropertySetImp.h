/********************************************************/
// FILE: PropertySetImp.h
// DESCRIPTION: Raytracer Property Set Implementation
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

#ifndef RAYTRACE_PROPERTY_SET_IMP_GUARD
#define RAYTRACE_PROPERTY_SET_IMP_GUARD

#include <RaytracePropertySet.h>
#include <boost/lexical_cast.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive/avl_set.hpp>
#include <boost/intrusive/avl_set_hook.hpp>
#include <boost/algorithm/string.hpp>
#include <map>
#include <vector>

#include "BaseImp.h"

namespace Raytrace {

	template<class _Output,class _Input> inline bool LexicalCast(const _Input& input,_Output& output)
	{
		output = boost::lexical_cast<_Output,_Input>(input);
		return true;
	}
	
	template<> inline bool LexicalCast<bool,String>(const String& input,bool& output)
	{
		if(boost::iequals(input,"true"))
			output = true;
		else if(boost::iequals(input,"false"))
			output = false;
		else
			return false;
		return true;
	}
	
	template<> inline bool LexicalCast<String,bool>(const bool& input,String& output)
	{
		if(input)
			output = String("true");
		else
			output = String("false");
		return true;
	}

	template<> inline bool LexicalCast<String,Vector3>(const Vector3& input,String& output)
	{
		output = boost::lexical_cast<String>(input.x) + " " + boost::lexical_cast<String>(input.y) + " " + boost::lexical_cast<String>(input.z);
		return true;
	}
	
	template<> inline bool LexicalCast<Vector3,String>(const String& input,Vector3& output)
	{
		std::vector<String> elements;
		boost::algorithm::split(elements,boost::algorithm::trim_copy(input),boost::algorithm::is_space(),boost::algorithm::token_compress_on);
		if(elements.size() != 3)
			return false;
		output.x = boost::lexical_cast<Real,String>(elements[0]);
		output.y = boost::lexical_cast<Real,String>(elements[1]);
		output.z = boost::lexical_cast<Real,String>(elements[2]);
		return true;
	}
	
	template<> inline bool LexicalCast<String,Vector4>(const Vector4& input,String& output)
	{
		output = boost::lexical_cast<String>(input.x) + " " + boost::lexical_cast<String>(input.y) + " " + boost::lexical_cast<String>(input.z) + " " + boost::lexical_cast<String>(input.w);
		return true;
	}
	
	template<> inline bool LexicalCast<Vector4,String>(const String& input,Vector4& output)
	{
		std::vector<String> elements;
		boost::algorithm::split(elements,boost::algorithm::trim_copy(input),boost::algorithm::is_space(),boost::algorithm::token_compress_on);
		if(elements.size() != 4)
			return false;
		output.x = boost::lexical_cast<Real,String>(elements[0]);
		output.y = boost::lexical_cast<Real,String>(elements[1]);
		output.z = boost::lexical_cast<Real,String>(elements[2]);
		output.w = boost::lexical_cast<Real,String>(elements[3]);
		return true;
	}

	template<class _Final,class _Base> class PropertySetImp : public BaseImp<_Final,_Base>
	{

	private:
		
		_Final& getFinal(){return *(_Final*)this;}
		const _Final& getFinal() const {return *(const _Final*)this;}
		
	protected:

		class Property
		{
		private:
			struct CastFromString
			{
				virtual ~CastFromString(){}
				virtual bool operator()(_Final* cFinal,const String& in) const = 0;
				virtual CastFromString* copy() const = 0;
			};
			struct CastToString
			{
				virtual ~CastToString(){}
				virtual bool operator()(const _Final* cFinal,String& out) const = 0;
				virtual CastToString* copy() const = 0;
			};
			template<class _Type> struct CastFromStringImp : public CastFromString
			{
				typedef void (_Final::*setter)(const _Type&);
				virtual bool operator()(_Final* cFinal,const String& in) const
				{
					_Type t;
					bool res = LexicalCast<_Type,String>(in,t);
					if(res)
						((*cFinal).*(_s))(t);
					return res;
				}

				CastFromStringImp(setter s) : _s(s)
				{
				}
				~CastFromStringImp() {}
				
				CastFromString* copy() const
				{
					return new CastFromStringImp(_s);
				}

				setter _s;
			};
			template<class _Type> struct CastToStringImp : public CastToString
			{
				typedef _Type (_Final::*getter)() const;
				virtual bool operator()(const _Final* cFinal,String& out) const
				{
					return LexicalCast<String,_Type>(((*cFinal).*(_g))(),out);
				}

				CastToStringImp(getter g) : _g(g)
				{
				}
				~CastToStringImp() {}
				
				CastToString* copy() const
				{
					return new CastToStringImp(_g);
				}

				getter _g;
			};

		public:

			template<class _Type> Property(_Type (_Final::*getter)() const,void (_Final::*setter)(const _Type&)) : 
				_toCast(getter?new CastToStringImp<_Type>(getter):nullptr),_fromCast(setter?new CastFromStringImp<_Type>(setter):nullptr)
			{
			}

			Property(Property&& org) : _toCast(org._toCast),_fromCast(org._fromCast)
			{
			}

			Property(const Property& org) : 
				_toCast((org._toCast.get() != nullptr)?org._toCast->copy():nullptr),
				_fromCast((org._fromCast.get() != nullptr)?org._fromCast->copy():nullptr){}
			
			inline Property& operator =(const Property& org)
			{
				_toCast.reset((org._toCast.get() != nullptr)?org._toCast->copy():nullptr);
				_fromCast.reset((org._fromCast.get() != nullptr)?org._fromCast->copy():nullptr);
				return *this;
			}

			inline bool GetValue(const _Final* final,String& value) const
			{
				if(_toCast.get() != nullptr)
					return (*_toCast)(final,value);
				else
					return false;
			}
			inline bool SetValue(_Final* final,const String& value) const
			{
				if(_toCast.get() != nullptr)
					return (*_fromCast)(final,value);
				else
					return false;
			}
		private:

			std::auto_ptr<CastToString> _toCast;
			std::auto_ptr<CastFromString> _fromCast;
		};

		PropertySetImp() {}
		~PropertySetImp() {}

		//template<class Type> Result RegisterProperty(Type* value,const String& name);
		
		
		typedef std::map<String,Property>     PropertyMap;

		static const PropertyMap& GetPropertySet()
		{
			static PropertyMap set;
			return set;
		};

	public:

		Result SetPropertyValue(const String& prop,const String& value)
		{
			const PropertyMap& set = getFinal().GetPropertySet();
			auto it = set.find(prop);
			if(it != set.end())
			{
				if(it->second.SetValue((_Final*)this,value))
					return Result::Succeeded;
				else
					return Result::Failed;
			}
			else
				return Result::PropertyNotFound;
		}

		Result GetPropertyValue(const String& prop,String& value) const
		{
			const PropertyMap& set = getFinal().GetPropertySet();
			auto it = set.find(prop);
			if(it != set.end())
			{
				if(it->second.GetValue((const _Final*)this,value))
					return Result::Succeeded;
				else
					return Result::Failed;
			}
			else
				return Result::PropertyNotFound;
		}
	};

};

#endif