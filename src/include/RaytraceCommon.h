/********************************************************/
// FILE: RaytraceCommon.h
// DESCRIPTION: Raytracer Common
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

#ifndef RAYTRACE_COMMON_GUARD
#define RAYTRACE_COMMON_GUARD

#include <boost/intrusive_ptr.hpp>
#include <string>
#include "types.h"

#include <Eigen/Eigen>

namespace Raytrace {

	class ICamera;
	class IObject;
	class IScene;
	class IOutput;
	class IMaterial;
	class ITriMesh;
	class IPropertySet;
	class IXmlParser;

	template<class _T> struct ObjectPointer : public boost::intrusive_ptr<_T>
	{
	public:
		explicit ObjectPointer(_T* t) : boost::intrusive_ptr<_T>(t)	{}
		ObjectPointer() {}
		ObjectPointer(const ObjectPointer<_T>& o) : boost::intrusive_ptr<_T>(o) {}
		template<class _A> inline operator ObjectPointer<_A>() const
		{
			return ObjectPointer<_A>(const_cast<_A*>(dynamic_cast<const _A*>(boost::intrusive_ptr<_T>::get())));
		}
	};

	typedef ObjectPointer<ICamera>		Camera;
	typedef ObjectPointer<IMaterial>	Material;
	typedef ObjectPointer<ITriMesh>		TriMesh;
	typedef ObjectPointer<IObject>		Object;
	typedef ObjectPointer<IScene>		Scene;
	typedef ObjectPointer<IOutput>		Output;
	typedef ObjectPointer<IPropertySet>	PropertySet;
	typedef ObjectPointer<IXmlParser>	XmlParser;
	typedef std::string					String;

	typedef f32								Real;
	typedef Eigen::Matrix<Real,2,1>			Vector2;
	typedef Eigen::Matrix<i32,2,1>			Vector2i;
	typedef Eigen::Matrix<u32,2,1>			Vector2u;
	typedef Eigen::Matrix<f32,2,1>			Vector2f;
	typedef Eigen::Matrix<f64,2,1>			Vector2d;
	typedef Eigen::Matrix<Real,3,1>			Vector3;
	typedef Eigen::Matrix<i32,3,1>			Vector3i;
	typedef Eigen::Matrix<u32,3,1>			Vector3u;
	typedef Eigen::Matrix<f32,3,1>			Vector3f;
	typedef Eigen::Matrix<f64,3,1>			Vector3d;
	typedef Eigen::Matrix<Real,4,1>			Vector4;
	typedef Eigen::Matrix<i32,4,1>			Vector4i;
	typedef Eigen::Matrix<u32,4,1>			Vector4u;
	typedef Eigen::Matrix<f32,4,1>			Vector4f;
	typedef Eigen::Matrix<f64,4,1>			Vector4d;
	typedef Eigen::Matrix<Real,4,4>			Matrix4;
	typedef Eigen::Quaternion<Real>			Quaternion;
	/*
	typedef Math::Ray<Real,3>			Ray;
	typedef Math::Triangle<Real,3>		Triangle;
	*/

	class IBase
	{
		public:
		virtual void AddRef() = 0;
		virtual void Release() = 0;
	};


	struct Result
	{
	public:

		struct ResultClass
		{
			explicit ResultClass(unsigned int code,String string) : _code(code),_string(string) {}
		private:
			friend struct Result;
			unsigned int _code;
			String _string;
		};
		
		static const ResultClass Undefined;
		static const ResultClass Succeeded;
		static const ResultClass Failed;
		static const ResultClass UnsupportedObjectType;
		static const ResultClass PropertyNotFound;
		static const ResultClass NoOutputSet;
		static const ResultClass FileNotFound;
		static const ResultClass ParsingError;
		static const ResultClass RenderingInProgress;
		static const ResultClass RenderingComplete;

		Result() :_class(&Undefined){}
		Result(const Result& r) :_class(r._class){}
		Result(const ResultClass& rClass) : _class(&rClass) { }

		// explicit // if you have it!
		inline operator String() const { return _class->_string; }
		inline operator bool() const { return (_class->_code & 0x80000000) != 0; }
		inline operator int() const { return _class->_code; }

		inline bool operator ==(const Result& other) const
		{
			return _class == other._class;
		}
		inline bool operator !=(const Result& other) const
		{
			return _class != other._class;
		}
		friend inline bool operator ==(const Result& a,const ResultClass& b)
		{
			return a._class == &b;
		}
		friend inline bool operator !=(const Result& a,const ResultClass& b)
		{
			return a._class != &b;
		}
		friend inline bool operator ==(const ResultClass& a,const Result& b)
		{
			return &a == b._class;
		}
		friend inline bool operator !=(const ResultClass& a,const Result& b)
		{
			return &a != b._class;
		}

	private:
		
		const ResultClass* _class;
	};
}

namespace boost
{
    template<class _C> inline void intrusive_ptr_add_ref(_C * p)
	{
		p->AddRef();
	}
    template<class _C> inline void intrusive_ptr_release(_C * p)
	{
		p->Release();
	}
}

#endif