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

namespace Raytrace {

	class SceneReader
	{
	public:
		SceneReader(const Scene& scene)
		{
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
		}

		inline int getNumTriangles() const
		{
			if(_triangles.empty())
				return 0;
			else
			{
				auto it = _triangles.rbegin();
				return it->first.upper();
			}
		}
		
		inline void getTriangle(int i,Triangle& t,int& material) const
		{
			int mat;

			auto it = _triangles.find(i);

			int lower = it->first.lower();

			it->second._triMesh->getTriangle( i - lower, t, mat);
			
			material = it->second._materials[mat];

			return;
		}

		inline int getNumMaterials() const
		{
			return _materials.size();
		}

		inline Material getMaterial(int i) const
		{
			return _materials[i];
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
						container._materials.push_back(it - _materials.begin());
						break;
					}
				assert(found);
			}

			int begin,end;
			/*
			Mesh m;
			m._triMesh = imp;*/
			if(_triangles.empty())
				begin = 0;
			else
			{
				auto it = _triangles.rbegin();
				begin = it->first.upper();
			}
			int num = imp->getNumTriangles();
			end = begin + num;

			_triangles.insert( std::make_pair(boost::icl::interval<int>::right_open( begin, end),container) );
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

		typedef boost::icl::split_interval_map<int,MeshContainer> IntervalMap;
		IntervalMap									_triangles;
		std::vector<Material>						_materials;
	};


}

#endif