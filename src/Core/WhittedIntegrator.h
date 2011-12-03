/********************************************************/
// FILE: WhittedIntegrator.h
// DESCRIPTION: Raytracer Whitted Integrator
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

#ifndef RAYTRACE_WHITTED_INTEGRATOR_GUARD
#define RAYTRACE_WHITTED_INTEGRATOR_GUARD

#include <RaytraceCommon.h>
#include "EngineBase.h"
#include "ConcurrentStorage.h"
#include "RayData.h"
#include "SampleData.h"
#include "SceneReader.h"
#include "IIntegrator.h"


namespace Raytrace {
	

template<class _RayType> struct FisheyeCamera
{
	inline void Initialize(const Vector2& fov)
	{
		_fov = fov;
		_angleStep = Vector2(_fov.x()*.5f,_fov.y()*.5f);
	}

	inline _RayType operator()(const Vector2& location)
	{
		Vector2 rayAngle( _angleStep.x() * (location.x()*2.0f-1.0f) , _angleStep.y() * (location.y()*2.0f-1.0f) );
		_RayType ray;
		
		ray.setOrigin( Vector3(0.0f, 0.0f, 0.0f) );
		ray.setDirection(Vector3(-sinf(rayAngle.x()), -sinf(rayAngle.y()), cosf(rayAngle.x())*cosf(rayAngle.y()) ).normalized() );
		ray.setLength(-1.0f);

		return ray;
	}

	Vector2					_fov;
	Vector2					_angleStep;
};

struct Light
{
	Light(const Vector3& l,const Vector3& c) : _location(l), _color(c) {}
	Vector3					_location;
	Vector3					_color;
};

static const std::array<Light,3> Lights = { 
	Light( Vector3(0,.8,1) , Vector3(.5,.5,.5) ),
	Light( Vector3(.5,.8,1) , Vector3(.5,.5,.5) ),
	Light( Vector3(0,.5,1) , Vector3(.5,.5,.5) )
};

static const Real Epsilon = 0.001f;

template<class _SampleData,class _RayData,class _SceneReader> struct WhittedIntegrator : public IIntegrator<_SampleData,_RayData,_SceneReader>
{
	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef _SceneReader SceneReader;

	typedef typename RayData::PrimitiveType PrimitiveType;
	typedef typename RayData::RayType RayType;

	typedef typename RayData::PrimitiveRelativeIntersection		IntersectionRelative;
	typedef typename RayData::AbsoluteIntersectionLocation		IntersectionAbsolute;

	typedef typename RayData::PrimitiveUserData					PrimitiveIdentifier;
	

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
		std::deque<IndirectNode>	_indirect;
		std::deque<DirectNode>		_direct;
	};
	
	inline WhittedIntegrator() : _sampleData(nullptr),_rayData(nullptr)
	{
	}
	
	void InitializePrepareST(size_t numThreads,const SceneReader& scene,SampleData& sampleData,RayData& rayData) 
	{
		PrimitiveType dummy;

		_primitives.resize(scene->getNumPrimitives());
		_materials.resize(scene->getNumMaterials());

		_camera.Initialize( Vector2(.75f,.75f));
		

		for(size_t i = 0; i < _materials.size(); ++i)
		{
			Material mat = scene->getMaterial((int)i);

			_materials[i]._color = mat->GetColor().head<3>();
			_materials[i]._mirrorColor = mat->GetMirrorColor().head<3>();
			_materials[i]._diffuseReflect = mat->GetDiffuseReflect();
			_materials[i]._specularReflect = mat->GetSpecularReflect();
			_materials[i]._emit = mat->GetEmit();
			_materials[i]._indexOfRefraction = 1.8f;
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
		if(_sampleData->getSampleBufferSize() > _pathBuffer.size())
			_pathBuffer.resize(_sampleData->getSampleBufferSize());
	}
	void IntegrateMT(size_t threadId) 
	{
		auto it = _sampleData->beginActive(threadId);
		auto end = _sampleData->endActive(threadId);
		
		static const Vector3 lightSource(0.0f,0.0f,1.5f);

		while(it != end)
		{
			size_t index = it.index();
			Path& path = _pathBuffer[it.index()];

			if(path._direct.size() == 0 && path._indirect.size() == 0)
			{
				RayType cameraRay = _camera( Vector2( it->getX(), it->getY() ));

				path._indirect.push_back(IndirectNode());
				IndirectNode& node = path._indirect.back();
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

				it->setResult( Vector3( 0.0f, 0.0f, 0.0f) );

				it.keepSample();
			}
			else
			{
				Vector3 colorChange(0.0f,0.0f,0.0f);

				for(auto id = path._direct.begin(); id != path._direct.end();id++)
					if(!id->_shadow)
						colorChange += id->_color;

				path._direct.clear();

				size_t end = path._indirect.size();

				//path._direct.reserve( path._indirect.size() * Lights.size());

				for(size_t i = 0; i < end; ++i)
				{
					IndirectNode& node = path._indirect.front();

					if(node._id != -1)
					{
						// direct lighting (added next step)

						for(auto il = Lights.begin(); il != Lights.end(); ++il)
						{
							Vector3 color;
							CalculateLighting( *il, node, color);
							if(color.x() >= Epsilon || color.y() >= Epsilon || color.z() >= Epsilon)
							{
								path._direct.push_back(DirectNode());
								path._direct.back()._color = color;

								RayType ray;
							
								ray.setOrigin( node._intersectionAbsolute);
								ray.setDirection( (il->_location-node._intersectionAbsolute).normalized() );
								ray.setLength( (il->_location-node._intersectionAbsolute).norm() );

								_rayData->pushRay(threadId, ray, &path._direct.back()._shadow);
							}
						}
						
						const PrimitiveData& primitive = _primitives[node._id];
						const MaterialSettings& material = _materials[primitive._material];

						// reflected

						Vector3 reflectFactor = (material._specularReflect * material._mirrorColor.array() * node._filter.array());

						if(reflectFactor.squaredNorm() > 0.001f )
						{
							Vector3 reflected = 2.0f*primitive._normal*primitive._normal.dot(node._parentDir) - node._parentDir;

							reflected.normalize();
							
							RayType reflectedRay;
							reflectedRay.setOrigin( node._intersectionAbsolute );
							reflectedRay.setDirection( reflected );
							reflectedRay.setLength( -1 );

							path._indirect.push_back(IndirectNode());
							IndirectNode& newNode = path._indirect.back();
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
						
						// emitted

						colorChange += Vector3(material._color.array() * node._filter.array() * material._emit);
					}
					else // we missed scene objects
						colorChange += Vector3(getBackgroundColor(node._parentDir).array() * node._filter.array());

					path._indirect.pop_front();
				}
				
				it->setResult( it->getResult() + colorChange);

				if(path._direct.size() == 0 && path._indirect.size() == 0)
					it.completeSample();
				else
					it.keepSample();
			}
		}
	}
	bool IntegrateCompleteST() 
	{
		return true;
	}

	inline Vector3 getBackgroundColor(const Vector3& direction)
	{
		return direction*.5f + Vector3(.5f,.5f,.5f);
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

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	SampleData* _sampleData;
	RayData* _rayData;
	FisheyeCamera<RayType> _camera;

	std::vector<Path>		_pathBuffer;

	struct PrimitiveData
	{
		int			_material;
		Vector3		_normal;
		PrimitiveType _primitive;
	};

	std::vector<PrimitiveData>	_primitives;

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

	std::vector<MaterialSettings> _materials;
};

}
#endif