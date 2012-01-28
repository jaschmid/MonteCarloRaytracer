/********************************************************/
// FILE: MayaSceneReader.h
// DESCRIPTION: Maya Scene Reader class
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

#ifndef RAYTRACE_MAYA_SCENE_READER_GUARD
#define RAYTRACE_MAYA_SCENE_READER_GUARD

#include <RaytraceScene.h>
#include <RaytraceCamera.h>
#include <RaytraceMaterial.h>
#include <RaytraceTriMesh.h>
#include <RaytraceOutput.h>
#include <RaytraceObject.h>
#include <RaytraceXmlParser.h>

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MObject.h>
#include <maya/MFnMesh.h>
#include <maya/MItDag.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>

#include <vector>
#include <deque>
#include <boost/icl/split_interval_map.hpp>
#include <boost/assign.hpp>

#include "PropertySetImp.h"

class MayaSceneReader  : public Raytrace::PropertySetImp<MayaSceneReader,Raytrace::ISceneReader>
{
public:
	static const PropertyMap& GetPropertySet()
	{
		const static PropertyMap set = boost::assign::map_list_of
			(Raytrace::SceneReaderProperty_Resolution,Property(&MayaSceneReader::GetResolution))
			(Raytrace::SceneReaderProperty_FieldOfView,Property(&MayaSceneReader::GetFieldOfView))
			(Raytrace::SceneReaderProperty_Aspect,Property(&MayaSceneReader::GetAspect))
			(Raytrace::SceneReaderProperty_MultisampleCount,Property(&MayaSceneReader::GetMultisampleCount))
			(Raytrace::SceneReaderProperty_PrimitiveType_Triangle,Property(&MayaSceneReader::GetPrimitiveType));
		return set;
	}

	MayaSceneReader();
	~MayaSceneReader();

	size_t				GetNumPrimitives() const;
	void				GetPrimitive(size_t i,void* pPrimitiveOut) const;

	size_t				GetNumMaterials() const;
	void				GetMaterial(size_t i,MaterialData* pMaterialOut) const;

	size_t				GetNumLights() const;
	void				GetLight(size_t i,LightData* pMaterialOut) const;


	void				SetCamera(const MDagPath& camera);
	void				SetResolution(const Raytrace::Vector2u& resolution);

	Raytrace::Vector2u	GetBackgroundRadianceSize() const;
	Format				GetBackgroundRadianceFormat() const;
	void 				GetBackgroundRadianceData(void* pData) const;
private:

	Raytrace::Vector2u GetResolution() const ;
	Raytrace::Real GetFieldOfView() const ;
	Raytrace::Real GetAspect() const ;
	Raytrace::u32 GetMultisampleCount() const ;
	Raytrace::String GetPrimitiveType() const ;

	void parseTriMesh(const MObject& triMesh);
	/*
	void parseMaterial(const MFnMaterial& material)
	{
		_materials.push_back(material);
	}*/
		
	struct MeshContainer
	{
		MObject									_triMesh;
		unsigned int							_primitesPerInstance;
		unsigned int							_numInstances;

		MDagPathArray							_dagPaths;

		MIntArray								_triangleCounts;
		MIntArray								_triangleVertices;

		bool operator ==(const MeshContainer& m2) const
		{
			if(_triMesh == m2._triMesh)
				return true;
			else
				return false;
		}
	};

	typedef boost::icl::split_interval_map<int,MeshContainer> IntervalMap;

	Raytrace::Matrix4							_viewMatrix;
	IntervalMap									_primitives;
	std::vector<MaterialData>					_materials;
	MDagPath									_camera;

	Raytrace::Vector2u							_envSize;
	std::vector<Raytrace::Vector4>				_envData;

	Raytrace::Vector2u	_resolution;
};

#endif
