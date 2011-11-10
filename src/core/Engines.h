/********************************************************/
// FILE: EngineBase.h
// DESCRIPTION: Raytracer Engine
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

#ifndef RAYTRACE_ENGINES_GUARD
#define RAYTRACE_ENGINES_GUARD

#include <boost/shared_ptr.hpp>
#include <RaytraceCommon.h>
#include <RaytraceOutput.h>
#include <RaytraceCamera.h>
#include <RaytraceScene.h>
#include "EngineBase.h"
#include "ImageWriter.h"


namespace Raytrace {

struct GenericEngineSettings
{
	void*	_pDataOut;
	int		_nDataSize;
	int		_iXResolution;
	int		_iYResolution;
	IMAGE_FORMAT	_format;
	Scene	_scene;
	Camera	_camera;
	Output	_output;
};


static const String RenderingEngineDummy = "Dummy";
static const String RenderingEngineSimple = "Simple";
static const String RenderingEngineFast = "Fast";

boost::shared_ptr<RaytraceEngineBase>	CreateDummyEngine(const GenericEngineSettings& settings);
boost::shared_ptr<RaytraceEngineBase>	CreateSimpleEngine(const GenericEngineSettings& settings);
boost::shared_ptr<RaytraceEngineBase>	CreateFastEngine(const GenericEngineSettings& settings);

}

#endif