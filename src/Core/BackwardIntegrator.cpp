#include "headers.h"
#include <RaytraceCommon.h>
#include "Engines.h"

#include "BackwardIntegrator.h"


namespace Raytrace {

template<class _SampleData,class _RayData,class _SceneReader> 
typename boost::shared_ptr<IIntegrator<_SampleData,_RayData,_SceneReader>> CreateBackwardIntegrator()
{
	return boost::shared_ptr<IIntegrator<_SampleData,_RayData,_SceneReader>>(new BackwardIntegrator<_SampleData,_RayData,_SceneReader,64>());
}

template boost::shared_ptr<IIntegrator<DefaultEngine::SampleData,DefaultEngine::RayData,DefaultEngine::SceneReader>>	CreateBackwardIntegrator();

}