/********************************************************/
// FILE: RaytraceOutput.h
// DESCRIPTION: Raytracer exported Output interface
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

#ifndef RAYTRACE_PROPERTY_SET_GUARD
#define RAYTRACE_PROPERTY_SET_GUARD

#include <RaytraceCommon.h>

namespace Raytrace {

/******************************************/
// Raytracer Property Set Interface
/******************************************/
// 
/******************************************/

	class IPropertySet : public virtual IBase
	{
	public:
		virtual Result SetPropertyValue(const String& prop,const String& value) = 0;
		virtual Result GetPropertyValue(const String& prop,String& value) const = 0;
	};

}

#endif