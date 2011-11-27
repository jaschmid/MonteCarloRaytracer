/********************************************************/
// FILE: RaytraceCamera.h
// DESCRIPTION: Raytracer exported Camera interface
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

#ifndef RAYTRACE_CAMERA_GUARD
#define RAYTRACE_CAMERA_GUARD

#include <RaytraceCommon.h>
#include <RaytraceObject.h>

namespace Raytrace {

/******************************************/
// Raytracer Output Interface
/******************************************/
// 
/******************************************/
	
	extern Camera CreateCamera(const String& name = String());

	class ICamera : public virtual IObject, public virtual IObjectContainer
	{
	public:
		// property From/Vector3
		virtual void SetFrom(const Vector3& from) = 0;
		virtual Vector3 GetFrom() const = 0;

		// property To/Vector3
		virtual void SetTo(const Vector3& from) = 0;
		virtual Vector3 GetTo() const = 0;

		// property Up/Vector3
		virtual void SetUp(const Vector3& from) = 0;
		virtual Vector3 GetUp() const = 0;

		// property Fov/Real
		virtual void SetFov(const Real& fovX) =0;
		virtual Real GetFov() const =0;
		
		// property Aspect/Real
		virtual void SetAspect(const Real& aspect) =0;
		virtual Real GetAspect() const =0;

		// property Aperture/Real
		virtual void SetAperture(const Real& aperture) =0;
		virtual Real GetAperture() const =0;
		
		// property Focal/Real
		virtual void SetFocal(const Real& focal) =0;
		virtual Real GetFocal() const =0;
		
		// property Camera/String
		virtual void SetCameraType(const String& type) =0;
		virtual String GetCameraType() const =0;
		
		// property Bokeh_Type/String
		virtual void SetBokehType(const String& bokeh_type) =0;
		virtual String GetBokehType() const =0;
		
		// property Bokeh_Rotation/Real
		virtual void SetBokehRotation(const Real& bokehRotation) =0;
		virtual Real GetBokehRotation() const =0;

		static RObjectType ObjectType;
	};

}

#endif