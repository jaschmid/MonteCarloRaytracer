#include "headers.h"
#include <RaytraceCommon.h>
#include "Engines.h"

#include "WhittedIntegrator.h"


namespace Raytrace {

template<class _SampleData,class _RayData,class _SceneReader> 
typename boost::shared_ptr<IIntegrator<_SampleData,_RayData,_SceneReader>> CreateWhittedIntegrator()
{
	return boost::shared_ptr<IIntegrator<_SampleData,_RayData,_SceneReader>>(new WhittedIntegrator<_SampleData,_RayData,_SceneReader,64>());
}

template boost::shared_ptr<IIntegrator<DefaultEngine::SampleData,DefaultEngine::RayData,DefaultEngine::SceneReader>>	CreateWhittedIntegrator();

}