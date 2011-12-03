/********************************************************/
// FILE: IIntegrator.h
// DESCRIPTION: Secifies interface for all Integrators (note it's still a template)
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

#ifndef RAYTRACE_ISURFACE_INTEGRATOR_GUARD
#define RAYTRACE_ISURFACE_INTEGRATOR_GUARD

#include <RaytraceCommon.h>

namespace Raytrace
{
	template<class _SampleData,class _RayData,class _SceneReader> struct IIntegrator
	{
		virtual void InitializePrepareST(size_t numThreads,const _SceneReader& scene,_SampleData& sampleData,_RayData& rayData) {}
		virtual void InitializeMT(size_t threadId) {}
		virtual void InitializeCompleteST() {}

		virtual void IntegratePrepareST() {}
		virtual void IntegrateMT(size_t threadId) {}
		virtual bool IntegrateCompleteST() {return true;}

		virtual ~IIntegrator() {}
	};
}

#endif