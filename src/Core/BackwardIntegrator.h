/********************************************************/
// FILE: BackwardIntegrator.h
// DESCRIPTION: Raytracer Backward Integrator
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

#ifndef RAYTRACE_BACKWARD_INTEGRATOR_GUARD
#define RAYTRACE_BACKWARD_INTEGRATOR_GUARD

#include <RaytraceCommon.h>
#include "EngineBase.h"
#include "RayData.h"
#include "SampleData.h"
#include "SceneReader.h"
#include "IIntegrator.h"
#include "SplitUseWorkQueue.h"
#include "chunk_vector.h"
#include "FisheyeCamera.h"

namespace Raytrace {
	
struct Light
{
	Light(const Vector3& l,const Vector3& c) : _location(l), _color(c) {}
	Vector3					_location;
	Vector3					_color;
};

static const std::array<Light,3> Lights = { 
	Light( Vector3(0.0f,.8f,1.0f) , Vector3(.5f,.5f,.5f) ),
	Light( Vector3(.5f,.8f,1.0f) , Vector3(.5f,.5f,.5f) ),
	Light( Vector3(0.0f,.5f,1.0f) , Vector3(.5f,.5f,.5f) )
};

static const Real Epsilon = 0.001f;

template<class _SampleData,class _RayData,class _SceneReader,int _NumPathsPerBlock> struct BackwardIntegrator : public IIntegrator<_SampleData,_RayData,_SceneReader>
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef _SceneReader SceneReader;

	typedef typename RayData::PrimitiveType PrimitiveType;
	typedef typename RayData::RayType RayType;

	typedef typename RayData::PrimitiveRelativeIntersection		IntersectionRelative;
	typedef typename RayData::AbsoluteIntersectionLocation		IntersectionAbsolute;

	typedef typename RayData::PrimitiveUserData					PrimitiveIdentifier;

	static const size_t ChunkSize = 8*1024;

	static const size_t NumPathsPerBlock = _NumPathsPerBlock;

	struct IndirectNode
	{
		Vector3					_parentDir;
		IntersectionRelative	_intersectionRelative;
		PrimitiveIdentifier		_id;
		IntersectionAbsolute	_intersectionAbsolute;
		Vector3					_filter;
	};

	struct DirectNode
	{
		u32						_shadow;
		Vector3					_color;
	};

	struct Path
	{
		typename SampleData::SampleInput		_sample;
		Vector3						_color;
		size_t						_threadId;

		size_t						_firstIndirect;
		size_t						_numIndirect;
		size_t						_firstDirect;
		size_t						_numDirect;
	};

	typedef chunked_vector<DirectNode,ChunkSize> DirectNodeArray;
	typedef chunked_vector<IndirectNode,ChunkSize> IndirectNodeArray;
	
	struct ThreadData
	{
		std::array<DirectNodeArray,2>	_directNodes;
		std::array<IndirectNodeArray,2> _indirectNodes;
	};

	struct PrimitiveData
	{
		int			_material;
		Vector3		_normal;
		PrimitiveType _primitive;
	};

	struct MaterialSettings
	{
		Vector3	_color;
		Vector3 _mirrorColor;

		Real	_diffuseReflect;
		Real	_specularReflect;
		Real	_emit;
		
		Real	_indexOfRefraction;
		Real	_translucency;
		Real	_transparency;
	};

	typedef SplitUseWorkQueue<Path,NumPathsPerBlock> PathsArrayType;


	
	inline BackwardIntegrator() : _sampleData(nullptr),_rayData(nullptr),_writePathArray(0),_readPathArray(1)
	{
	}
	
	void InitializePrepareST(size_t numThreads,const SceneReader& scene,SampleData& sampleData,RayData& rayData) 
	{
		PrimitiveType dummy;

		_primitives.resize(scene->getNumPrimitives());
		_materials.resize(scene->getNumMaterials());
		_threads.resize(numThreads);
		_pathArrays[_readPathArray].prepare(numThreads);
		_pathArrays[_writePathArray].prepare(numThreads);

		_camera.Initialize( Vector2(.75f,.75f));

		for(size_t i = 0; i < _materials.size(); ++i)
		{
			Material mat = scene->getMaterial((int)i);

			_materials[i]._color = mat->GetColor().head<3>();
			_materials[i]._mirrorColor = mat->GetMirrorColor().head<3>();
			_materials[i]._diffuseReflect = mat->GetDiffuseReflect();
			_materials[i]._specularReflect = mat->GetSpecularReflect();
			_materials[i]._emit = mat->GetEmit();
			_materials[i]._indexOfRefraction = mat->GetIOR();
			_materials[i]._translucency = mat->GetTranslucency();
			_materials[i]._transparency = mat->GetTransparency();

		}

		for(size_t i = 0; i < _primitives.size(); ++i)
		{
			scene->getPrimitive((int)i,_primitives[i]._primitive,_primitives[i]._material);

			assert( _primitives[i]._material < (int)_materials.size() );
			
			Vector3 A = _primitives[i]._primitive.point(0);
			Vector3 AB = _primitives[i]._primitive.point(1) - _primitives[i]._primitive.point(0);
			Vector3 AC = _primitives[i]._primitive.point(2) - _primitives[i]._primitive.point(0);

			_primitives[i]._normal = AB.cross(AC).normalized();
		}

		_sampleData = &sampleData;
		_rayData = &rayData;
	}
	void InitializeMT(size_t threadId) 
	{
	}
	void InitializeCompleteST() 
	{
	}

	void IntegratePrepareST() 
	{
	}

	void IntegrateMT(size_t threadId) 
	{

		ThreadData& threadDataWrite = _threads[threadId];
		IndirectNodeArray& indirectNodeWrite = threadDataWrite._indirectNodes[_writePathArray];
		DirectNodeArray& directNodeWrite = threadDataWrite._directNodes[_writePathArray];

		
		typename SampleData::SampleInput newSample;

		indirectNodeWrite.clear();
		directNodeWrite.clear();

		// process new samples

		while(_sampleData->popGeneratedSample(threadId,newSample))
		{
			Path newPath;

			PathsArrayType& pathWrite = _pathArrays[_writePathArray];

			// create a new indirect node
			RayType cameraRay = _camera( _sampleData->getSampleValueImageXY(newSample,threadId));

			size_t nodeId = indirectNodeWrite.size();
			indirectNodeWrite.push_back(IndirectNode());
			IndirectNode& node = indirectNodeWrite.back();
			node._filter = Vector3( 1.0f, 1.0f, 1.0f);
			node._parentDir = - cameraRay.direction();

			_rayData->pushRay(
				threadId,
				cameraRay,
				&node._intersectionAbsolute,
				nullptr,
				&node._intersectionRelative,
				&node._id
				);

			// create a new sample

			newPath._sample = newSample;
			newPath._color = Vector3(0.0f,0.0f,0.0f);
			newPath._threadId = threadId;
			newPath._firstIndirect = nodeId;
			newPath._numIndirect = 1;
			newPath._firstDirect = -1;
			newPath._numDirect = 0;

			pathWrite.pushElement(newPath,threadId);
		}


		PathsArrayType& pathRead = _pathArrays[_readPathArray];

		// process active samples

		// go to first element
		pathRead.advanceElement(threadId);

		Path* currPath;

		while(currPath = pathRead.currElement(threadId))
		{
			processPath(*currPath,threadId);

			pathRead.advanceElement(threadId);
		}
	}
	bool IntegrateCompleteST() 
	{
		_pathArrays[_readPathArray].clear();
		_pathArrays[_writePathArray].reset();

		size_t temp = _writePathArray;
		_writePathArray = _readPathArray;
		_readPathArray = temp;

		return true;
	}

	inline Vector3 getBackgroundColor(const Vector3& direction)
	{
		return direction*.5f + Vector3(.5f,.5f,.5f);
	}

	inline void processPath(Path& path,size_t threadId)
	{
		ThreadData& threadDataRead = _threads[path._threadId];
		ThreadData& threadDataWrite = _threads[threadId];
		PathsArrayType& pathWrite = _pathArrays[_writePathArray];
		
		IndirectNodeArray& indirectNodeWrite = threadDataWrite._indirectNodes[_writePathArray];
		DirectNodeArray& directNodeWrite = threadDataWrite._directNodes[_writePathArray];

		const IndirectNodeArray& indirectNodeRead = threadDataRead._indirectNodes[_readPathArray];
		const DirectNodeArray& directNodeRead = threadDataRead._directNodes[_readPathArray];

		if(path._numIndirect ==0)
		{
			completePathDirectLight(path,directNodeRead);
			_sampleData->pushCompletedSample( threadId, typename SampleData::SampleOutput( path._sample, path._color) );
		}
		else
		{
			completePathDirectLight(path,directNodeRead);
			Path newPath;

			newPath._sample = path._sample;
			newPath._color = path._color;
			newPath._threadId = threadId;
			newPath._firstIndirect = indirectNodeWrite.size();
			newPath._firstDirect = directNodeWrite.size();

			const size_t end = path._firstIndirect + path._numIndirect;

			for(size_t i = path._firstIndirect; i < end; ++i)
				newPath._color += createNodeChildren(threadId,indirectNodeRead[i],indirectNodeWrite,directNodeWrite);
				
			newPath._numIndirect = indirectNodeWrite.size() - newPath._firstIndirect;
			newPath._numDirect = directNodeWrite.size() - newPath._firstDirect;
				
			pathWrite.pushElement(newPath,threadId);
		}
	}
	inline void completePathDirectLight(Path& path,const DirectNodeArray& directNodeRead)
	{
		Vector3 colorChange(0.0f,0.0f,0.0f);

		size_t end = path._firstDirect + path._numDirect;

		for(size_t i = path._firstDirect; i < end; ++i)
			if(!directNodeRead[i]._shadow)
				colorChange += directNodeRead[i]._color;

		path._color += colorChange;
	}
	
	inline Vector3 createNodeChildren(size_t threadId,const IndirectNode& node,IndirectNodeArray& indirectNodeWrite,DirectNodeArray& directNodeWrite)
	{
		if(node._id != -1)
		{
			const PrimitiveData& primitive = _primitives[node._id];
			const MaterialSettings& material = _materials[primitive._material];

			// direct lighting (added next step)

			for(auto il = Lights.begin(); il != Lights.end(); ++il)
			{
				Vector3 color;
				CalculateLighting( *il, node, color);
				if(color.x() >= Epsilon || color.y() >= Epsilon || color.z() >= Epsilon)
				{
					directNodeWrite.push_back(DirectNode());
					DirectNode& direct = directNodeWrite.back();
					direct._color = color;

					RayType ray;
							
					ray.setOrigin( node._intersectionAbsolute);
					ray.setDirection( (il->_location-node._intersectionAbsolute).normalized() );
					ray.setLength( (il->_location-node._intersectionAbsolute).norm() );

					_rayData->pushRay(threadId, ray, &direct._shadow);
				}
			}

			// reflected
			
			Vector3 reflectFactor = (material._specularReflect * material._mirrorColor.array() * node._filter.array());
			

			if(reflectFactor.squaredNorm() > 0.0001f )
			{
				Vector3 reflected = 2.0f*primitive._normal.dot(node._parentDir)*primitive._normal - node._parentDir;

				reflected.normalize();
							
				RayType reflectedRay;
				reflectedRay.setOrigin( node._intersectionAbsolute );
				reflectedRay.setDirection( reflected );
				reflectedRay.setLength( -1 );

				indirectNodeWrite.push_back(IndirectNode());
				IndirectNode& newNode = indirectNodeWrite.back();
				newNode._filter = reflectFactor;
				newNode._parentDir = - reflected;

				_rayData->pushRay(
					threadId,
					reflectedRay,
					&newNode._intersectionAbsolute,
					nullptr,
					&newNode._intersectionRelative,
					&newNode._id
					);
			}

			
			// refracted

			Vector3 refractFactor = (material._transparency * material._color.array() * node._filter.array());
			if(refractFactor.squaredNorm() > 0.0001f )
			{
				Real n;
				
				Real cosTheta1 = primitive._normal.dot(node._parentDir);

				if(primitive._normal.dot(node._parentDir) > 0.0f)
					n = 1.0f/ material._indexOfRefraction;
				else
					n = material._indexOfRefraction;
				
				Real cosTheta2sq =  1.0f - n*n*(1.0f - cosTheta1*cosTheta1);

				if(cosTheta2sq >= 0.0f)
				{
					Real cosTheta2 = sqrtf(cosTheta2sq);
					Vector3 refracted;
					
					if(cosTheta1 >= 0.0f)
						refracted = (n*cosTheta1 - cosTheta2 ) * primitive._normal - n * node._parentDir ;
					else
						refracted = (-n*cosTheta1 - cosTheta2 ) * primitive._normal - n * node._parentDir ;

					refracted.normalize();

					RayType refractedRay;
					refractedRay.setOrigin( node._intersectionAbsolute );
					refractedRay.setDirection( refracted );
					refractedRay.setLength( -1 );

					indirectNodeWrite.push_back(IndirectNode());
					IndirectNode& newNode = indirectNodeWrite.back();
					newNode._filter = refractFactor;
					newNode._parentDir = - refracted;

					_rayData->pushRay(
						threadId,
						refractedRay,
						&newNode._intersectionAbsolute,
						nullptr,
						&newNode._intersectionRelative,
						&newNode._id
						);

				}
			}
						
			// emitted

			return Vector3(material._color.array() * node._filter.array() * material._emit);
		}
		else
			return getBackgroundColor(-node._parentDir);
	}
	
	inline void CalculateLighting( const Light& light, const IndirectNode& node, Vector3& color)
	{
		const PrimitiveData& primitive = _primitives[node._id];
		const MaterialSettings& material = _materials[primitive._material];
		
		Real remainder = 1.0f-node._intersectionRelative.x()-node._intersectionRelative.y();

		Vector3 location = primitive._primitive.point(0) * remainder + primitive._primitive.point(1) * node._intersectionRelative.x() + primitive._primitive.point(2) * node._intersectionRelative.y();

		Vector3 lightDir(light._location-location);


		Real lightDistance = lightDir.squaredNorm();
		lightDir.normalize();

		Real diffuseFactor = std::min<Real>(1.0f,std::max<Real>(0.0f,lightDir.dot( primitive._normal ))) * material._diffuseReflect;


		Vector3 half = (lightDir + node._parentDir).normalized();

		Real specularFactor = powf( std::min<Real>(1.0f,std::max<Real>(0.0f,half.dot( primitive._normal ))), 25.0f ) * material._specularReflect;
		

		color = ((diffuseFactor*material._color + specularFactor*material._mirrorColor).array() * light._color.array() * node._filter.array()).matrix();
	}

	SampleData*						_sampleData;
	RayData*						_rayData;
	FisheyeCamera<RayType>			_camera;
	std::vector<ThreadData>			_threads;
	std::array<PathsArrayType,2>	_pathArrays;
	std::vector<PrimitiveData>		_primitives;
	std::vector<MaterialSettings>	_materials;
	size_t							_readPathArray;
	size_t							_writePathArray;

};

}
#endif