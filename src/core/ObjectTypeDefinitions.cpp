#include "headers.h"

namespace Raytrace {

	ObjectType const ObjectType::Camera(0,String("Camera"));
	ObjectType const ObjectType::Output(2,String("Output"));
	ObjectType const ObjectType::TriMesh(3,String("TriMesh"));
	ObjectType const ObjectType::Material(4,String("Material"));

}