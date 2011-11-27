#include "headers.h"
#include "CameraImp.h"

namespace Raytrace {

Camera CreateCamera(const String& name)
{
	return Camera(new CameraImp(name));
}

CameraImp::CameraImp(const String& name) : 
	Base(name),
	_fov(3.14f*60.0f/180.0f),
	_aspect(3.0f/4.0f),
	_from(0.0f,0.0f,-1.0f),
	_to(0.0f,0.0f,0.0f),
	_up(0.0f,1.0f,0.0f),
	_aperture(0.0f),
	_focal(1.0f),
	_bokehRotation(0.0f)
{
}

CameraImp::~CameraImp()
{
}

Result CameraImp::InsertObject(const Object& object)
{
	if(object->GetType() != ObjectType::Output)
		return Result::UnsupportedObjectType;

	return Base::InsertObject(object);
}

RObjectType ICamera::ObjectType = ObjectType::Camera;

}