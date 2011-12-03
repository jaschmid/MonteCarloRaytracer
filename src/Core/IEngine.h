/********************************************************/
// FILE: IEngine.h
// DESCRIPTION: Specifies an interface for a complete Raytracing Engine (not a template)
// AUTHOR: Jan Schmid (jaschmid@eml.cc)    
/********************************************************/
// This work is licensed under the Creative Commons 
// Attribution-NonCommercial 3.0 Unported License. 
// To view a copy of this license, visit 
// http://creativecommons.org/licenses/by-nc/3.0/ or send 
// a letter to Creative Commons, 444 Castro Street, 
// Suite 900, Mountain View, California, 94041, USA.
/********************************************************/


#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef RAYTRACE_IENGINE_GUARD
#define RAYTRACE_IENGINE_GUARD

#include <RaytraceCommon.h>
#include "ImageWriter.h"

namespace Raytrace
{
	template<class _SceneReader> struct IEngine : public IBase
	{
		virtual Result Initialize(const _SceneReader& scene) {return Result::NotImplemented;}

		virtual Result Begin() {return Result::NotImplemented;}
		virtual Result Pause() {return Result::NotImplemented;}
		virtual Result Resume() {return Result::NotImplemented;}

		virtual Result GetStatus(const String& status_type,String& status_out) const {return Result::NotImplemented;}

		virtual Result GatherPreview(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const {return Result::NotImplemented;}

		virtual Result GetImage(IMAGE_FORMAT format,size_t xRes,size_t yRes,void* pDataOut) const {return Result::NotImplemented;}

		virtual ~IEngine() {}
	};
}

#endif