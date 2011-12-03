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
#include "MathHelper.h"
#include "RayData.h"
#include "BVH.h"
#include "RayAABBIntersection.h"
#include "RayTriangleIntersection.h"
#include "IIntersector.h"
#include "SceneReader.h"
#include "static_vector.h"

namespace Raytrace {
	
template<class _RayData,class _SceneReader,int _SimdWidth,int _NodeArraySize,int _LeafArraySize> struct BVHIntersector : public IIntersector<_RayData,_SceneReader>
{
	typedef _RayData RayData;
	typedef _SceneReader SceneReader;

	static const size_t NodeArraySize = _NodeArraySize;
	static const size_t LeafArraySize = _LeafArraySize;
	static const size_t SimdWidth = _SimdWidth;
	static const size_t NodeWidth = SimdWidth*NodeArraySize;
	static const size_t LeafWidth = SimdWidth*LeafArraySize;

	typedef typename RayData::RayType BaseRayType;
	typedef typename RayData::PrimitiveType BasePrimitiveType;
	typedef AABB	BaseVolumeType;

	typedef SimdType<int,SimdWidth>	Scalari_T;
	typedef SimdType<float,SimdWidth>	Scalar_T;
	typedef typename Vector2v<SimdWidth>::type Vector2_T;
	typedef typename Vector3v<SimdWidth>::type Vector3_T;

	typedef mpl::vector<
				RaySignModePrecompute,
				RayLengthModeNone,
				RayInvDirModePrecompute,
				RayScalarType<Scalar_T>,
				RayDimensions<BaseRayType::Dimensions> > RayOptions;
	typedef mpl::vector<
				TriangleBaseModeBarycentricPlane,
				TriangleScalarType<Scalar_T>,
				TriangleDimensions<BasePrimitiveType::Dimensions>,
				TriangleUserDataType<Scalari_T>	> PrimitiveOptions;

	typedef mpl::vector< TriangleUserDataType<int>	> PrimitiveUserOptions;

	typedef typename BaseRayType::adapt<RayOptions>::type		RayType;

	typedef typename BasePrimitiveType::adapt<PrimitiveUserOptions>::type UserPrimitiveType;

	typedef typename BasePrimitiveType::adapt<PrimitiveOptions>::type	PrimitiveType;
	typedef AABBAccel<SimdWidth>	VolumeType;
	
	template<class _RayType> struct RayTypeInfo
	{

	};

	template<> struct RayTypeInfo<AnyHitRay>
	{
		typedef typename BVHIntersector::RayType type;
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
		typedef typename BVHIntersector::RayType type;
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
	

	typedef ContainerMultiplier<UserPrimitiveType,PrimitiveType,SimdWidth,LeafArraySize> PrimitiveContainer;
	typedef ContainerMultiplier<typename VolumeType::Minimum,VolumeType,SimdWidth,NodeArraySize> VolumeContainer;

	typedef BVH<UserPrimitiveType,typename VolumeType::Minimum,LeafWidth,NodeWidth,PrimitiveContainer,VolumeContainer> BVHType;
	
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
	
	void InitializePrepareST(size_t numThreads,const SceneReader& scene,RayData& rayData) 
	{
		BVHType::Constructor constructor(64);

		int num = scene->getNumPrimitives();
		for(int i = 0; i< num; ++i)
		{
			BasePrimitiveType tri;
			int material;
			scene->getPrimitive(i,tri,material);
			
			BaseVolumeType volume(tri);

			Vector3 centroid = (tri.point(0) + tri.point(1) + tri.point(2))/3.0f;
			UserPrimitiveType triUser(tri);
			triUser.setUser( i + 1 );
			constructor.addElement( triUser, centroid, volume );
		}

		_sceneData.reset( new BVHType(constructor) );

		_rayData = &rayData;
	}
	void InitializeMT(size_t threadId) 
	{
	}
	void InitializeCompleteST() 
	{
	}

	void IntersectPrepareST() 
	{
	}
	void IntersectMT(size_t threadId) 
	{
		unsigned int prevCSR = _mm_getcsr();
		_mm_setcsr(0xffc0);

		doIntersections<AnyHitRay>(threadId);
		doIntersections<FirstHitRay>(threadId);

		_mm_setcsr(prevCSR);
	}
	void IntersectCompleteST() 
	{
	}
	
	template<class _RayType> inline void doIntersections(int threadId)
	{
		typename RayData::Element<_RayType> element;
		while(_rayData->popRay<_RayType>(threadId,element))
			processRay<_RayType>(element);
	}

	template<class _RayType> void processRay(const typename RayData::Element<_RayType>& it);


	template<> void processRay<AnyHitRay>(const typename RayData::Element<AnyHitRay>& element)
	{
		const BaseRayType&									rayBase = element.ray;
		static_vector<typename BVHType::nodeIterator,128 >	stack;
		std::array<BaseRayType,SimdWidth>					rayArray;
		bool												found = false;

		for(int i = 0; i < SimdWidth; ++i)
			rayArray[i] = rayBase;

		RayTypeInfo<AnyHitRay>::type ray(rayArray);

		Scalar_T tTemp(rayBase.length() > .0f ? rayBase.length() : std::numeric_limits<Real>::infinity());

		stack.push_back(_sceneData->root());

		while(!stack.empty())
		{
			BVHType::nodeIterator node = stack.back();
			stack.pop_back();

			if(node.isLeaf())
			{
				if(RayTypeInfo<AnyHitRay>::intersector_primitive()(ray, node.leaf(), tTemp))
				{
					found = true;
					break;
				}
			}
			else
			{
				std::array< RayTypeInfo<AnyHitRay>::intersector_volume::BooleanMask ,NodeArraySize> resultMask;
				
				RayTypeInfo<AnyHitRay>::intersector_volume()(ray, node.volumes(), tTemp,resultMask);

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

		if(found)
			(*element.resultOut) = 1;
		else
			(*element.resultOut) = 0;
	}
	
	template<> void processRay<FirstHitRay>(const typename RayData::Element<FirstHitRay>& element)
	{
		const BaseRayType& rayBase = element.ray;
		ActiveStack<128> stack;

		std::array<BaseRayType,SimdWidth> rayArray;

		for(int i = 0; i < SimdWidth; ++i)
			rayArray[i] = rayBase;

		RayTypeInfo<FirstHitRay>::type ray(rayArray);

		Scalar_T tTemp(rayBase.length() > .0f ? rayBase.length() : std::numeric_limits<Real>::infinity());
		Vector2_T baryTemp;
		Scalari_T triIds(0);
		
		stack.push(_sceneData->root(),0.0f);
		while(!stack.empty())
		{
			BVHType::nodeIterator node = stack.getLowest();
			if(node.isLeaf())
			{
				bool found = false;

				if(RayTypeInfo<FirstHitRay>::intersector_primitive()(ray, node.leaf(), tTemp, baryTemp,triIds))
					found = true;

				if(found)
					stack.compact(tTemp[0]);
			}
			else
			{

				std::array<Scalar_T,NodeArraySize> tTempArr;
				std::array< RayTypeInfo<FirstHitRay>::intersector_volume::BooleanMask ,NodeArraySize> resultMask;
				for(int i = 0; i < NodeArraySize; ++i)
					tTempArr[i] = tTemp;

				
				RayTypeInfo<FirstHitRay>::intersector_volume()(ray, node.volumes(), tTempArr, resultMask);
				
				for(size_t j = 0; j < NodeArraySize; ++j)
					if(resultMask[j])
					{
						for(size_t i = 0; i < SimdWidth; ++i)
						{
							//node.prefetchChild(j*SimdWidth+i);
							stack.conditional_push( (resultMask[j] >> i) & 1, node.node(j*SimdWidth+i),tTempArr[j][i]);
						}
					}

			}
		}
		
		Real t = std::numeric_limits<Real>::infinity();
		Vector2 bary;
		int triId = 0;

		for(int i = 0; i < SimdWidth; ++i)
			if(triIds[i] != 0)
			{
				t = tTemp[i];
				bary.x() = baryTemp.x()[i];
				bary.y() = baryTemp.y()[i];
				triId = triIds[i];
			}
		
		if(triId != 0)
		{
			if(element.absoluteIntersectionLocation)(*element.absoluteIntersectionLocation) = rayBase.origin()+ rayBase.direction()*t;
			if(element.rayRelativeIntersectionLocation)(*element.rayRelativeIntersectionLocation) = t;
			if(element.primitiveRelativeIntersectionLocation)(*element.primitiveRelativeIntersectionLocation) = bary;
			if(element.primitiveIdentifier)(*element.primitiveIdentifier) = triId - 1;
		}
		else
		{
			if(element.absoluteIntersectionLocation)(*element.absoluteIntersectionLocation) = Vector3(0.0f,0.0f,0.0f);
			if(element.rayRelativeIntersectionLocation)(*element.rayRelativeIntersectionLocation) = -1.0f;
			if(element.primitiveRelativeIntersectionLocation)(*element.primitiveRelativeIntersectionLocation) = Vector2(0.0f,0.0f);
			if(element.primitiveIdentifier)(*element.primitiveIdentifier) = -1;
		}
	}

	__declspec(noinline) void processNode(const typename RayTypeInfo<FirstHitRay>::type& ray,int raysigns,const typename BVHType::nodeIterator& it,Scalari_T& triId,Scalar_T& t,Vector2_T& bary)
	{
	}
	
	std::auto_ptr<BVHType>	_sceneData;

	RayData* _rayData;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
/*
template<class _RayData,class _SceneReader,int _SimdWidth,int _NodeArraySize,int _LeafArraySize> struct BVHIntersectorEngine : public IIntersector<_RayData,_SceneReader>
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
	



	std::auto_ptr<BVHType>	_sceneData;

	SampleData& _sampleData;
	RayData& _rayData;
};
*/
}
#endif