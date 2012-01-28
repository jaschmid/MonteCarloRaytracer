#include "headers.h"

#include <math.h>

#include <maya/MSimple.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MPoint.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>

#include "MayaRenderView.h"
#include "MayaGlobals.h"


__declspec(dllexport) MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin( obj, "Jan Schmid", "1.0", "Any");
	MGlobal::displayInfo(MString("Initializing Raytracer"));
	
	status = plugin.registerCommand( "raytrace", MayaRenderView::creator ,MayaRenderView::newSyntax);
	CHECKSTATUS( status, "Can't register RaytracerRenderView command" );
	status.clear();
	
	status = plugin.registerNode( "raytraceGlobalsNode", raytraceGlobalsNode::id, raytraceGlobalsNode::creator, raytraceGlobalsNode::initialize, MPxNode::kDependNode );
	CHECKSTATUS( status, "Can't register raytraceGlobals node" );
	status.clear();

	MString sourceLine("source ");
    sourceLine += "\"Raytrace/RaytraceUI.mel\"";
	status = MGlobal::executeCommand(sourceLine);
	CHECKSTATUS( status, "Can't register RaytracerRenderViewUI command" );

	status = plugin.registerUI("raytraceStartup", "raytraceShutdown");
	CHECKSTATUS( status, "Can't register Raytrace Interface scripts" );

	MGlobal::displayInfo(MString("Raytracer Initialization Complete"));
	return MS::kSuccess;
}

__declspec(dllexport) MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);
	MGlobal::displayInfo(MString("Uninitializing Raytracer"));
	
	status = plugin.deregisterCommand( "raytrace");
	CHECKSTATUS( status, "Can't deregister RaytracerRenderView command" );
	
    status = plugin.deregisterNode( raytraceGlobalsNode::id );
    CHECKSTATUS( status, "Can't deregister raytraceGlobals node" );

	MGlobal::displayInfo(MString("Raytracer Uninitializiation Complete"));
	return MS::kSuccess;
}