/********************************************************/
// FILE: ImportanceSamplers.h
// DESCRIPTION: Various Importance Samplers for lightsources and materials
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

#ifndef RAYTRACE_IMPORTANCE_SAMPLERS_GUARD
#define RAYTRACE_IMPORTANCE_SAMPLERS_GUARD

#include <RaytraceCommon.h>

namespace Raytrace
{

	struct ImportanceSampler
	{
		virtual Vector2 Sample(const Vector2& location) = 0;
		virtual Real GetSamplePDF(const Vector2& v) = 0;
	};



}

#endif