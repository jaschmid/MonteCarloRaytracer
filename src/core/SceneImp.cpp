#include "headers.h"
#include "SceneImp.h"

namespace Raytrace {

Scene CreateScene(const String& name)
{
	return Scene(new SceneImp(name));
}

SceneImp::SceneImp(const String& name) : _name(name)
{
}

SceneImp::~SceneImp()
{
}

const String& SceneImp::GetName() const
{
	return _name;
}

Result SceneImp::InsertObject(const Object& object)
{
	if(
		object->GetType() != ObjectType::Camera && 
		object->GetType() != ObjectType::TriMesh &&
		object->GetType() != ObjectType::Material)
		return Result::UnsupportedObjectType;
	
	return Base::InsertObject(object);
}

}