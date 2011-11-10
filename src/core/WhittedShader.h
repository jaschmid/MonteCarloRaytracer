/********************************************************/
// FILE: WhittedShader.h
// DESCRIPTION: Raytracer Whitted Shader
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

#ifndef RAYTRACE_WHITTED_SHADER_GUARD
#define RAYTRACE_WHITTED_SHADER_GUARD

#include <RaytraceCommon.h>
#include "EngineBase.h"
#include "ConcurrentStorage.h"


namespace Raytrace {
	
#ifdef _DEBUG
static const int MAX_SAMPLES_PER_STACK	= 256;
static const int SAMPLES_PER_BLOCK		= 4;
#else
static const int MAX_SAMPLES_PER_STACK	= 32*1024;
static const int SAMPLES_PER_BLOCK		= 64;
#endif

template<class _SampleData,class _RayData> struct WhittedShaderEngine;
struct WhittedSampleData;


struct WhittedShader
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef WhittedShaderEngine<_SampleData,_RayData> type;
	};
	typedef WhittedSampleData ExternalData;
	typedef EngineMultiThreaded ThreadingMode;
	typedef EngineSampler EngineType;
};

template<class _SampleData,class _RayData> struct WhittedShaderEngine
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;

	inline WhittedShaderEngine(const WhittedShader& shader,_SampleData& sampleData,RayData& rayData,const SceneReader& r) :
		_sampleData(sampleData),_rayData(rayData)
	{

		Triangle dummy;

		_triangleMaterials.resize(r.getNumTriangles());
		_materials.resize(r.getNumMaterials());
		

		for(size_t i = 0; i < _materials.size(); ++i)
		{
			Material mat = r.getMaterial(i);

			_materials[i]._color = mat->GetColor().head<3>();
		}

		for(size_t i = 0; i < _triangleMaterials.size(); ++i)
		{
			Triangle dummy;
			r.getTriangle(i,dummy,_triangleMaterials[i]);
			assert( _triangleMaterials[i] < (int)_materials.size() );
		}
	}
	inline void prepare(int numThreads) {}
	inline void threadEnterSubsample(int threadId) 
	{
		int count = 1;
		int sampleId = _sampleData.getNextSamples(threadId,count);
		while(count)
		{
			int triangleId;
			int parentRay;
			Vector2 barycentric;
			Ray ray;

			_sampleData.getSampleSubsampleInfo(sampleId,parentRay,triangleId,barycentric);
			_rayData.getRayInfo(parentRay,ray);

			_sampleData.setSampleRays(sampleId,0,0);

			count = 1;
			sampleId = _sampleData.getNextSamples(threadId,count);
		}
		_sampleData.threadCompleteSubsampling(threadId,sampleId);
	}
	inline void threadEnterShade(int threadId) 
	{
		int count = 1;
		int sampleId = _sampleData.getNextSamples(threadId,count);
		while(count)
		{
			int triangleId;
			int parentRay;
			Vector2 barycentric;
			int firstRay;
			int numRays;
			Ray ray;

			_sampleData.getSampleShadingInfo(sampleId,parentRay,triangleId,barycentric,firstRay,numRays);
			_rayData.getRayInfo(parentRay,ray);

			if(triangleId >= 0)
			{
				/*
				float r = (float)((triangleId * 251) % 0x100) / 255.0f;
				float g = (float)((triangleId * 79) % 0x100) / 255.0f;
				float b = (float)((triangleId * 67) % 0x100) / 255.0f;*/
				int materialId = _triangleMaterials[triangleId];
				if(materialId < (int)_materials.size() )
				{
					const Vector3& color = _materials[materialId]._color;
					_rayData.shadeRay(parentRay,Vector4(color.x(),color.y(),color.z(),1.0f));
				}
				else
				{
					_rayData.shadeRay(parentRay,Vector4(0.0f,0.0f,0.0f,1.0f));
				}
			}
			else
				_rayData.shadeRay(parentRay,Vector4(1.0f,1.0f,1.0f,1.0f));

			count = 1;
			sampleId = _sampleData.getNextSamples(threadId,count);

		}
		_sampleData.threadCompleteShading(threadId,sampleId);
	}

	SampleData& _sampleData;
	RayData& _rayData;

	std::vector<int>	_triangleMaterials;

	struct MaterialSettings
	{
		Vector3	_color;
	};

	std::vector<MaterialSettings> _materials;
};

struct WhittedSampleData
{
public:

	inline WhittedSampleData() : _maxSamplesPerStack(MAX_SAMPLES_PER_STACK) {}

	inline void prepare(int numThreads) 
	{
		_sampleData.prepare(numThreads);
		_threadData.resize(numThreads);
		_currentNextSubsample = 0;
		_currentNextShade = 0;
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			it->_nextSubsample = -1;
			it->_nextShade = -1;
		}
	}

	inline void pushSample(int threadId,int parentRay,int triangleId,const Vector2& barycentric)
	{
		int id =  _sampleData.pushElements(threadId,1);

		Sample& sample = getSample(id);
		sample._barycentricLocation = barycentric;
		sample._parentRay = parentRay;
		sample._triangleId = triangleId;
	}

	inline void preparePushingSamples()
	{
		_sampleData.setMaxElement(_maxSamplesPerStack + _sampleData.endElementId());
	}

	inline void completePushingSamples()
	{
		_sampleData.finishWritingBlock();
	}

	inline void setSampleRays(int id,int firstRayId,int count)
	{
		Sample& sample = getSample(id);
		sample._firstChildRay = firstRayId;
		sample._numChildRays = count;
	}

	inline int getNextSamples(int thread,int& count)
	{
		return _sampleIterator.getNextElements(thread,count);
	}

	inline void getSampleSubsampleInfo(int id,int& parentRay,int& triangleId,Vector2& barycentric) const
	{
		const Sample& sample = getSample(id);
		parentRay = sample._parentRay;
		triangleId  = sample._triangleId;
		barycentric = sample._barycentricLocation;
	}

	inline void getSampleShadingInfo(int id,int& parentRay,int& triangleId,Vector2& barycentric,int& firstRayId,int& numRays) const
	{
		const Sample& sample = getSample(id);
		parentRay = sample._parentRay;
		triangleId  = sample._triangleId;
		barycentric = sample._barycentricLocation;
		firstRayId = sample._firstChildRay;
		numRays = sample._numChildRays;
	}

	inline void pushStack()
	{
		StackLevel newEntry;
		newEntry.nextSubsample = _currentNextSubsample;
		newEntry.nextShade = _currentNextShade;
		newEntry.endSample = _sampleData.endElementId();
		_sampleStack.push_back(newEntry);
		_currentNextSubsample = _sampleData.endElementId();
		_currentNextShade = _sampleData.endElementId();
	}

	inline void popStack()
	{
		_currentNextSubsample = _sampleStack.back().nextSubsample;
		_currentNextShade = _sampleStack.back().nextShade;
		_sampleData.popElements(_sampleStack.back().endSample);
		_sampleStack.pop_back();
	}

	inline bool emptyStack() const
	{
		if(_sampleStack.empty())
			return true;
		else
			return true;
	}
	
	inline void prepareSubsampling()
	{
		_sampleIterator = _sampleData.begin(_currentNextSubsample,_currentNextShade);
	}

	inline bool hasSamplesToSubsample() const
	{
		if(_currentNextSubsample >= _sampleData.endElementId())
			return false;
		else
			return true;
	}

	inline void threadCompleteSubsampling(int threadId,int nextSubsample)
	{
		_threadData[threadId]._nextSubsample = nextSubsample;
	}

	inline void completeSubsampling()
	{
		int nextSubsample = -1;

		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			if(it->_nextSubsample != -1 && (it->_nextSubsample < nextSubsample || nextSubsample == -1) )
				nextSubsample = it->_nextSubsample;
		}

		_currentNextSubsample = nextSubsample;
	}
	
	inline void prepareShading() 
	{
		_sampleIterator = _sampleData.begin(_currentNextShade);
	}

	inline void threadCompleteShading(int threadId,int nextShade)
	{
		_threadData[threadId]._nextShade = nextShade;
	}

	inline void completeShading()
	{
		int nextShade = -1;

		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			if(it->_nextShade != -1 && (it->_nextShade < nextShade || nextShade == -1) )
				nextShade = it->_nextShade;
		}

		_currentNextShade = nextShade;
	}

	inline bool hasSamplesToShade() const
	{
		if(_currentNextShade >= _sampleData.endElementId())
			return false;
		else
			return true;
	}

	inline void popShaded()
	{
		_sampleData.popElements(_currentNextShade);
	}
	
	struct Sample
	{
		inline Sample() :
			_numChildRays(-1)
		{
		}
		
		// initial data, should always be set
		int			_parentRay;
		int			_triangleId; //-1 for not resolved
		Vector2		_barycentricLocation;

		// only set once shaded
		int			_firstChildRay;
		int			_numChildRays;//-1 for unprocessed, 0 for no child samples
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

private:
	inline WhittedSampleData(const WhittedSampleData& other) : _maxSamplesPerStack(10000) {}

	// sample organisation
	// 1. parent sample creates child samples and sets initial values
	// 2. intersector calculates intersections sets intersector values
	// 3. if there's subsamples create sub samples (1), push stack
	// 4. when sub samples complete, finalize sample



	struct StackLevel
	{
		int nextSubsample; // -1 for none
		int nextShade; // -1 for none
		int endSample; // -1 for none (1 past the last sample)
		

		// first -> firstSubsampled = completed samples
		// firstSubsampled -> firstFresh = subsampled samples
		// firstSubsampled -> endSample = fresh samples
	};

	struct ThreadData
	{
		int _nextShade; // -1 for all, 0 for none
		int _nextSubsample; // -1 for all, 0 for none
	};

	Sample& getSample(int id)
	{
		return _sampleData.getElement(id);
	}
	const Sample& getSample(int id) const
	{
		return _sampleData.getElement(id);
	}

	typedef ConcurrentStorage<Sample,SAMPLES_PER_BLOCK> SampleDataType;

	SampleDataType::MultiAccessIterator			_sampleIterator;

	int									_currentNextSubsample;
	int									_currentNextShade;

	const int							_maxSamplesPerStack;

	SampleDataType						_sampleData;
	std::vector<StackLevel>				_sampleStack;
	std::vector<ThreadData>				_threadData;
};

}
#endif