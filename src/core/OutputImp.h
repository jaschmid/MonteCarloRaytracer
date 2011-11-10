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
				("AdaptiveRendering",Property(&GetAdaptiveRendering,&SetAdaptiveRendering))
				("MultisampleCount",Property(&GetMultisampleCount,&SetMultisampleCount))
				("RenderingEngine",Property(&GetRenderingEngine,&SetRenderingEngine));
			return set;
		}

		static const std::array<String,3> GetRenderingEngines()
		{
			const std::array<String,3> engines = {
				RenderingEngineDummy,
				RenderingEngineSimple,
				RenderingEngineFast
			};

			return engines;
		}

		Result SetOutputSurface(void* pData, int nDataSize, int xResolution, int yResolution, Format format);
		Result Refresh();
		
		Result UpdateOutput();
		Result GetLastFrameTime(int &);

		inline int GetNumRenderingEngines() const { return GetRenderingEngines().size(); }
		inline String GetRenderingEngineName(int i) const { return GetRenderingEngines()[i]; }

		//property Enabled/bool
		inline void SetEnabled(const bool& enabled) { _enabled = enabled;}
		inline bool GetEnabled() const {return _enabled;}

		//property AdaptiveRendering/bool
		inline void SetAdaptiveRendering(const bool& adaptive) { _adaptiveRendering = adaptive; }
		inline bool GetAdaptiveRendering() const { return _adaptiveRendering; }

		//property MultisampleCount/int
		inline void SetMultisampleCount(const int& count) { _multisampleCount = count; }
		inline int GetMultisampleCount() const {return _multisampleCount; }

		//property RenderingEngine/string
		inline void SetRenderingEngine(const String& engine) { _engine = engine; }
		inline String GetRenderingEngine() const { return _engine; }

	private:

		typedef ObjectImp<OutputImp,IOutput> Base;

		String	_engine;

		bool	_enabled;
		bool	_adaptiveRendering;
		int		_multisampleCount;
		boost::shared_ptr<RaytraceEngineBase>	_raytraceEngine;

		GenericEngineSettings _engineSettings;

		OutputImp(const String& name);

		friend Output CreateOutput(const String& name);
	};

};

#endif