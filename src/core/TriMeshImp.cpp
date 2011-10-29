#include "headers.h"
#include "TriMeshImp.h"

namespace Raytrace {

TriMesh CreateTriMesh(const String& name)
{
	return TriMesh(new TriMeshImp(name));
}

TriMeshImp::TriMeshImp(const String& name) : ObjectImp<TriMeshImp,ITriMesh>(name)
{
}

TriMeshImp::~TriMeshImp()
{
}


int TriMeshImp::PushVertex(const Vector3& location)
{
	_vertexLocations.push_back(location);
	return _vertexLocations.size() -1;
}

int TriMeshImp::PushTriangle(int vertex1,int vertex2,int vertex3,Material material)
{
	int material_index = -1;
	for(auto it = _materials.begin(); it != _materials.end(); ++it)
		if(*it == material)
		{
			material_index = it - _materials.begin();
			break;
		}
	if(material_index == -1)
	{
		material_index = _triangleMaterials.size();
		_materials.push_back(material);
	}

	_triangleVertices.push_back(Math::Vector3<int>(vertex1,vertex2,vertex3));
	_triangleMaterials.push_back(material_index);
	return _triangleVertices.size() - 1;
}

RObjectType ITriMesh::ObjectType = ObjectType::TriMesh;
}