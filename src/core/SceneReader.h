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

	template<class _PrimitiveType> struct LoadedSceneReader
	{
	public:
		typedef _PrimitiveType PrimitiveType;

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

			size_t primitives = getNumPrimitives();
		}

		inline int getNumPrimitives() const
		{
			if(_primitives.empty())
				return 0;
			else
			{
				auto it = _primitives.rbegin();
				return it->first.upper();
			}
		}
		
		inline void getPrimitive(int i,PrimitiveType& t,int& material) const
		{
			int mat;

			auto it = _primitives.find(i);

			int lower = it->first.lower();

			it->second._triMesh->getTriangle( i - lower, t, mat);

			Vector4 a,b,c;

			a.head<3>() = t.point(0);
			b.head<3>() = t.point(1);
			c.head<3>() = t.point(2);
			a.w() = 1.0f; b.w() = 1.0f; c.w() = 1.0f;

			a = _viewMatrix*a;
			b = _viewMatrix*b;
			c = _viewMatrix*c;

			t.setPoint(0, a.head<3>());
			t.setPoint(1, b.head<3>());
			t.setPoint(2, c.head<3>());
			
			material = it->second._materials[mat];

			return;
		}

		inline int getNumMaterials() const
		{
			return (int)_materials.size();
		}

		inline Material getMaterial(int i) const
		{
			return _materials[i];
		}

		inline Vector2u getResolution() const
		{
			return _resolution;
		}

	private:

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


}

#endif