/********************************************************/
// FILE: EngineBase.h
// DESCRIPTION: Raytracer Engine
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

#ifndef RAYTRACE_ENGINES_GUARD
#define RAYTRACE_ENGINES_GUARD

#include <boost/shared_ptr.hpp>
#include <RaytraceCommon.h>
#include <RaytraceOutput.h>
#include <RaytraceCamera.h>
#include <RaytraceScene.h>
#include <functional>
#include <map>
#include <boost/assign.hpp>

#include "Triangle.h"
#include "Ray.h"
#include "TemplateOptions.h"

#include "IEngine.h"
#include "IIntegrator.h"
#include "IIntersector.h"
#include "ISampler.h"

namespace Raytrace {

namespace assign = boost::assign;
	

// ray Data
template<class _RayType,class _PrimitiveType,int _RaysPerBlock> struct SimpleRayData;

// sample Data
template<int _NumSamplesPerBlock,int _ControlledSampleDimensions,int _UncontrolledSampleDimensions,class _SampleOutput,class _Scalar> struct SimpleSampleData;

// loaded Scene Reader
template<class _PrimitiveType> struct LoadedSceneReader;

// samplers

template<class _SampleData,class _SceneReader> 
typename boost::shared_ptr<ISampler<_SampleData,_SceneReader>> CreateMCSampler();

// integrators

template<class _SampleData,class _RayData,class _SceneReader> 
typename boost::shared_ptr<IIntegrator<_SampleData,_RayData,_SceneReader>> CreateWhittedIntegrator();

// intersectors

template<class _RayData,class _SceneReader> 
typename boost::shared_ptr<IIntersector<_RayData,_SceneReader>> CreateSimpleIntersector();

template<class _RayData,class _SceneReader> 
typename boost::shared_ptr<IIntersector<_RayData,_SceneReader>> CreateBVHIntersector();

// engines

template<
	class _SampleData,
	class _RayData, 
	class _SceneReader>
	ObjectPointer<IEngine<_SceneReader>> CreateBasicRaytraceEngine(
		const boost::shared_ptr<IIntersector<_RayData,_SceneReader>>& intersector,
		const boost::shared_ptr<ISampler<_SampleData,_SceneReader>>& sampler,
		const boost::shared_ptr<IIntegrator<_SampleData,_RayData,_SceneReader>>& integrator,
		const _SceneReader& scene);

template<class _RayData,class _SampleData,class _SceneReader> struct EngineOptions
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef boost::shared_ptr<_SceneReader> SceneReader;

	typedef IIntersector<RayData,SceneReader> IIntersector;
	typedef ISampler<SampleData,SceneReader> ISampler;
	typedef IIntegrator<SampleData,RayData,SceneReader> IIntegrator;
	typedef IEngine<SceneReader> IEngine;

	typedef boost::shared_ptr<IIntersector> Intersector;
	typedef boost::shared_ptr<ISampler> Sampler;
	typedef boost::shared_ptr<IIntegrator> Integrator;
	typedef ObjectPointer<IEngine> Engine;

	typedef std::function<Intersector()> IntersectorConstructor;
	typedef std::function<Sampler()> SamplerConstructor;
	typedef std::function<Integrator()> IntegratorConstructor;
	typedef std::function<Engine(const Intersector& intersector,const Sampler& sampler,const Integrator& integrator,const SceneReader& sceneReader)> EngineConstructor;

	static const std::map<String,IntersectorConstructor>& getIntersectors()
	{
		static const std::map<String,IntersectorConstructor> intersectors = assign::map_list_of
			( String("Simple Intersector"), IntersectorConstructor( &CreateSimpleIntersector<RayData,SceneReader> ) )
			( String("BVH Intersector"), IntersectorConstructor( &CreateBVHIntersector<RayData,SceneReader> ) );
		return intersectors;
	}

	static const std::map<String,SamplerConstructor>& getSamplers()
	{
		static const std::map<String,SamplerConstructor> intersectors = assign::map_list_of
			( String("Monte Carlo Sampler"), SamplerConstructor( &CreateMCSampler<SampleData,SceneReader> ) );
		return intersectors;
	}


	static const std::map<String,IntegratorConstructor>& getIntegrators()
	{
		static const std::map<String,IntegratorConstructor> intersectors = assign::map_list_of
			( String("Whitted Integrator"), IntegratorConstructor( &CreateWhittedIntegrator<SampleData,RayData,SceneReader> ) );
		return intersectors;
	}

	static const std::map<String,EngineConstructor>& getEngines()
	{
		static const std::map<String,EngineConstructor> intersectors = assign::map_list_of
			( String("Basic Engine"), EngineConstructor( &CreateBasicRaytraceEngine<SampleData,RayData,SceneReader> ) );
		return intersectors;
	}
	
	static const std::vector<String>& getSamplerNames()
	{
		static const std::vector<String> names(mapToString(getSamplers()));
		return names;
	}
	static const std::vector<String>& getIntegratorNames()
	{
		static const std::vector<String> names(mapToString(getIntegrators()));
		return names;
	}
	static const std::vector<String>& getIntersectorNames()
	{
		static const std::vector<String> names(mapToString(getIntersectors()));
		return names;
	}
	static const std::vector<String>& getEngineNames()
	{
		static const std::vector<String> names(mapToString(getEngines()));
		return names;
	}
private:

	template<class _C> static std::vector<String> mapToString(const std::map<String,_C>& map)
	{
		std::vector<String> result;
		for(auto it = map.begin(); it != map.end(); it++)
			result.push_back(it->first);
		return result;
	}
};

typedef Ray<mpl::vector<RayLengthModeStored>> SimpleRay;
typedef Triangle<> SimpleTriangle;

typedef EngineOptions<
	SimpleRayData<SimpleRay,SimpleTriangle,64>,
	SimpleSampleData<64,2,64,Eigen::Matrix<f32,3,1>,f32>,
	LoadedSceneReader<SimpleTriangle>
> DefaultEngine;

}

#endif