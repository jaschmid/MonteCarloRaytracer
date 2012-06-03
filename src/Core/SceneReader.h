/********************************************************/
// FILE: SceneReader.h
// DESCRIPTION: Provides 1:1 mapping of a scene with material and triangle id's
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

#ifndef RAYTRACE_SCENE_READER_GUARD
#define RAYTRACE_SCENE_READER_GUARD

#include <RaytraceCommon.h>
#include <boost/icl/split_interval_map.hpp>
#include "SceneImp.h"
#include "TriMeshImp.h"
#include "MaterialImp.h"
#include "MathHelper.h"
#include "OutputImp.h"

namespace Raytrace {

	class LoadedSceneReader : public PropertySetImp<LoadedSceneReader,ISceneReader>
	{
	public:
		static const PropertyMap& GetPropertySet()
		{
			const static PropertyMap set = boost::assign::map_list_of
				(SceneReaderProperty_Resolution,Property(&LoadedSceneReader::GetResolution))
				(SceneReaderProperty_FieldOfView,Property(&LoadedSceneReader::GetFieldOfView))
				(SceneReaderProperty_Aspect,Property(&LoadedSceneReader::GetAspect))
				(SceneReaderProperty_MultisampleCount,Property(&LoadedSceneReader::GetMultisampleCount))
				(SceneReaderProperty_PrimitiveType_Triangle,Property(&LoadedSceneReader::GetPrimitiveType));
			return set;
		}

		LoadedSceneReader(const Scene& scene,const Camera& camera,const Output& output,const Vector2u& resolution)
		{
			_scene = scene;
			_camera = camera;
			_output = output;

			_resolution = resolution;

			Object curr = scene->GetFirstObject(ObjectType::Material);
			while(curr.get())
			{
				parseMaterial(curr);

				curr = scene->GetNextObject(curr,ObjectType::Material);
			}

			curr = scene->GetFirstObject(ObjectType::TriMesh);
			while(curr.get())
			{
				parseTriMesh(curr);

				curr = scene->GetNextObject(curr,ObjectType::TriMesh);
			}

			Vector3 from = camera->GetFrom();
			Vector3 to = camera->GetTo();
			Vector3 up = (camera->GetUp() - from).normalized();

			_viewMatrix = FromLookAt(from,to,up);
		}

		virtual size_t GetNumPrimitives() const
		{
			if(_primitives.empty())
				return 0;
			else
			{
				auto it = _primitives.rbegin();
				return it->first.upper();
			}
		}
		
		virtual void GetPrimitive(size_t i,void* pPrimitiveOut) const
		{
			if(pPrimitiveOut == nullptr)
				return;

			PrimitiveTriangle* pOut = (PrimitiveTriangle*)pPrimitiveOut;
			int mat;

			auto it = _primitives.find(i);

			int lower = it->first.lower();

			SimpleTriangle t;

			it->second._triMesh->getTriangle( i - lower, t, mat);

			Vector4 a,b,c;

			a.head<3>() = t.point(0);
			b.head<3>() = t.point(1);
			c.head<3>() = t.point(2);
			a.w() = 1.0f; b.w() = 1.0f; c.w() = 1.0f;

			a = _viewMatrix*a;
			b = _viewMatrix*b;
			c = _viewMatrix*c;

			pOut->_p1 = a.head<3>();
			pOut->_p2 = b.head<3>();
			pOut->_p3 = c.head<3>();
			
			pOut->_material = it->second._materials[mat];

			return;
		}

		virtual size_t GetNumMaterials() const
		{
			return _materials.size();
		}
		
		virtual void			GetMaterial(size_t i,MaterialData* pMaterialOut) const
		{
			if(pMaterialOut == nullptr)
				return;

			pMaterialOut->_color =				_materials[i]->GetColor().head<3>();
			pMaterialOut->_diffuseReflect =		_materials[i]->GetDiffuseReflect();
			pMaterialOut->_emit =				_materials[i]->GetEmit();
			pMaterialOut->_fresnel_effect =		_materials[i]->GetFresnelEffect();
			pMaterialOut->_ior =				_materials[i]->GetIOR();
			pMaterialOut->_specular_reflect =	_materials[i]->GetSpecularReflect();
			pMaterialOut->_translucency =		_materials[i]->GetTranslucency();
			pMaterialOut->_transmit_filter =	_materials[i]->GetTransmitFilter();
			pMaterialOut->_transparency =		_materials[i]->GetTransparency();
			pMaterialOut->_mirror_color =		_materials[i]->GetMirrorColor().head<3>();
			
		}

		virtual Vector2u GetOutputResolution() const
		{
			return _resolution;
		}
		
		virtual size_t			GetNumLights() const
		{
			return 0;
		}

		virtual void			GetLight(size_t i,LightData* pMaterialOut) const
		{
			return;
		}
		
		Vector2u		GetBackgroundRadianceSize() const
		{
			return Vector2u(1,1);
		}

		Format			GetBackgroundRadianceFormat() const
		{
			return BACKGROUND_FORMAT_RGBA_F32;
		}

		void 			GetBackgroundRadianceData(void* pData) const
		{
			(*(Vector4*)pData) = Vector4(10.0f,10.0f,10.0f,0.0f);
		}

	private:
		
		inline Vector2u GetResolution() const 
		{
			return _resolution;
		}
		inline Real GetFieldOfView() const 
		{
			return _camera->GetFov();
		}
		inline Real GetAspect() const 
		{
			return _camera->GetAspect();
		}
		inline u32 GetMultisampleCount() const 
		{
			return 256;
		}
		inline String GetPrimitiveType() const 
		{
			return SceneReaderProperty_PrimitiveType_Triangle;
		}

		void parseMaterial(const Material& material)
		{
			_materials.push_back(material);
		}

		void parseTriMesh(const TriMesh& triMesh)
		{
			boost::intrusive_ptr<TriMeshImp> imp(dynamic_cast<TriMeshImp*>(triMesh.get()));

			MeshContainer container;

			container._triMesh = imp;
			int numMats = imp->getNumMaterials();
			for(int i = 0; i < numMats; ++i)
			{
				Material mat = imp->getMaterial(i);

				int found = false;
				for(auto it = _materials.begin(); it != _materials.end(); ++it)
					if(*it == mat)
					{
						found = true;
						container._materials.push_back((int)(it - _materials.begin()));
						break;
					}
				assert(found);
			}

			int begin,end;
			/*
			Mesh m;
			m._triMesh = imp;*/
			if(_primitives.empty())
				begin = 0;
			else
			{
				auto it = _primitives.rbegin();
				begin = it->first.upper();
			}
			int num = imp->getNumTriangles();
			end = begin + num;

			_primitives.insert( std::make_pair(boost::icl::interval<int>::right_open( begin, end),container) );
		}
		
		struct MeshContainer
		{
			std::vector<int>						_materials;
			boost::intrusive_ptr<TriMeshImp>		_triMesh;

			bool operator ==(const MeshContainer& m2) const
			{
				if(_triMesh == m2._triMesh)
					return true;
				else
					return false;
			}
		};

		Scene										_scene;
		Camera										_camera;
		Output										_output;

		typedef boost::icl::split_interval_map<int,MeshContainer> IntervalMap;
		Matrix4										_viewMatrix;
		IntervalMap									_primitives;
		std::vector<Material>						_materials;
		Vector2u									_resolution;
		public:
		  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	template<> struct SceneReaderAdapter<SimpleTriangle>
	{
	public:
		typedef SimpleTriangle PrimitiveType;
		typedef ISceneReader::MaterialData MaterialData;

		SceneReaderAdapter( const boost::shared_ptr<ISceneReader>& sceneReader) : _sceneReader(sceneReader)
		{
		}
		
		inline size_t getNumPrimitives() const
		{
			return (int)_sceneReader->GetNumPrimitives();
		}
		
		inline void getPrimitive(size_t i,PrimitiveType& t,int& material) const
		{
			ISceneReader::PrimitiveTriangle triangle;
			_sceneReader->GetPrimitive(i,&triangle);

			t.setPoint(0, triangle._p1);
			t.setPoint(1, triangle._p2);
			t.setPoint(2, triangle._p3);
			material = triangle._material;
		}

		inline size_t getNumMaterials() const
		{
			return (int)_sceneReader->GetNumMaterials();
		}
		
		inline size_t getNumLights() const
		{
			return _sceneReader->GetNumLights();
		}

		inline MaterialData getMaterial(size_t i) const
		{
			MaterialData material;
			_sceneReader->GetMaterial(i,&material);
			return material;
		}

		inline ISceneReader::LightData getLight(size_t i) const
		{
			ISceneReader::LightData lightData;
			_sceneReader->GetLight(i,&lightData);
			return lightData;
		}

		inline Vector2u getBackgroundSize() const
		{
			return _sceneReader->GetBackgroundRadianceSize();
		}
		
		inline void getBackgroundData(Vector4* pDataOut) const
		{
			assert( _sceneReader->GetBackgroundRadianceFormat() == ISceneReader::BACKGROUND_FORMAT_RGBA_F32);
			_sceneReader->GetBackgroundRadianceData(pDataOut);
		}

		inline Vector2u getResolution() const
		{
			Vector2u resolution;
			if(_sceneReader->GetPropertyValueTyped(SceneReaderProperty_Resolution,resolution))
			{
				return resolution;
			}
			else
				return Vector2u(240,160);

		}
		
		inline u32 getMultisampleCount() const
		{
			u32 count;
			if(_sceneReader->GetPropertyValueTyped(SceneReaderProperty_MultisampleCount,count))
			{
				return count;
			}
			else
				return 1;

		}
		
		inline Real getFoV() const
		{
			Real fov;
			if(_sceneReader->GetPropertyValueTyped(SceneReaderProperty_FieldOfView,fov))
			{
				return fov;
			}
			else
				return .75f;

		}
		
		inline Real getAspect() const
		{
			Real aspect;
			if(_sceneReader->GetPropertyValueTyped(SceneReaderProperty_Aspect,aspect))
			{
				return aspect;
			}
			else
			{
				Vector2u res = getResolution();

				return (Real)res.x() / (Real)res.y();
			}

		}
	private:

		boost::shared_ptr<ISceneReader> _sceneReader;
	};


}

#endif