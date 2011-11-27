/********************************************************/
// FILE: TriMeshImp.h
// DESCRIPTION: Raytracer TriMesh
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

#ifndef RAYTRACE_TRI_MESH_IMP_GUARD
#define RAYTRACE_TRI_MESH_IMP_GUARD

#include <RaytraceTriMesh.h>
#include "ObjectImp.h"
#include "Triangle.h"

namespace Raytrace {

	class TriMeshImp : public ObjectImp<TriMeshImp,ITriMesh>
	{
	public:
		~TriMeshImp();

		int PushVertex(const Vector3& location);
		int PushTriangle(int vertex1,int vertex2,int vertex3,Material material);

		inline int getNumTriangles() const
		{
			return (int)_triangleVertices.size();
		}

		inline void getTriangle(int i,Triangle& t,int& mat) const
		{
			const Vector3i triIndices = _triangleVertices[i];
			t.setPoint(0, _vertexLocations[triIndices[0]]);
			t.setPoint(1, _vertexLocations[triIndices[1]]);
			t.setPoint(2, _vertexLocations[triIndices[2]]);
			mat = _triangleMaterials[i];
		}

		inline int getNumMaterials() const
		{
			return (int)_materials.size();
		}

		inline Material getMaterial(int i) const
		{
			return _materials[i];
		}

	private:

		TriMeshImp(const String& name);

		friend TriMesh CreateTriMesh(const String& name);

		std::vector<Material>			_materials;
		std::vector<Vector3>			_vertexLocations;
		std::vector<Vector3i>			_triangleVertices;
		std::vector<int>				_triangleMaterials;
	};

};

#endif