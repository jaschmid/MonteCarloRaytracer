#include "headers.h"
#include <RaytraceCommon.h>
#include "DummyEngine.h"
#include "Engines.h"

namespace Raytrace {

boost::shared_ptr<RaytraceEngineBase>	CreateDummyEngine(const GenericEngineSettings& settings)
{
	DummyIntersector		dummyInteresctor;
	DummyShader				dummyShader;

	SceneReader				sceneReader(settings._scene);

	switch(settings._format)
	{
	case R8G8B8A8:
		{
			DummySampleGenerator<ImageRect<R8G8B8A8>>	dummyGenerator(
				ImageRect<R8G8B8A8>(settings._pDataOut,Vector2u(settings._iXResolution,settings._iYResolution),settings._nDataSize/settings._iYResolution/sizeof(Pixel<R8G8B8A8>))
				);

			return CreateRaytraceEngine(dummyGenerator,dummyInteresctor,dummyShader,sceneReader);
		}
	case A8R8G8B8:
		{
			DummySampleGenerator<ImageRect<A8R8G8B8>>	dummyGenerator(
				ImageRect<A8R8G8B8>(settings._pDataOut,Vector2u(settings._iXResolution,settings._iYResolution),settings._nDataSize/settings._iYResolution/sizeof(Pixel<A8R8G8B8>))
				);

			return CreateRaytraceEngine(dummyGenerator,dummyInteresctor,dummyShader,sceneReader);
		}
	default:
		return boost::shared_ptr<RaytraceEngineBase>();
	}

}

}