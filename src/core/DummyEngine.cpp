#include "headers.h"
#include <RaytraceCommon.h>
#include "DummyEngine.h"
#include "Engines.h"

namespace Raytrace {

boost::shared_ptr<RaytraceEngineBase>	CreateDummyEngine(const GenericEngineSettings& settings)
{
	DummyIntersector		dummyInteresctor;
	DummyShader				dummyShader;

	switch(settings._format)
	{
	case ::Math::R8G8B8A8:
		{
			DummySampleGenerator<::Math::ImageRect<::Math::R8G8B8A8>>	dummyGenerator(
				::Math::ImageRect<::Math::R8G8B8A8>(settings._pDataOut,::Math::Vector2<::Math::u32>(settings._iXResolution,settings._iYResolution),settings._nDataSize/settings._iYResolution/sizeof(::Math::Pixel<::Math::R8G8B8A8>))
				);

			return CreateRaytraceEngine(dummyGenerator,dummyInteresctor,dummyShader);
		}
	case ::Math::A8R8G8B8:
		{
			DummySampleGenerator<::Math::ImageRect<::Math::A8R8G8B8>>	dummyGenerator(
				::Math::ImageRect<::Math::A8R8G8B8>(settings._pDataOut,::Math::Vector2<::Math::u32>(settings._iXResolution,settings._iYResolution),settings._nDataSize/settings._iYResolution/sizeof(::Math::Pixel<::Math::A8R8G8B8>))
				);

			return CreateRaytraceEngine(dummyGenerator,dummyInteresctor,dummyShader);
		}
	default:
		return boost::shared_ptr<RaytraceEngineBase>();
	}

}

}