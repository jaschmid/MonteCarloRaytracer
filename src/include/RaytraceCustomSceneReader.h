/********************************************************/
// FILE: RaytraceCustomSceneReader.h
// DESCRIPTION: Raytracer custom scene reader, for user supplied scene data
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

#ifndef RAYTRACE_CUSTOM_SCENE_READER_GUARD
#define RAYTRACE_CUSTOM_SCENE_READER_GUARD

#include <RaytraceCommon.h>
#include <RaytraceObject.h>

namespace Raytrace {

/******************************************/
// Raytracer CustomSceneReader Interface
/******************************************/
// 
/******************************************/


	static const String		SceneReaderProperty_Resolution("Resolution");
	static const String		SceneReaderProperty_FieldOfView("FieldOfView");
	static const String		SceneReaderProperty_Aspect("Aspect");
	static const String		SceneReaderProperty_MultisampleCount("MultisampleCount");
	static const String		SceneReaderProperty_PrimitiveType("PrimitiveType");
	static const String		SceneReaderProperty_PrimitiveType_Triangle("PrimitiveType_Triangle");

	class ISceneReader : public IPropertySet
	{
	public:

		struct PrimitiveTriangle
		{
			Vector3		_p1,_p2,_p3;
			int			_material;
		};

		struct MaterialData
		{
			Real	_ior;
			Vector3 _color;
			Real	_diffuseReflect;
			Real	_emit;
			bool	_fresnel_effect;
			Vector3 _mirror_color;
			Real	_specular_reflect;
			Real	_translucency;
			Real	_transmit_filter;
			Real	_transparency;
		};

		struct LightData
		{
			// only point lights supported
			Vector3		_color;
			Vector3		_location;
		};
		
		enum Format
		{
			BACKGROUND_FORMAT_RGBA_F32
		};

		virtual ~ISceneReader(){}

		virtual size_t			GetNumPrimitives() const = 0;
		virtual void			GetPrimitive(size_t i,void* pPrimitiveOut) const = 0;

		virtual size_t			GetNumMaterials() const = 0;
		virtual void			GetMaterial(size_t i,MaterialData* pMaterialOut) const = 0;

		virtual size_t			GetNumLights() const = 0;
		virtual void			GetLight(size_t i,LightData* pMaterialOut) const = 0;

		virtual Vector2u		GetBackgroundRadianceSize() const = 0;
		virtual Format			GetBackgroundRadianceFormat() const = 0;
		virtual void 			GetBackgroundRadianceData(void* pData) const = 0;
	};

}

#endif