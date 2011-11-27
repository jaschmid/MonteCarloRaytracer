/********************************************************/
// FILE: SampleData.h
// DESCRIPTION: Data container for Samples
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

#ifndef RAYTRACE_SAMPLE_DATA_GUARD
#define RAYTRACE_SAMPLE_DATA_GUARD

#include <RaytraceCommon.h>

#ifdef _DEBUG
static const int MAX_SAMPLES_PER_STACK	= 256;
static const int SAMPLES_PER_BLOCK		= 4;
#else
static const int MAX_SAMPLES_PER_STACK	= 1024*1024;
static const int SAMPLES_PER_BLOCK		= 16;
#endif

static const int SHADOWS_PER_BLOCK		= 8*64;

namespace Raytrace {
	
template<class _SampleIdType = u32,class _ShadowIdType = u32> struct SimpleSampleData
{
public:
	
	typedef _SampleIdType IntersectionIdType;
	typedef _SampleIdType SampleIdType;
	typedef _ShadowIdType ShadowIdType;
	
	static const IntersectionIdType INVALID_INTERSECTION_ID = -1;
	static const SampleIdType INVALID_SAMPLE_ID = -1;
	static const ShadowIdType INVALID_SHADOW_ID = -1;
	
	static const IntersectionIdType IntersectionBlockSize = SAMPLES_PER_BLOCK;
	static const SampleIdType SampleBlockSize = SAMPLES_PER_BLOCK;
	static const ShadowIdType ShadowBlockSize = SHADOWS_PER_BLOCK;

	inline SimpleSampleData() : _maxSamplesPerStack(MAX_SAMPLES_PER_STACK) {}

	inline void prepare(int numThreads) 
	{
		_sampleData.prepare(numThreads);
		_shadowData.prepare(numThreads);
		_intersectionData.prepare(numThreads);
		_threadData.resize(numThreads);
		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			it->_nextSubsample = -1;
			it->_nextShade = -1;
		}
	}
		
	inline SampleIdType allocateSamples(int threadId,SampleIdType count)
	{
		SampleIdType id =  _sampleData.pushElements(threadId,count);
		
		if(id != -1)
		{
			return id;
		}
		else
			return INVALID_SAMPLE_ID;
	}


	std::pair<SampleIdType,ShadowIdType> allocateSubsamples(int threadId,IntersectionIdType idParent,SampleIdType countSamples,ShadowIdType countShadow)
	{
		Intersection& intersection = getIntersection(idParent);
		std::pair<SampleIdType,ShadowIdType> result;
		
		if(countSamples == 0)
		{
			intersection._firstChildSample = 0;
			intersection._numChildSamples = 0;
			result.first = INVALID_SAMPLE_ID;
		}
		else
		{

			intersection._firstChildSample =  _sampleData.pushElements(threadId,countSamples);

			if(intersection._firstChildSample != INVALID_SAMPLE_ID)
			{
				intersection._numChildSamples = countSamples;
				result.first = intersection._firstChildSample;
			}
			else
			{
				intersection._numChildSamples = 0;
				result.first = INVALID_SAMPLE_ID;
			}
		}

		if(countShadow == 0)
		{
			intersection._firstChildShadow = 0;
			intersection._numChildShadows = 0;
			result.second = INVALID_SHADOW_ID;
		}
		else
		{
			intersection._firstChildShadow =  _shadowData.pushElements(threadId,countShadow);

			if(intersection._firstChildShadow != INVALID_SHADOW_ID)
			{
				intersection._numChildShadows = countShadow;
				result.second = intersection._firstChildShadow;
			}
			else
			{
				intersection._numChildShadows = 0;
				result.second = INVALID_SHADOW_ID;
			}
		}

		return result;
	}
	
	inline void getSubsampleInfo(IntersectionIdType idParent,SampleIdType& parentSample,SampleIdType& firstSample,SampleIdType& countSample,ShadowIdType& firstShadow,ShadowIdType& countShadow) const
	{
		const Intersection& intersection = getIntersection(idParent);

		parentSample = intersection._parentSample;
		firstSample = intersection._firstChildSample;
		countSample = intersection._numChildSamples;
		firstShadow = intersection._firstChildShadow;
		countShadow = intersection._numChildShadows;
	}

	inline IntersectionIdType getNextIntersections(int threadId,IntersectionIdType& count)
	{
		return _intersectionIterator.getNextElements(threadId,count);
	}
	
	inline IntersectionIdType setIntersectionResult(int threadId,SampleIdType parentSample,const int& triangleId,const Vector2& barycentric,const Vector3& parentDirection)
	{
		IntersectionIdType intersectionId = _intersectionData.pushElements(threadId,1);
		Intersection& intersection = getIntersection(intersectionId);
		intersection._parentSample = parentSample;
		intersection._triangleId = triangleId;
		intersection._barycentricLocation = barycentric;
		intersection._parentVector = parentDirection;
		return intersectionId;
	}

	inline void getIntersectionResult(IntersectionIdType id,int& triangleId,Vector2& barycentric,Vector3& parentDirection) const
	{
		const Intersection& intersection = getIntersection(id);
		triangleId  = intersection._triangleId;
		barycentric = intersection._barycentricLocation;
		parentDirection = intersection._parentVector;
	}
	
	inline void setIntersectionResult(ShadowIdType id,const bool& result)
	{
		getShadow(id) = result; 
	}

	inline void getIntersectionResult(ShadowIdType id,bool& result) const
	{
		result = getShadow(id); 
	}

	inline void getSampleInfo(SampleIdType id,Vector3& intensity,Real& contribution) const
	{
		const Sample& sample = getSample(id);
		intensity = sample._color.head<3>();
		contribution = sample._color.w();
	}

	inline void getSampleInfo(SampleIdType id,Vector4& total) const
	{
		const Sample& sample = getSample(id);
		total = sample._intensity;
	}

	inline void setSampleIntensity(SampleIdType id,const Vector3& intensity)
	{
		Sample& sample = getSample(id);
		sample._intensity.x() = intensity.x();
		sample._intensity.y() = intensity.y();
		sample._intensity.z() = intensity.z();
	}

	inline void setSampleContribution(SampleIdType id, const Real& contribution)
	{
		Sample& sample = getSample(id);
		sample._intensity.w() = contribution;
	}

	inline void pushStack()
	{
		StackLevel newEntry;
		if(_sampleStack.empty())
		{
			newEntry.nextSubsample = 0;
			newEntry.nextShade = 0;
		}
		else
		{
			newEntry.nextSubsample = _intersectionData.endElementId();
			newEntry.nextShade = _intersectionData.endElementId();
		}
		newEntry.endIntersection = _intersectionData.endElementId();
		newEntry.endSample = _sampleData.endElementId();
		_sampleStack.push_back(newEntry);
	}

	inline void clearOut()
	{
		_sampleData.popElements(_sampleStack.back().endSample);
		_intersectionData.popElements(_sampleStack.back().endIntersection);
	}

	inline void popStack()
	{
		if(!_sampleStack.empty())
		{
			clearOut();
			_sampleStack.pop_back();
		}
		else
			return;

	}

	inline void prepareIntersecting()
	{
	}

	inline void completeIntersecting()
	{
		_intersectionData.finishWritingBlock();
	}

	inline bool emptyStack() const
	{
		if(_sampleStack.empty())
			return true;
		else
			return false;
	}
	
	inline bool hasIntersectionsToSubsample() const
	{
		if(_sampleStack.back().nextSubsample >= _sampleStack.back().endSample)
			return false;
		else
			return true;
	}

	inline void prepareSubsampling()
	{
		_intersectionIterator = _intersectionData.begin(_sampleStack.back().nextSubsample,_sampleStack.back().endSample);
		_sampleData.setMaxElement(_maxSamplesPerStack + _sampleData.endElementId());
	}

	inline void completeSubsampling()
	{
		IntersectionIdType nextSubsample = INVALID_INTERSECTION_ID;

		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			if(it->_nextSubsample != INVALID_INTERSECTION_ID && (it->_nextSubsample < nextSubsample || nextSubsample == INVALID_INTERSECTION_ID) )
				nextSubsample = it->_nextSubsample;
		}

		_sampleStack.back().nextSubsample = nextSubsample;
		_sampleData.finishWritingBlock();
	}

	inline void prepareGenerating()
	{
		while(!emptyStack());
			popStack();

		_sampleData.popElements(0);
		_intersectionData.popElements(0);
	}

	inline void completeGenerating()
	{
	}
	
	inline void prepareShading() 
	{
		_intersectionIterator = _intersectionData.begin(_sampleStack.back().nextShade,_sampleStack.back().nextSubsample);
	}
	
	inline void threadCompleteSubsampling(int threadId,const IntersectionIdType& nextSubsample)
	{
		_threadData[threadId]._nextSubsample = nextSubsample;
	}

	inline void threadCompleteShading(int threadId,const IntersectionIdType& nextShade)
	{
		_threadData[threadId]._nextShade = nextShade;
	}

	inline void completeShading()
	{
		IntersectionIdType nextShade = INVALID_INTERSECTION_ID;

		for(auto it = _threadData.begin(); it != _threadData.end(); ++it)
		{
			if(it->_nextShade != INVALID_INTERSECTION_ID && (it->_nextShade < nextShade || nextShade == INVALID_INTERSECTION_ID) )
				nextShade = it->_nextShade;
		}

		_sampleStack.back().nextShade = nextShade;
	}

	inline bool hasIntersectionsToShade() const
	{
		if(_sampleStack.back().nextShade >= _sampleStack.back().nextSubsample)
			return false;
		else
			return true;
	}

	struct Sample
	{
		Sample() :
			_intensity(-1.0f,-1.0f,-1.0f,-1.0f)
			{
			}
		// shading info, -1 for not set
		Vector4		_intensity;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	struct Intersection
	{
		inline Intersection() :
			_numChildSamples(-1),
			_triangleId(-1)
		{
		}

		// initial info
		SampleIdType		_parentSample;
		Vector3				_parentVector;
		Vector2				_barycentricLocation;
		int					_triangleId; //-1 for not resolved or none

		// set when subsampling
		SampleIdType		_firstChildSample;
		SampleIdType		_numChildSamples;//-1 for unprocessed, 0 for no child samples

		ShadowIdType		_firstChildShadow;
		ShadowIdType		_numChildShadows;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

private:
	inline SimpleSampleData(const SimpleSampleData& other) : _maxSamplesPerStack(10000) {}

	// sample organisation
	// 1. parent sample creates child samples and sets initial values
	// 2. intersector calculates intersections sets intersector values
	// 3. if there's subsamples create sub samples (1), push stack
	// 4. when sub samples complete, finalize sample



	struct StackLevel
	{
		IntersectionIdType nextSubsample; // -1 for none
		IntersectionIdType nextShade; // -1 for none
		IntersectionIdType endIntersection;

		SampleIdType endSample; // -1 for none (1 past the last sample)
		

		// first -> firstSubsampled = completed samples
		// firstSubsampled -> firstFresh = subsampled intersections
		// firstSubsampled -> endIntersection = fresh intersections
	};

	struct ThreadData
	{
		IntersectionIdType _nextShade; // -1 for all, 0 for none
		IntersectionIdType _nextSubsample; // -1 for all, 0 for none
	};
	
	typedef ConcurrentStorage<Intersection,SAMPLES_PER_BLOCK,IntersectionIdType> IntersectionDataType;
	typedef ConcurrentStorage<Sample,SAMPLES_PER_BLOCK,SampleIdType> SampleDataType;
	typedef ConcurrentStorage<bool,SHADOWS_PER_BLOCK,ShadowIdType> ShadowDataType;

	typename SampleDataType::reference getSample(const SampleIdType& id)
	{
		return _sampleData.getElement(id);
	}
	typename SampleDataType::const_reference getSample(const SampleIdType& id) const
	{
		return _sampleData.getElement(id);
	}
	
	typename IntersectionDataType::reference getIntersection(const IntersectionIdType& id)
	{
		return _intersectionData.getElement(id);
	}
	typename IntersectionDataType::const_reference getIntersection(const IntersectionIdType& id) const
	{
		return _intersectionData.getElement(id);
	}

	typename ShadowDataType::reference getShadow(const ShadowIdType& id)
	{
		return _shadowData.getElement(id);
	}
	typename ShadowDataType::const_reference getShadow(const ShadowIdType& id) const
	{
		return _shadowData.getElement(id);
	}

	typename IntersectionDataType::MultiAccessIterator			_intersectionIterator;

	const IntersectionIdType			_maxSamplesPerStack;

	SampleDataType						_sampleData;
	ShadowDataType						_shadowData;
	IntersectionDataType				_intersectionData;

	std::vector<StackLevel>				_sampleStack;
	std::vector<ThreadData>				_threadData;
};

}

#endif