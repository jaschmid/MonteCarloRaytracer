/********************************************************/
// FILE: RaytraceScene.h
// DESCRIPTION: Raytracer exported Scene interface
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

#ifndef RAYTRACE_SCENE_GUARD
#define RAYTRACE_SCENE_GUARD

#include <RaytraceCommon.h>

namespace Raytrace {

/******************************************/
// Raytracer Scene Interface
/******************************************/
//
/******************************************/

	class IScene
	{
	public:
		virtual void InsertCamera(const Camera& camera) = 0;
		virtual void InsertLight(const Light& light) = 0;
		virtual void InsertObject(const Object& object) = 0;

		static Scene CreateScene(const std::string);
	};

}

#endif