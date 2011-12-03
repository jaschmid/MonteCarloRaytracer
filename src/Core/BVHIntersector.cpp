#include "headers.h"
#include <RaytraceCommon.h>
#include "Engines.h"

#include "BVHIntersector.h"

namespace Raytrace {
	
template<class _RayData,class _SceneReader> 
typename boost::shared_ptr<IIntersector<_RayData,_SceneReader>> CreateBVHIntersector()
{
	return boost::shared_ptr<IIntersector<_RayData,_SceneReader>>(new BVHIntersector<_RayData,_SceneReader,4,1,1>());
}

template boost::shared_ptr<IIntersector<DefaultEngine::RayData,DefaultEngine::SceneReader>>	CreateBVHIntersector();

}