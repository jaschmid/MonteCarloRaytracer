/********************************************************/
// FILE: XmlParserImp.h
// DESCRIPTION: Raytracer Xml Parser
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

#ifndef RAYTRACE_XML_PARSER_IMP_GUARD
#define RAYTRACE_XML_PARSER_IMP_GUARD

#include <RaytraceXmlParser.h>
#include "BaseImp.h"
#include <../TinyXml/tinyxml.h>

namespace Raytrace {

	class XmlParserImp : public BaseImp<XmlParserImp,IXmlParser>
	{
	public:
		~XmlParserImp();
		
		String GetError() const
		{
			return _error;
		}
		Result ParseFile(const String& path,Scene& output);

	private:

		bool readProperty(const TiXmlElement& e,String& outProp);
		bool readCameraNode(const TiXmlElement& e,Camera& outNode);
		bool readSceneNode(const TiXmlElement& e,Scene& scene);
		bool readTriMeshNode(const TiXmlElement& e,TriMesh& trimesh);
		bool readMaterialNode(const TiXmlElement& e,Material& material);

		XmlParserImp();

		friend XmlParser CreateXmlParser();

		Scene	_currentScene;
		String	_error;
	};

};

#endif