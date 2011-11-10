/********************************************************/
// FILE: MaterialImp.h
// DESCRIPTION: Raytracer Material
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

#ifndef RAYTRACE_MATERIAL_IMP_GUARD
#define RAYTRACE_MATERIAL_IMP_GUARD

#include <RaytraceMaterial.h>
#include <boost/assign.hpp>
#include "ObjectImp.h"

namespace Raytrace {

	class MaterialImp : public ObjectImp<MaterialImp,IMaterial>
	{
	public:
		~MaterialImp();
		
		static const PropertyMap& GetPropertySet()
		{
			const static PropertyMap set = boost::assign::map_list_of
				("IOR",Property(&GetIOR,&SetIOR))
				("emit",Property(&GetEmit,&SetEmit))
				("diffuse_reflect",Property(&GetDiffuseReflect,&SetDiffuseReflect))
				("specular_reflect",Property(&GetSpecularReflect,&SetSpecularReflect))
				("translucency",Property(&GetTranslucency,&SetTranslucency))
				("transmit_filter",Property(&GetTransmitFilter,&SetTransmitFilter))
				("transparency",Property(&GetTransparency,&SetTransparency))
				("fresnel_effect",Property(&GetFresnelEffect,&SetFresnelEffect))
				("color",Property(&GetColor,&SetColor))
				("mirror_color",Property(&GetMirrorColor,&SetMirrorColor));
			return set;
		}

		inline void SetIOR(const Real& f){_ior = f;}
		inline Real GetIOR() const{return _ior;}
		
		inline void SetDiffuseReflect(const Real& f){_diffuseReflect = f;}
		inline Real GetDiffuseReflect() const{return _diffuseReflect;}
		
		inline void SetEmit(const Real& f){_emit = f;}
		inline Real GetEmit() const{return _emit;}
		
		inline void SetSpecularReflect(const Real& f){_specular_reflect = f;}
		inline Real GetSpecularReflect() const{return _specular_reflect;}
		
		inline void SetTranslucency(const Real& f){_translucency = f;}
		inline Real GetTranslucency() const{return _translucency;}
		
		inline void SetTransmitFilter(const Real& f){_transmit_filter = f;}
		inline Real GetTransmitFilter() const{return _transmit_filter;}
		
		inline void SetTransparency(const Real& f){_transparency = f;}
		inline Real GetTransparency() const{return _transparency;}
		
		inline void SetFresnelEffect(const bool& f){_fresnel_effect = f;}
		inline bool GetFresnelEffect() const{return _fresnel_effect;}
		
		inline void SetColor(const Vector4& f){_color = f;}
		inline Vector4 GetColor() const{return _color;}
		
		inline void SetMirrorColor(const Vector4& f){_mirror_color = f;}
		inline Vector4 GetMirrorColor() const{return _mirror_color;}
	private:

		MaterialImp(const String& name);

		friend Material CreateMaterial(const String& name);

		Real	_ior;
		Vector4 _color;
		Real	_diffuseReflect;
		Real	_emit;
		bool	_fresnel_effect;
		Vector4 _mirror_color;
		Real	_specular_reflect;
		Real	_translucency;
		Real	_transmit_filter;
		Real	_transparency;

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

};

#endif