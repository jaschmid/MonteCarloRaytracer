/********************************************************/
// FILE: RaytraceMaterial.h
// DESCRIPTION: Raytracer exported Material interface
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

#ifndef RAYTRACE_MATERIAL_GUARD
#define RAYTRACE_MATERIAL_GUARD

#include <RaytraceCommon.h>
#include <RaytraceObject.h>

namespace Raytrace {

/******************************************/
// Raytracer Material Interface
/******************************************/
// 
/******************************************/
	
	extern Material CreateMaterial(const String& name = String());

	class IMaterial : public virtual IObject
	{
	public:
		
		virtual void SetIOR(const Real& f) = 0;
		virtual Real GetIOR() const = 0;
		
		virtual void SetDiffuseReflect(const Real& f) = 0;
		virtual Real GetDiffuseReflect() const = 0;
		
		virtual void SetEmit(const Real& f) = 0;
		virtual Real GetEmit() const = 0;
		
		virtual void SetSpecularReflect(const Real& f) = 0;
		virtual Real GetSpecularReflect() const = 0;
		
		virtual void SetTranslucency(const Real& f) = 0;
		virtual Real GetTranslucency() const = 0;
		
		virtual void SetTransmitFilter(const Real& f) = 0;
		virtual Real GetTransmitFilter() const = 0;
		
		virtual void SetTransparency(const Real& f) = 0;
		virtual Real GetTransparency() const = 0;
		
		virtual void SetFresnelEffect(const bool& f) = 0;
		virtual bool GetFresnelEffect() const = 0;

		virtual void SetColor(const Vector4& f) = 0;
		virtual Vector4 GetColor() const = 0;
		
		virtual void SetMirrorColor(const Vector4& f) = 0;
		virtual Vector4 GetMirrorColor() const = 0;

		static RObjectType ObjectType;
	};

}

#endif