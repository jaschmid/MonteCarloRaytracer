/********************************************************/
// FILE: MayaGlobals.h
// DESCRIPTION: Maya Raytrace Globals class
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

#ifndef RAYTRACE_MAYA_RAYTRACE_GLOBALS_NODE_GUARD
#define RAYTRACE_MAYA_RAYTRACE_GLOBALS_NODE_GUARD

#define raytraceGlobalsNodeId		0x00134201

#include <RaytraceScene.h>
#include <RaytraceCamera.h>
#include <RaytraceMaterial.h>
#include <RaytraceTriMesh.h>
#include <RaytraceOutput.h>
#include <RaytraceObject.h>
#include <RaytraceXmlParser.h>

#include <maya/MPxNode.h>
#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MImage.h>
#include <maya/MSwatchRenderBase.h>
#include <maya/MFnDependencyNode.h>

class raytraceGlobalsNode : public MPxNode
{
  public:
                      raytraceGlobalsNode();
    virtual          ~raytraceGlobalsNode();

    static  void *    creator();
    static  MStatus   initialize();

	static MObject get();

    //  Id tag for use with binary file format
    static  MTypeId   id;

	static MObject		multisampleCount;
	static MObject		lightEnvironment;
	static MObject		integrator;
	static MObject		sampler;
	static MObject		intersector;
};

#endif