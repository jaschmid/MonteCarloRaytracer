/********************************************************/
// FILE: RenderView.h
// DESCRIPTION: Maya Render View class
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

#ifndef RAYTRACE_MAYA_RENDER_VIEW_GUARD
#define RAYTRACE_MAYA_RENDER_VIEW_GUARD


#include <RaytraceScene.h>
#include <RaytraceCamera.h>
#include <RaytraceMaterial.h>
#include <RaytraceTriMesh.h>
#include <RaytraceOutput.h>
#include <RaytraceObject.h>
#include <RaytraceXmlParser.h>

#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MObject.h>
#include <boost/thread.hpp>
#include <vector>
#include <deque>


#ifndef CHECKSTATUS
#define CHECKSTATUS(stat,msg) \
  if (!(stat)) { \
    stat.perror((msg)); \
    return (stat); \
  }
#endif

#define CHECKERR(STAT,MSG)       \
    if ( MS::kSuccess != STAT ) {   \
	cerr <<"Failed: " <<MSG << " : "<<STAT<< endl;        \
            return MS::kFailure;    \
    }

class MayaRenderView : public MPxCommand
{
public:
				MayaRenderView();
	virtual		~MayaRenderView();

	MStatus		doIt( const MArgList& );
	MStatus		redoIt();
	MStatus		undoIt();
	bool		isUndoable() const;
	static MSyntax newSyntax();
	static		void* creator();

private:

	struct ThreadLauncher
	{
		inline void operator()()
		{
			_renderView->threadEnter();
		}
		MayaRenderView* _renderView;
	};

	MStatus threadEnter();

	friend struct ThreadLauncher;
	void reloadScene();

	boost::thread			_renderingThread;
	Raytrace::Vector2u		_resolution;
	std::string				_camera;
};

#endif