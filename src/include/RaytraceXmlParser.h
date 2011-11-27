/********************************************************/
// FILE: RaytraceXmlParser.h
// DESCRIPTION: Raytracer exported XmlParser interface
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

#ifndef RAYTRACE_XMLPARSER_GUARD
#define RAYTRACE_XMLPARSER_GUARD

#include <RaytraceCommon.h>

namespace Raytrace {

/******************************************/
// Raytracer Scene Interface
/******************************************/
//
/******************************************/
	
	extern XmlParser CreateXmlParser();

	class IXmlParser : public virtual IBase
	{
	public:
		virtual String GetError() const = 0;
		virtual Result ParseFile(const String& path,Scene& output) = 0;
	};

}

#endif