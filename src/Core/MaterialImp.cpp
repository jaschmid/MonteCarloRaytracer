#include "headers.h"
#include "MaterialImp.h"

namespace Raytrace {

Material CreateMaterial(const String& name)
{
	return Material(new MaterialImp(name));
}

MaterialImp::MaterialImp(const String& name) : ObjectImp<MaterialImp,IMaterial>(name),
		_ior(1.8f),
		_color(1.0f,1.0f,1.0f,1.0f),
		_diffuseReflect(1.0f),
		_emit(0.0f),
		_fresnel_effect(false),
		_mirror_color(1.0f,1.0f,1.0f,1.0f),
		_specular_reflect(false),
		_translucency(0.0f),
		_transmit_filter(1.0f),
		_transparency(0.0f)
{
}

MaterialImp::~MaterialImp()
{
}

RObjectType IMaterial::ObjectType = ObjectType::Material;

}