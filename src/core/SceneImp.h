/********************************************************/
// FILE: SceneImp.h
// DESCRIPTION: Raytracer Scene Implementation
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

#ifndef RAYTRACE_SCENE_IMP_GUARD
#define RAYTRACE_SCENE_IMP_GUARD

#include <RaytraceScene.h>
#include <map>
#include "PropertySetImp.h"
#include "ObjectContainerImp.h"

namespace Raytrace {

	class CameraImp;
	class TrimeshImp;

	class SceneImp : public ObjectContainer<PropertySetImp<SceneImp,IScene>>
	{
	public:

		~SceneImp();
		const String& GetName() const;

		Result InsertObject(const Object& object);

	private:

		typedef ObjectContainer<PropertySetImp<SceneImp,IScene>> Base;

		SceneImp(const String& name);
		friend Scene CreateScene(const String& name);

		String						_name;
	};

};

#endif