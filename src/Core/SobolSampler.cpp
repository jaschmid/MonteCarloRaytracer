#include "headers.h"
#include <RaytraceCommon.h>
#include "Engines.h"

#include "SobolSampler.h"

namespace Raytrace {

template<class _SampleData,class _SceneReader> 
typename boost::shared_ptr<ISampler<_SampleData,_SceneReader>> CreateSobolSampler()
{
	return boost::shared_ptr<ISampler<_SampleData,_SceneReader>>(new SobolSampler<_SampleData,_SceneReader>());
}

template boost::shared_ptr<ISampler<DefaultEngine::SampleData,DefaultEngine::SceneReader>>	CreateSobolSampler();

}