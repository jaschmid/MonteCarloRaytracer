/********************************************************/
// FILE: CameraImp.h
// DESCRIPTION: Raytracer Camera
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

#ifndef RAYTRACE_CAMERA_IMP_GUARD
#define RAYTRACE_CAMERA_IMP_GUARD

#include <RaytraceCamera.h>
#include <boost/assign.hpp>
#include "ObjectImp.h"
#include "ObjectContainerImp.h"

namespace Raytrace {

	class CameraImp : public ObjectContainer<ObjectImp<CameraImp,ICamera>>
	{
	public:
		~CameraImp();

		
		static const PropertyMap& GetPropertySet()
		{
			const static PropertyMap set = boost::assign::map_list_of
				("up",Property(&GetUp,&SetUp))
				("to",Property(&GetTo,&SetTo))
				("from",Property(&GetFrom,&SetFrom))
				("fov",Property(&GetFov,&SetFov))
				("aspect",Property(&GetAspect,&SetAspect))
				("aperture",Property(&GetAperture,&SetAperture))
				("focal",Property(&GetFocal,&SetFocal))
				("type",Property(&GetCameraType,&SetCameraType))
				("bokeh_type",Property(&GetBokehType,&SetBokehType))
				("bokeh_rotation",Property(&GetBokehRotation,&SetBokehRotation));
			return set;
		}

		Result InsertObject(const Object& object);
		
		inline void SetFrom(const Vector3& from){_from = from;}
		inline Vector3 GetFrom() const{return _from;}

		inline void SetTo(const Vector3& to){_to = to;}
		inline Vector3 GetTo() const{return _to;}

		inline void SetUp(const Vector3& up){_up = up;}
		inline Vector3 GetUp() const{return _up;}

		inline void SetFov(const Real& fovX){_fov=fovX;}
		inline Real GetFov() const{return _fov;}
		
		inline void SetAspect(const Real& aspect){_aspect=aspect;}
		inline Real GetAspect() const{return _aspect;}

		inline void SetAperture(const Real& aperture){_aperture=aperture;}
		inline Real GetAperture() const{return _aperture;}
		
		inline void SetFocal(const Real& focal){_focal=focal;}
		inline Real GetFocal() const{return _focal;}
		
		inline void SetCameraType(const String& type){_type=type;}
		inline String GetCameraType() const{return _type;}
		
		inline void SetBokehType(const String& bokeh_type){_bokehType=bokeh_type;}
		inline String GetBokehType() const{return _bokehType;}
		
		inline void SetBokehRotation(const Real& bokehRotation){_bokehRotation=bokehRotation;}
		inline Real GetBokehRotation() const{return _bokehRotation;}
		
		inline IObjectContainer* GetScene() const
		{
			return Base::_parent;
		}

	private:

		typedef ObjectContainer<ObjectImp<CameraImp,ICamera>> Base;

		CameraImp(const String& name);

		friend Camera CreateCamera(const String& name);

		Vector3 _from;
		Vector3 _to;
		Vector3 _up;
		Real	_fov;
		Real	_aspect;
		Real	_aperture;
		Real	_focal;
		String	_type;
		String	_bokehType;
		Real	_bokehRotation;
	};

};

#endif