/********************************************************/
// FILE: RaytraceCommon.h
// DESCRIPTION: Raytracer Common
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

#ifndef RAYTRACE_COMMON_GUARD
#define RAYTRACE_COMMON_GUARD

#include <boost/shared_ptr.hpp>
#include <string>

namespace Raytrace {

	class ICamera;
	class ILight;
	class IObject;
	class IScene;

	typedef boost::shared_ptr<ICamera>	Camera;
	typedef boost::shared_ptr<ILight>	Light;
	typedef boost::shared_ptr<IObject>	Object;
	typedef boost::shared_ptr<IScene>	Scene;
	typedef std::string					String;

}

#endif