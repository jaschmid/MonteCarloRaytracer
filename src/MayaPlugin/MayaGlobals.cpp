#include "headers.h"
#include "MayaGlobals.h"
#include <maya/MItDependencyNodes.h>
#include <maya/MDagModifier.h>
#include <maya/MFnStringData.h>

#define CREATE_INT(attr, obj, name, shortName, default)    \
	obj = attr.create( name, shortName, MFnNumericData::kInt, default, &status); \
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(true));      \
    CHECK_MSTATUS(addAttribute(obj));

#define CREATE_STRING(attr, obj, name, shortName, default)    \
	{ MFnStringData fnStringData; MObject  defaultString = fnStringData.create( default );      \
	obj = attr.create( name, shortName, MFnData::kString, defaultString , &status); }\
    CHECK_MSTATUS(attr.setKeyable(true));     \
    CHECK_MSTATUS(attr.setStorable(true));    \
    CHECK_MSTATUS(attr.setReadable(true));    \
    CHECK_MSTATUS(attr.setWritable(true));    \
    CHECK_MSTATUS(attr.setHidden(true));      \
    CHECK_MSTATUS(addAttribute(obj));

raytraceGlobalsNode::raytraceGlobalsNode()
{
}

raytraceGlobalsNode::~raytraceGlobalsNode()
{
}

void* raytraceGlobalsNode::creator()
{
    return new raytraceGlobalsNode();
}

MObject raytraceGlobalsNode::get()
{
	MStatus status;
	for (MItDependencyNodes it(MFn::kPluginDependNode ); !it.isDone(&status); it.next()) 
	{		
		MObject obj = it.item();		
		MFnDependencyNode globals(obj,&status);
		if (!status) 
		{ 
			status.perror("MFnDependencyNode globals"); 
			continue;
		}
		if( globals.typeName() != "raytraceGlobalsNode" )
			continue;
		
		return obj;
	}

	//create new node
	MDagModifier dagModifier;

	MObject newNode = dagModifier.createNode( "raytraceGlobalsNode", MObject::kNullObj );
	dagModifier.doIt();

	return newNode;
}

MStatus   raytraceGlobalsNode::initialize()
{
	MFnTypedAttribute     tAttr;
	MFnNumericAttribute   nAttr;
	MStatus status;

	CREATE_INT( nAttr,  multisampleCount,           "multisampleCount",                "mspc",   1     );
	
	CREATE_STRING( tAttr,  lightEnvironment,        "lightEnvironment",              "lenv",   ""    );
	CREATE_STRING( tAttr,  integrator,              "integrator",                   "rint",   "Backward Integrator"    );
	CREATE_STRING( tAttr,  sampler,                 "sampler",						"rsam",   "Sobol Sampler"    );
	CREATE_STRING( tAttr,  intersector,             "intersector",                   "rsec",   "BVH Intersector"    );

	return MS::kSuccess;
}

MObject raytraceGlobalsNode::multisampleCount;
MObject raytraceGlobalsNode::lightEnvironment;
MObject raytraceGlobalsNode::integrator;
MObject raytraceGlobalsNode::sampler;
MObject raytraceGlobalsNode::intersector;


MTypeId raytraceGlobalsNode::id( raytraceGlobalsNodeId );