/********************************************************/
// FILE: OutputImp.h
// DESCRIPTION: Raytracer Output
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

#ifndef RAYTRACE_OTUPUT_IMP_GUARD
#define RAYTRACE_OTUPUT_IMP_GUARD

#include <RaytraceOutput.h>
#include <boost/assign.hpp>
#include "ObjectImp.h"
#include "Engines.h"



namespace Raytrace {

	class OutputImp : public ObjectImp<OutputImp,IOutput>
	{
	public:
		~OutputImp();
		
		static const PropertyMap& GetPropertySet()
		{
			const static PropertyMap set = boost::assign::map_list_of
				("Enabled",Property(&GetEnabled,&SetEnabled))
				("Intersector",Property(&GetIntersector,&SetIntersector))
				("Integrator",Property(&GetIntegrator,&SetIntegrator))
				("Sampler",Property(&GetSampler,&SetSampler))
				("Engine",Property(&GetEngine,&SetEngine));
			return set;
		}

		Result SetOutputSurface(void* pData, int nDataSize, int xResolution, int yResolution, Format format);
		Result Refresh();
		
		Result UpdateOutput();
		Result GetLastFrameInfo(std::string& info);

		//property Enabled/bool
		inline void SetEnabled(const bool& enabled) { _enabled = enabled;}
		inline bool GetEnabled() const {return _enabled;}

		//property RenderingEngine/string
		inline void SetEngine(const String& engine) { _engine = engine; }
		inline String GetEngine() const { return _engine; }

		inline int GetNumEngines() const { return (int)DefaultEngine::getEngineNames().size(); }
		inline String GetEngineName(int i) const { return DefaultEngine::getEngineNames()[i]; }

		//property RenderingEngine/string
		inline void SetIntegrator(const String& integrator) { _integrator = integrator; }
		inline String GetIntegrator() const { return _integrator; }

		inline int GetNumIntegrators() const { return (int)DefaultEngine::getIntegratorNames().size(); }
		inline String GetIntegratorName(int i) const { return DefaultEngine::getIntegratorNames()[i]; }

		//property RenderingEngine/string
		inline void SetIntersector(const String& intersector) { _intersector = intersector; }
		inline String GetIntersector() const { return _intersector; }

		inline int GetNumIntersectors() const { return (int)DefaultEngine::getIntersectorNames().size(); }
		inline String GetIntersectorName(int i) const { return DefaultEngine::getIntersectorNames()[i]; }

		//property RenderingEngine/string
		inline void SetSampler(const String& sampler) { _sampler = sampler; }
		inline String GetSampler() const { return _sampler; }

		inline int GetNumSamplers() const { return (int)DefaultEngine::getSamplerNames().size(); }
		inline String GetSamplerName(int i) const { return DefaultEngine::getSamplerNames()[i]; }

	private:

		typedef ObjectImp<OutputImp,IOutput> Base;

		String	_engine;
		String	_integrator;
		String	_intersector;
		String	_sampler;

		bool	_enabled;

		IMAGE_FORMAT _outputFormat;
		size_t	_xResOut;
		size_t	_yResOut;
		void*	_pDataOut;
		size_t	_nDataOut;

		DefaultEngine::Engine	_raytraceEngine;

		OutputImp(const String& name);

		friend Output CreateOutput(const String& name);
	};

};

#endif