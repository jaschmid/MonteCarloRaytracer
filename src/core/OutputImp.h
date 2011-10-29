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
#include "ObjectImp.h"
#include "Engines.h"

namespace Raytrace {

	class OutputImp : public ObjectImp<OutputImp,IOutput>
	{
	public:
		~OutputImp();

		Result SetOutputSurface(void* pData, int nDataSize, int xResolution, int yResolution, Format format);
		Result Refresh();
		
		Result UpdateOutput();
		Result GetLastFrameTime(int &);

		//property Enabled/bool
		inline void SetEnabled(const bool& enabled) { _enabled = enabled;}
		inline bool GetEnabled() const {return _enabled;}

		//property AdaptiveRendering/bool
		inline void SetAdaptiveRendering(const bool& adaptive) { _adaptiveRendering = adaptive; }
		inline bool GetAdaptiveRendering() const { return _adaptiveRendering; }

		//property MultisampleCount/int
		inline void SetMultisampleCount(const int& count) { _multisampleCount = count; }
		inline int GetMultisampleCount() const {return _multisampleCount; }

	private:

		typedef ObjectImp<OutputImp,IOutput> Base;

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