/********************************************************/
// FILE: BVHIntersector.h
// DESCRIPTION: Ray Intersector using MBVH's
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

#ifndef RAYTRACE_BVH_INTERSECTOR_GUARD
#define RAYTRACE_BVH_INTERSECTOR_GUARD

#include <RaytraceCommon.h>
#include <queue>
#include "Ray.h"
#include "EngineBase.h"
#include "ConcurrentStorage.h"
#include "MathHelper.h"
#include "RayData.h"
#include "BVH.h"
#include "RayAABBIntersection.h"
#include "RayTriangleIntersection.h"

namespace Raytrace {

template<class _SampleData,class _RayData,int _SimdWidth,int _NodeArraySize,int _LeafArraySize> struct BVHIntersectorEngine;

struct BVHIntersector
{
	template<class _SampleData,class _RayData> struct Engine
	{
		typedef BVHIntersectorEngine<_SampleData,_RayData,4,1,1> type;
	};
	typedef DualRayData<> ExternalData;
	typedef EngineMultiThreaded ThreadingMode;
	typedef EngineIntersector EngineType;
};

template<class _SampleData,class _RayData,int _SimdWidth,int _NodeArraySize,int _LeafArraySize> struct BVHIntersectorEngine
{
	
	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef typename _RayData::RayIdType RayIdType;

	static const int NodeArraySize = _NodeArraySize;
	static const int LeafArraySize = _LeafArraySize;
	static const int SimdWidth = _SimdWidth;
	static const int NodeWidth = SimdWidth*NodeArraySize;
	static const int LeafWidth = SimdWidth*LeafArraySize;

	typedef SimdType<int,SimdWidth>	Scalari_T;
	typedef SimdType<float,SimdWidth>	Scalar_T;
	typedef typename Vector2v<SimdWidth>::type Vector2_T;
	typedef typename Vector3v<SimdWidth>::type Vector3_T;

	typedef TriAccel<SimdWidth>		PrimitiveType;
	typedef AABBAccel<SimdWidth>	VolumeType;

	typedef typename RayData::AnyHitRay AnyHitRay;
	typedef typename RayData::FirstHitRay FirstHitRay;
	
	template<class _RayType> struct RayTypeInfo
	{

	};

	template<> struct RayTypeInfo<AnyHitRay>
	{
		typedef RayAccel<SimdType<f32,SimdWidth>,3,SignModePrecompute,InvDirModePrecompute> type;
		typedef typename Intersector<boost::mpl::vector< 
				Raytrace::RayType<type>,
				Raytrace::PrimitiveType<VolumeType>, 
				Raytrace::RayCount<1>, 
				Raytrace::PrimitiveCount<NodeArraySize>,
				Raytrace::RayModeConstant
			>>::type intersector_volume;
		typedef typename Intersector<boost::mpl::vector< 
				Raytrace::RayType<type>,
				Raytrace::PrimitiveType<PrimitiveType>, 
				Raytrace::RayCount<1>, 
				Raytrace::PrimitiveCount<LeafArraySize>,
				Raytrace::RayModeConstant
			>>::type intersector_primitive;
	};
	
	template<> struct RayTypeInfo<FirstHitRay>
	{
		typedef RayAccel<SimdType<f32,SimdWidth>,3,SignModePrecompute,InvDirModePrecompute> type;
		typedef typename Intersector<boost::mpl::vector< 
				Raytrace::RayType<type>,
				Raytrace::PrimitiveType<VolumeType>, 
				Raytrace::RayCount<1>, 
				Raytrace::PrimitiveCount<NodeArraySize>,
				Raytrace::RayModeUpdateMinimum
			>>::type intersector_volume;
		typedef typename Intersector<boost::mpl::vector< 
				Raytrace::RayType<type>,
				Raytrace::PrimitiveType<PrimitiveType>, 
				Raytrace::RayCount<1>, 
				Raytrace::PrimitiveCount<LeafArraySize>,
				Raytrace::RayModeUpdateMinimum
			>>::type intersector_primitive;
	};

	typedef ContainerMultiplier<typename PrimitiveType::Minimum,PrimitiveType,SimdWidth,LeafArraySize> PrimitiveContainer;
	typedef ContainerMultiplier<typename VolumeType::Minimum,VolumeType,SimdWidth,NodeArraySize> VolumeContainer;

	typedef BVH<typename PrimitiveType::Minimum,typename VolumeType::Minimum,LeafWidth,NodeWidth,PrimitiveContainer,VolumeContainer> BVHType;
	
	template<int _MaxStackSize> struct ActiveStack
	{
		typedef typename BVHType::nodeIterator Node;
		static const size_t MaxStackSize = _MaxStackSize;

		ActiveStack()
		{
		}

		inline Node getLowest()
		{
			Node result = _stack.back().node();
			_stack.pop_back();
	//		_stack.back().node().prefetch();
			return result;
			/*
			element* curr = & _stack.front();
			
			for(u32 i = 1; i < _stack.size();++i)
				if(_stack[i] < *curr)
					curr = &_stack[i];
					
			Node result = curr->node();
			*curr = _stack.back();
			_stack.pop_back();
			return result;*/
		}

		inline void compact(f32 v)
		{
			u32 pos = 0;
			for(; pos < _stack.size();++pos)
				if(_stack[pos].t() < v)
					break;

			if(pos != 0)
			{
				for(u32 i = 0; i < _stack.size() - pos; ++i)
					_stack[i] = _stack[i+pos];
				_stack.resize(_stack.size()-pos);
			}

			/*
			for(u32 i = 0; i < _stack.size();)
			{
				if(_stack[i].t() >= v)
				{
					_stack[i] = _stack.back();
					_stack.pop_back();
				}
				else
					++i;
			}*/
		}

		inline void push(Node node,f32 t)
		{
			//bubble sort push
			element curr(node,t);
			u32 pos = (u32)_stack.size();
			_stack.push_back(curr);

			for(; pos > 0;--pos)
			{
				if(_stack[pos-1] < _stack[pos])
				{
					element t = _stack[pos];
					_stack[pos] = _stack[pos-1];
					_stack[pos-1] = t;
				}
					//std::swap<element>(_stack[pos],_stack[pos-1]);
				else
					break;
			}
		}

		inline void conditional_push(bool condition,Node node,f32 t)
		{
			if(condition)
				_stack.push_back(element(node,t));

			//_stack.conditional_push_back(condition,element(node,t));
		}

		bool empty() const
		{
			return _stack.empty();
		}

		
		struct element
		{
			inline element(Node n,f32 t)
			{
				_t = t;
				_n = n;
			}
			inline element() {}

			inline const Node& node()
			{
				return _n;
			}

			inline f32& t()
			{
				return _t;
			}
			
			inline const f32& t() const
			{
				return _t;
			}

			inline bool operator < (const element& other) const
			{
				return t() < other.t();
			}

			struct
			{
				f32 _t;
				Node _n;
			};
		};


		static_vector<element,MaxStackSize>	_stack;
	};

	inline BVHIntersectorEngine(const BVHIntersector& shader,_SampleData& sampleData,RayData& rayData,const SceneReader& r) : _rayData(rayData),_sampleData(sampleData)
	{
		BVHType::Constructor constructor(64);

		int num = r.getNumTriangles();
		for(int i = 0; i< num; ++i)
		{
			Triangle tri;
			int material;
			r.getTriangle(i,tri,material);
			
			AABB volume(tri);

			Vector3 centroid = (tri.point(0) + tri.point(1) + tri.point(2))/3.0f;

			tri._user = i + 1;
			constructor.addElement( tri, centroid, volume );
		}

		_sceneData.reset( new BVHType(constructor) );
	}

	inline void prepare(int numThreads) 
	{
	}

	inline void threadEnter(int threadId) 
	{
		unsigned int prevCSR = _mm_getcsr();
		_mm_setcsr(0xffc0);

		doIntersections<typename RayData::AnyHitRay>(threadId);
		doIntersections<typename RayData::FirstHitRay>(threadId);

		_mm_setcsr(prevCSR);
	}

	template<class _RayType> void doIntersections(int threadId)
	{
		RayIdType count = 1;
		RayIdType rayId = _rayData.getNextRays<_RayType>(threadId,count);
		while(count)
		{
			Ray rayBase;
			typename RayData::OutputIdType<_RayType>::type outputSample;
			_rayData.getRayInfo<_RayType>(rayId,rayBase,outputSample);

			processRay<_RayType>(threadId,rayBase,outputSample);

			count = 1;
			rayId = _rayData.getNextRays<_RayType>(threadId,count);
		}
		_rayData.threadCompleteIntersecting<_RayType>(threadId,rayId);
	}
	template<class _RayType> void processRay(int threadId,const Ray& ray,const typename RayData::OutputIdType<_RayType>::type& outputSample)
	{
		static_assert(false,"Unsupported Ray Type")
	}


	template<> void processRay<typename RayData::AnyHitRay>(int threadId,const Ray& ray,const typename RayData::OutputIdType<typename RayData::AnyHitRay>::type& outputSample)
	{
		bool found;

		found = processNode(ray);
		/*
		if(ray.direction().x() < 0.0)
			found = processNodeSplitX<-1>( ray);
		else if(ray.direction().x() == 0.0)
			found = processNodeSplitX<0>(ray);
		else// if(ray.direction().x() > 0.0)
			found = processNodeSplitX<1>(ray);*/

		if(found)
			_sampleData.setIntersectionResult(outputSample,true);
		else
			_sampleData.setIntersectionResult(outputSample,false);
	}
	
	template<> void processRay<typename RayData::FirstHitRay>(int threadId,const Ray& rayBase,const typename RayData::OutputIdType<typename RayData::FirstHitRay>::type& outputSample)
	{
		typedef RayData::FirstHitRay RayType;
		std::array<Ray,SimdWidth> rayArray;

		for(int i = 0; i < SimdWidth; ++i)
			rayArray[i] = rayBase;

		typename RayTypeInfo<AnyHitRay>::type ray(rayArray);
		Scalar_T ts(rayBase.length() > .0f ? rayBase.length() : std::numeric_limits<Real>::infinity());
		Vector2_T barys;
		Scalari_T triIds = 0;
		int raysigns = 0;

		if(rayBase.direction().x() <= 0)
			raysigns |= 1;
		if(rayBase.direction().y() <= 0)
			raysigns |= 2;
		if(rayBase.direction().z() <= 0)
			raysigns |= 4;

		processNode(ray, raysigns, _sceneData->root() , triIds, ts, barys);

		int triId = 0;
		Vector2 bary;
		Real t;

		for(int i = 0; i < SimdWidth; ++i)
			if(triIds[i] != 0)
			{
				triId = triIds[i];
				bary.x() = barys.x()[i];
				bary.y() = barys.y()[i];
				t = ts[i];
			}

		if(triId != 0)
			_sampleData.setIntersectionResult(threadId,outputSample,triId - 1,bary,rayBase.direction());
		else
			_sampleData.setIntersectionResult(threadId,outputSample,-1,Vector2(0.0f,0.0f),rayBase.direction());
	}

	/*
	template<int _XSign> inline bool processNodeSplitX(const Ray& ray)
	{
		if(ray.direction().y() < 0.0)
			return processNodeSplitXY<_XSign,-1>(ray);
		else if(ray.direction().y() == 0.0)
			return processNodeSplitXY<_XSign,0>(ray);
		else// if(ray.direction().y() > 0.0)
			return processNodeSplitXY<_XSign,1>(ray);
	}
	
	template<int _XSign,int _YSign> inline bool processNodeSplitXY(const Ray& ray)
	{
		if(ray.direction().z() < 0.0)
			return processNodeSplitXYZ<_XSign,_YSign,-1>(ray);
		else if(ray.direction().z() == 0.0)
			return processNodeSplitXYZ<_XSign,_YSign,0>(ray);
		else// if(ray.direction().z() > 0.0)
			return processNodeSplitXYZ<_XSign,_YSign,1>(ray);
	}
	

	template<int _XSign,int _YSign,int _ZSign> inline bool processNodeSplitXYZ(const Ray& rayBase)
	{
		static_vector<typename BVHType::nodeIterator,128 > stack;
		typedef RayData::AnyHitRay RayType;
		std::array<Ray,SimdWidth> rayArray;

		for(int i = 0; i < SimdWidth; ++i)
			rayArray[i] = rayBase;

		RayAccelSlope ray(rayArray);
		Scalar_T t(rayBase.length() > .0f ? rayBase.length() : std::numeric_limits<Real>::infinity());

		stack.push_back(_sceneData->root());

		while(!stack.empty())
		{
			BVHType::nodeIterator node = stack.back();
			stack.pop_back();

			if(node.isLeaf())
			{
				if(IntersectArray(ray, node.leaf(), t))
					return true;
			}
			else
			{
				int resultMask = IntersectArraySplit<_XSign,_YSign,_ZSign>(ray, node.volumes(), t);

				if(resultMask)
				{
					node.prefetchChildren();
					for(size_t i = 0; i < NodeWidth; ++i)
						if((resultMask >> i) & 1)
							stack.push_back(node.node(i));
				}

			}
		}
		return false;
	}*/

	__declspec(noinline) bool processNode(const Ray& rayBase)
	{
		static_vector<typename BVHType::nodeIterator,128 > stack;
		typedef RayData::AnyHitRay RayType;
		std::array<Ray,SimdWidth> rayArray;
		RayTypeInfo<RayType>::intersector_primitive	intersectorPrimitive;
		RayTypeInfo<RayType>::intersector_volume		intersectorVolume;

		for(int i = 0; i < SimdWidth; ++i)
			rayArray[i] = rayBase;

		typename RayTypeInfo<AnyHitRay>::type ray(rayArray);
		Scalar_T t(rayBase.length() > .0f ? rayBase.length() : std::numeric_limits<Real>::infinity());

		stack.push_back(_sceneData->root());

		while(!stack.empty())
		{
			BVHType::nodeIterator node = stack.back();
			stack.pop_back();

			if(node.isLeaf())
			{
				if(intersectorPrimitive(ray, node.leaf(), t))
					return true;
			}
			else
			{
				std::array< RayTypeInfo<AnyHitRay>::intersector_volume::BooleanMask ,NodeArraySize> resultMask;
				
				intersectorVolume(ray, node.volumes(), t,resultMask);

				for(size_t j = 0; j < NodeArraySize; ++j)
					if(resultMask[j])
					{
						for(size_t i = 0; i < SimdWidth; ++i)
						{
							//node.prefetchChild(j*SimdWidth+i);
							stack.conditional_push_back( (resultMask[j] >> i) & 1, node.node(j*SimdWidth+i));
						}
					}

			}
		}
		return false;
	}

	__declspec(noinline) void processNode(const typename RayTypeInfo<FirstHitRay>::type& ray,int raysigns,const typename BVHType::nodeIterator& it,Scalari_T& triId,Scalar_T& t,Vector2_T& bary)
	{
		ActiveStack<128> stack;
		stack.push(it,0.0f);
		while(!stack.empty())
		{
			BVHType::nodeIterator node = stack.getLowest();
			if(node.isLeaf())
			{
				bool found = false;

				if(RayTypeInfo<FirstHitRay>::intersector_primitive()(ray, node.leaf(), t, bary,triId))
					found = true;

				if(found)
					stack.compact(t[0]);
			}
			else
			{

				std::array<Scalar_T,NodeArraySize> tTemp;
				std::array< RayTypeInfo<AnyHitRay>::intersector_volume::BooleanMask ,NodeArraySize> resultMask;
				for(int i = 0; i < NodeArraySize; ++i)
					tTemp[i] = t;

				
				RayTypeInfo<FirstHitRay>::intersector_volume()(ray, node.volumes(), tTemp, resultMask);
				
				for(size_t j = 0; j < NodeArraySize; ++j)
					if(resultMask[j])
					{
						for(size_t i = 0; i < SimdWidth; ++i)
						{
							//node.prefetchChild(j*SimdWidth+i);
							stack.conditional_push( (resultMask[j] >> i) & 1, node.node(j*SimdWidth+i),tTemp[j][i]);
						}
					}

			}
		}
	}


	std::auto_ptr<BVHType>	_sceneData;

	SampleData& _sampleData;
	RayData& _rayData;
};

}
#endif