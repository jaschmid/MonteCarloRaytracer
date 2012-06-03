/********************************************************/
// FILE: RaytraceOutput.h
// DESCRIPTION: Raytracer exported Output interface
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

#ifndef RAYTRACE_OUTPUT_GUARD
#define RAYTRACE_OUTPUT_GUARD

#include <RaytraceCommon.h>
#include <RaytraceObject.h>
#include <RaytraceCustomSceneReader.h>
#include <boost/shared_ptr.hpp>

namespace Raytrace {

/******************************************/
// Raytracer Output Interface
/******************************************/
// 
/******************************************/
	
	extern Output CreateOutput(const String& name = String());
	extern Output CreateCustomOutput(const boost::shared_ptr<ISceneReader>& _sceneReader, const String& name = String());

	class IOutput : public virtual IObject
	{
	public:
		enum Format
		{
			FORMAT_R8G8B8A8,
			FORMAT_A8R8G8B8,
			FORMAT_RGBA_F32
		};

		//implicitly calls refresh
		virtual Result SetOutputSurface(void* pData=nullptr, int nDataSize=0, int xResolution=0, int yResolution=0, Format format=FORMAT_R8G8B8A8) = 0;
		//make a new image
		virtual Result Refresh() = 0;

		//get the time it took to generate the last frame, in milliseconds
		virtual Result GetLastFrameInfo(std::string& info) = 0;

		//updates the output image
		virtual Result UpdateOutput() = 0;

		//property Enabled/bool
		virtual void SetEnabled(const bool& enabled) = 0;
		virtual bool GetEnabled() const = 0;

		//property RenderingEngine/String
		virtual void SetEngine(const String& renderer) = 0;
		virtual String GetEngine() const = 0;

		virtual int GetNumEngines() const = 0;
		virtual String GetEngineName(int i) const = 0;

		//property Integrator/String
		virtual void SetIntegrator(const String& renderer) = 0;
		virtual String GetIntegrator() const = 0;

		virtual int GetNumIntegrators() const = 0;
		virtual String GetIntegratorName(int i) const = 0;

		//property Intersector/String
		virtual void SetIntersector(const String& renderer) = 0;
		virtual String GetIntersector() const = 0;

		virtual int GetNumIntersectors() const = 0;
		virtual String GetIntersectorName(int i) const = 0;

		//property Sampler/String
		virtual void SetSampler(const String& renderer) = 0;
		virtual String GetSampler() const = 0;

		virtual int GetNumSamplers() const = 0;
		virtual String GetSamplerName(int i) const = 0;

		static RObjectType ObjectType;
	};

}

#endif