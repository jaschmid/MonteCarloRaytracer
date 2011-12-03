/********************************************************/
// FILE: FisheyeCamera.h
// DESCRIPTION: Raytracer FisheyeCamera
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

#ifndef RAYTRACE_FISHEYE_CAMERA_GUARD
#define RAYTRACE_FISHEYE_CAMERA_GUARD

#include <RaytraceCommon.h>


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

}

#endif