#include "headers.h"
#include <RaytraceCommon.h>
#include "DummyEngine.h"
#include "Engines.h"

#ifdef DUMMY_ENGINE

namespace Raytrace {

boost::shared_ptr<RaytraceEngineBase>	CreateDummyEngine(const GenericEngineSettings& settings)
{
	DummyIntersector		dummyInteresctor;
	DummyShader				dummyShader;

	SceneReader				sceneReader(settings._scene,settings._camera);

	switch(settings._format)
	{
	case R8G8B8A8:
		{
			DummySampleGenerator<ImageRect<R8G8B8A8>>	dummyGenerator(settings._iXResolution,settings._iYResolution);

			return CreateRaytraceEngine(dummyGenerator,dummyInteresctor,dummyShader,sceneReader);
		}
	case A8R8G8B8:
		{
			DummySampleGenerator<ImageRect<A8R8G8B8>>	dummyGenerator(settings._iXResolution,settings._iYResolution);

			return CreateRaytraceEngine(dummyGenerator,dummyInteresctor,dummyShader,sceneReader);
		}
	default:
		return boost::shared_ptr<RaytraceEngineBase>();
	}

}

}

#endif