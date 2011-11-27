/********************************************************/
// FILE: RaytraceTriMesh.h
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

#ifndef RAYTRACE_TRIMESH_GUARD
#define RAYTRACE_TRIMESH_GUARD

#include <RaytraceCommon.h>
#include <RaytraceObject.h>

namespace Raytrace {

/******************************************/
// Raytracer TriMesh Interface
/******************************************/
// 
/******************************************/
	
	extern TriMesh CreateTriMesh(const String& name = String());

	class ITriMesh : public virtual IObject
	{
	public:

		//very primitive right now but sufficient for our purposes

		virtual int PushVertex(const Vector3& location) = 0;
		virtual int PushTriangle(int vertex1,int vertex2,int vertex3,Material material) = 0;

		static RObjectType ObjectType;
	};

}

#endif