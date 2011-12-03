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
	return (int)(_vertexLocations.size() -1);
}

int TriMeshImp::PushTriangle(int vertex1,int vertex2,int vertex3,Material material)
{
	int material_index = -1;
	for(auto it = _materials.begin(); it != _materials.end(); ++it)
		if(*it == material)
		{
			material_index = (int)(it - _materials.begin());
			break;
		}
	if(material_index == -1)
	{
		material_index = (int)(_materials.size());
		_materials.push_back(material);
	}

	_triangleVertices.push_back(Vector3i(vertex1,vertex2,vertex3));
	_triangleMaterials.push_back(material_index);
	return (int)(_triangleVertices.size() - 1);
}

RObjectType ITriMesh::ObjectType = ObjectType::TriMesh;
}