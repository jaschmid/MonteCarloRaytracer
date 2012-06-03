/********************************************************/
// FILE: headers.h
// DESCRIPTION: Raytracer Core precompiled headers include
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

#ifndef RAYTRACE_CORE_HEADERS_GUARD
#define RAYTRACE_CORE_HEADERS_GUARD

#ifdef __INTEL_COMPILER
#pragma warning(disable:2586)
#endif 

#pragma warning(disable:4996)
#include <RaytraceScene.h>
#include <RaytraceCamera.h>
#include <RaytraceMaterial.h>
#include <RaytraceTriMesh.h>
#include <RaytraceObject.h>
#include <RaytraceXmlParser.h>

#include "Aligned.h"

#include <../TinyXml/tinyxml.h>

#endif