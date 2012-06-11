#include "headers.h"

#include <maya/MArgDatabase.h>
#include <maya/MGlobal.h>
#include <maya/MRenderView.h>
#include <maya/MSelectionList.h>
#include <maya/MComputation.h>
#include <maya/M3dView.h>
#include <maya/MPlug.h>
#include <maya/MFnDependencyNode.h>
#include <stdio.h>
#include <errno.h>
#include "MayaRenderView.h"
#include "MayaSceneReader.h"
#include "MayaGlobals.h"

#include <vector>

#include <string>
#include <iostream>

#include <vector>
#include <string>

using namespace std;


inline static MObject getNode(MString name,MStatus *returnStatus)
{
	MObject node;
	MSelectionList list;

	*returnStatus=MGlobal::getSelectionListByName(name,list);

	if(MS::kSuccess!=*returnStatus){
		ERROR("[raytraceMayaRenderView] Cound't get node :"+ name +". There might be multiple nodes called "+name);
		return node;
	}

	*returnStatus=list.getDependNode(0,node);

	if(MS::kSuccess!=*returnStatus){
		ERROR("[raytraceMayaRenderView] Cound't get node :"+ name +". There might be multiple nodes called "+name);
		return MObject::kNullObj;
	}
	return node;
}

MayaRenderView::MayaRenderView()
{
	_camera = std::string("");
	_resolution.x() = 640;
	_resolution.y() = 480;
}

MayaRenderView::~MayaRenderView()
{
}

MStatus MayaRenderView::doIt( const MArgList& args)
{
    MArgDatabase argData(syntax(), args);
	if(argData.isFlagSet( "-camera"))
	{
		MString camera;
		argData.getFlagArgument("-camera", 0, camera);
		_camera = std::string(camera.asChar());
	}
	if(argData.isFlagSet( "-width"))
	{
		unsigned int width;
		argData.getFlagArgument("-width",0,width);
		_resolution.x() = width;
	}
	if(argData.isFlagSet( "-height"))
	{
		unsigned int height;
		argData.getFlagArgument("-height",0,height);
		_resolution.y() = height;
	}

	return redoIt();
}

inline void getCustomAttribute(MString &strAttribute, MString attribute, MFnDependencyNode &node)
{
	MPlug paramPlug;
	MStatus status;
	paramPlug = node.findPlug( attribute, &status );
	if ( status == MS::kSuccess ) {
		paramPlug.getValue( strAttribute );
	}else{
		strAttribute="";
	}
	return;
}

MStatus MayaRenderView::redoIt()
{
    MStatus retStatus;
	//get the camera
	MDagPath camera;
	if(_camera != std::string("")){
		MObject node = getNode(MString(_camera.c_str()),&retStatus);
		camera.getAPathTo(node);
	}
	else
	{
		M3dView view = M3dView::active3dView(&retStatus);
		CHECKERR(retStatus,"M3dView::active3dView");
		retStatus = view.getCamera(camera);
		CHECKERR(retStatus,"M3dView::active3dView");
	}
	
	
	MString integrator;
	MFnDependencyNode globals(raytraceGlobalsNode::get());
	getCustomAttribute(integrator, "integrator", globals);

	cout << "Integrator: " << integrator.asChar() << endl;
	
	MString intersector;
	getCustomAttribute(intersector, "intersector", globals);

	cout << "Intersector: " << intersector.asChar() << endl;
	
	MString sampler;
	getCustomAttribute(sampler, "sampler", globals);

	cout << "Sampler: " << sampler.asChar() << endl;

	//set the renderview
	retStatus = MRenderView::setCurrentCamera (camera );
	CHECKERR(retStatus,"MRenderView::setCurrentCamera (camera )");
	
	cout << "Camera: " << camera.fullPathName().asChar() << endl;
	
	int xSize = _resolution.x(),ySize = _resolution.y();

	// raytrace things
	MayaSceneReader* mayaReader(new MayaSceneReader());

	boost::shared_ptr<Raytrace::ISceneReader> sceneReader(mayaReader);

	mayaReader->SetResolution(_resolution);
	mayaReader->SetCamera(camera);

	Raytrace::Output		output = Raytrace::CreateCustomOutput(sceneReader,"mayaOutput");


	RV_PIXEL* pixels = new RV_PIXEL[xSize*ySize];

	MRenderView::startRender ( xSize, ySize, true, true );

	output->SetIntegrator(integrator.asChar());
	output->SetIntersector(intersector.asChar());
	output->SetSampler(sampler.asChar());

	output->SetOutputSurface(pixels, sizeof(RV_PIXEL)*xSize*ySize, xSize, ySize, Raytrace::IOutput::FORMAT_RGBA_F32);

	output->Refresh();

	std::cout << "Begin Rendering" << std::endl;
	
	Raytrace::String status;

	while(true)
	{
		if(output->GetLastFrameInfo(status) == Raytrace::Result::RenderingComplete)
			break;
		
		std::cout << "Update Output : " << status << std::endl;
		
		output->UpdateOutput();
		MRenderView::updatePixels(0,xSize-1,0,ySize-1, pixels,true);
		MRenderView::refresh(0,xSize-1,0,ySize-1);
		Sleep(1000);
	}
	
	std::cout << "Complete Rendering: " << status << std::endl;
	
	output->UpdateOutput();
	MRenderView::updatePixels(0,xSize-1,0,ySize-1, pixels,true);

	MRenderView::endRender();

    return MS::kSuccess;
}

MStatus MayaRenderView::undoIt()
{
    return MS::kSuccess;
}

bool MayaRenderView::isUndoable() const
{
    return false;
}

MSyntax MayaRenderView::newSyntax()
{
    MSyntax syntax;

    syntax.enableQuery(true);

	syntax.addFlag("-c","-camera",MSyntax::kString);
	syntax.addFlag("-w","-width",MSyntax::kUnsigned);
	syntax.addFlag("-h","-height",MSyntax::kUnsigned);

    syntax.useSelectionAsDefault(false);

    return syntax;
}

void* MayaRenderView::creator()
{
	return new MayaRenderView();
}
