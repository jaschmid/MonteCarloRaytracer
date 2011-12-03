#include "headers.h"
#include <RaytraceCommon.h>
#include "Engines.h"
#include "SimpleGenerator.h"
#include "BVHIntersector.h"
#include "WhittedShader.h"

#ifdef FAST_ENGINE

namespace Raytrace {

boost::shared_ptr<RaytraceEngineBase>	CreateFastEngine(const GenericEngineSettings& settings)
{
	BVHIntersector			intersector;
	WhittedShader			whittedShader;

	SceneReader				sceneReader(settings._scene,settings._camera);

	switch(settings._format)
	{
	case R8G8B8A8:
		{
			SimpleSampleGenerator<ImageRect<R8G8B8A8>>	simpleGenerator(settings._iXResolution,settings._iYResolution);
			simpleGenerator._camera = settings._camera;
			simpleGenerator._output = settings._output;

			return CreateRaytraceEngine(simpleGenerator,intersector,whittedShader,sceneReader);
		}
	case A8R8G8B8:
		{
			SimpleSampleGenerator<ImageRect<A8R8G8B8>>	simpleGenerator(settings._iXResolution,settings._iYResolution);
			simpleGenerator._camera = settings._camera;
			simpleGenerator._output = settings._output;

			return CreateRaytraceEngine(simpleGenerator,intersector,whittedShader,sceneReader);
		}
	default:
		return boost::shared_ptr<RaytraceEngineBase>();
	}

}

}

#endif