#include "headers.h"
#include <RaytraceCommon.h>
#include "Engines.h"

#include "SimpleIntersector.h"

namespace Raytrace {

template<class _RayData,class _SceneReader> 
typename boost::shared_ptr<IIntersector<_RayData,_SceneReader>> CreateSimpleIntersector()
{
	return boost::shared_ptr<IIntersector<_RayData,_SceneReader>>(new SimpleIntersector<_RayData,_SceneReader,4>());
}

template boost::shared_ptr<IIntersector<DefaultEngine::RayData,DefaultEngine::SceneReader>>	CreateSimpleIntersector();

}