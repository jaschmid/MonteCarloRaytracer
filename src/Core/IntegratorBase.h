/********************************************************/
// FILE: IntegratorBase.h
// DESCRIPTION: Functions all Integrators need
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

#ifndef RAYTRACE_INTEGRATOR_BASE_GUARD
#define RAYTRACE_INTEGRATOR_BASE_GUARD

#include <RaytraceCommon.h>
#include "EngineBase.h"
#include "RayData.h"
#include "SampleData.h"
#include "SceneReader.h"
#include "IIntegrator.h"
#include "SplitUseWorkQueue.h"
#include "chunk_vector.h"
#include "FisheyeCamera.h"
#include "math.h"


namespace Raytrace {

static const Real Epsilon = 0.001f;

template<class _SampleData,class _RayData,class _SceneReader> struct IntegratorBase : public IIntegrator<_SampleData,_RayData,_SceneReader>
{
	typedef Eigen::Array<Real,3,1> ColorArray;

	typedef _SampleData SampleData;
	typedef _RayData RayData;
	typedef _SceneReader SceneReader;

	typedef typename RayData::PrimitiveType PrimitiveType;
	typedef typename RayData::RayType RayType;

	typedef typename RayData::PrimitiveRelativeIntersection		IntersectionRelative;
	typedef typename RayData::AbsoluteIntersectionLocation		IntersectionAbsolute;

	typedef typename RayData::PrimitiveUserData					PrimitiveIdentifier;

	struct Light
	{
		Light(const Vector3& l,const Vector3& c) : _location(l), _color(c) {}
		Light() {}
		Vector3					_location;
		Vector3					_color;
	};

	struct PrimitiveData
	{
		int			_material;
		Vector3		_normal;
		PrimitiveType _primitive;
	};

	struct MaterialSettings
	{
		ColorArray	_color;
		ColorArray  _mirrorColor;
		ColorArray	_filterColor;

		Real	_diffuseReflect;
		Real	_specularReflect;
		Real	_emit;

		Real	_specularPower;
		Real	_refractionPower;
		
		Real	_indexOfRefraction;
		Real	_transparency;
	};

	
	typedef Real (*pdfAt)(const Vector3&);

	void InitializeSceneData(const SceneReader& scene) 
	{
		PrimitiveType dummy;

		_primitives.resize(scene->getNumPrimitives());
		_materials.resize(scene->getNumMaterials());
		_lights.resize(scene->getNumLights());

		for(size_t i = 0; i < _materials.size(); ++i)
		{
			ISceneReader::MaterialData mat = scene->getMaterial((int)i);

			_materials[i]._color = mat._color.array();
			_materials[i]._mirrorColor = mat._mirror_color.array();
			_materials[i]._diffuseReflect = mat._diffuseReflect;
			_materials[i]._specularReflect = mat._specular_reflect;
			_materials[i]._emit = mat._emit;
			_materials[i]._indexOfRefraction = mat._ior;
			_materials[i]._transparency = mat._transparency;
			_materials[i]._filterColor = Vector3(mat._transmit_filter, mat._transmit_filter, mat._transmit_filter);
			_materials[i]._specularPower = mat._specular_power;
			_materials[i]._refractionPower = mat._translucency_power;

			Real reflect_sum = _materials[i]._transparency + _materials[i]._specularReflect + _materials[i]._diffuseReflect;
			if(reflect_sum > 1.0f)
			{
				_materials[i]._transparency/= reflect_sum;
				_materials[i]._specularReflect/= reflect_sum;
				_materials[i]._diffuseReflect/= reflect_sum;
			}
		}

		for(size_t i = 0; i < _primitives.size(); ++i)
		{
			scene->getPrimitive((int)i,_primitives[i]._primitive,_primitives[i]._material);

			assert( _primitives[i]._material < (int)_materials.size() );
			
			Vector3 A = _primitives[i]._primitive.point(0);
			Vector3 AB = _primitives[i]._primitive.point(1) - _primitives[i]._primitive.point(0);
			Vector3 AC = _primitives[i]._primitive.point(2) - _primitives[i]._primitive.point(0);

			_primitives[i]._normal = AB.cross(AC).normalized();
		}

		for(size_t i = 0; i < _lights.size(); ++i)
		{
			ISceneReader::LightData lightData = scene->getLight(i);
			_lights[i]._location = lightData._location;
			_lights[i]._color = lightData._color;
		}
		
		_backgroundSize = scene->getBackgroundSize();
		_backgroundData.resize( _backgroundSize.x()* _backgroundSize.y() );

		std::vector<Vector4> inverted_background(_backgroundSize.x()* _backgroundSize.y());

		scene->getBackgroundData( inverted_background.data() );

		for(int iy = 0; iy < _backgroundSize.y(); ++iy)
			for(int ix = 0; ix < _backgroundSize.x(); ++ix)
				_backgroundData[iy * _backgroundSize.x() + ix] =
					inverted_background[ (_backgroundSize.y() - iy - 1) * _backgroundSize.x() + ix];
		
		generateImportanceLookup();

		_view = scene->getViewMatrix();
		_invView = _view.inverse();
	}

	inline ColorArray getBackgroundColor(const Vector3& direction)
	{
		Vector2 spherical = cartesianToSpherical((_invView * Vector4(direction.x(),direction.y(),direction.z(),0.0f)).head<3>());
		return getBackgroundColor(spherical.x(),spherical.y());
	}

	inline Vector2 cartesianToSpherical(const Vector3& direction)
	{
		/*
		//code from ILM library
		Real r = sqrtf(direction.z() * direction.z() + direction.x()*direction.x());
		Real latitude,longitude;
		if( r < fabs(direction.y()) )
			latitude = acosf( r / direction.norm()) * ((direction.y() >= 0.0f) ? (1.0f) : (-1.0f));
		else
			latitude = asinf( direction.y() / direction.norm());

		if(direction.z() == 0.0f && direction.x() == 0.0f)
			longitude = 0.0f;
		else
			longitude = atan2( direction.x(), direction.z() ) + R_PI;

		assert(longitude >= 0.0f);
		assert(latitude >= -R_PI*.5f);
		
		assert(longitude <= R_PI*2.0f);
		assert(latitude <= R_PI*.5f);
		return Vector2(longitude, latitude + R_PI*.5f);*/
		Vector3 normalized = direction / direction.norm();
		Vector2 spherical( atan2(normalized.x() , -normalized.z()), acosf(normalized.y()) );

		if(spherical.x() < 0.0f)
			spherical.x() += R_PI * 2.0f;
		else if(spherical.x() > R_PI * 2.0f)
			spherical.x() -= R_PI * 2.0f;
		
		if(spherical.y() < 0.0f)
			spherical.y() += R_PI;
		else if(spherical.y() > R_PI)
			spherical.y() -= R_PI;

		return spherical;
	}

	inline Vector3 sphericalToCartesian(const Vector2& spherical)
	{
		Real cos_phi,sin_phi;
		Real cos_theta,sin_theta;

		cos_phi = cosf(spherical.x());
		sin_phi = sinf(spherical.x());
		cos_theta = cosf(spherical.y());
		sin_theta = sinf(spherical.y());
		
		return Vector3( sin_phi*sin_theta,cos_theta,-cos_phi*sin_theta);
	}

	inline ColorArray getBackgroundColor(Real phi, Real theta)
	{
		size_t x = phi * (Real)_backgroundSize.x() / (R_PI*2.0f);
		size_t y = theta * (Real)_backgroundSize.y() / (R_PI);

		if(x >= _backgroundSize.x() )
			x = _backgroundSize.x() -1;
		if(y >= _backgroundSize.y() )
			y = _backgroundSize.y() -1;

		return _backgroundData[ x + y*_backgroundSize.x() ].head<3>();
	}

	static inline Vector3 sphericalToCartesian(Real phi,Real theta,const Vector3& x,const Vector3& y,const Vector3& z)
	{
		Real cos_phi,sin_phi;
		Real cos_theta,sin_theta;

		//sincosf(phi,&sin_phi,&cos_phi);
		//sincosf(theta,&sin_theta,&cos_theta);

		cos_phi = cosf(phi);
		sin_phi = sinf(phi);
		cos_theta = cosf(theta);
		sin_theta = sinf(theta);

		Vector3 result =  (cos_phi*sin_theta*x + sin_phi*sin_theta*y + cos_theta*z);

		return result;
	}

	static inline Real pdfAtUniformHemisphere(const Vector3& x,const Vector3& y,const Vector3& z,const Vector3& dir)
	{
		if(z.dot(dir)>0.0f)
			return 1.0f/(2.0f*R_PI);
		else
			return 0.0f;
	}

	static inline Real uniformHemisphere(const Vector2& uniform,const Vector3& x,const Vector3& y,const Vector3& z,Vector3& out)
	{
		
		Real phi = uniform.x() * (Real)R_PI*2.0f;

		Real theta = acosf(uniform.y());
			
		out = sphericalToCartesian(phi,theta,x,y,z);
		return pdfAtUniformHemisphere(x,y,z,out);
	}

	static inline Real pdfAtCosineWeightedHemisphere(const Vector3& x,const Vector3& y,const Vector3& z,const Vector3& dir)
	{		
		if(z.dot(dir)>0.0f)
			return z.dot(dir)/(Real)(R_PI);
		else
			return 0.0f;
	}

	static inline Real cosineWeightedHemisphere(const Vector2& uniform,const Vector3& x,const Vector3& y,const Vector3& z,Vector3& out)
	{
		Real phi = uniform.x() * R_PI*2.0f;
		Real cos_theta = sqrtf(uniform.y());
		Real theta = acosf(cos_theta);

		out = sphericalToCartesian(phi,theta,x,y,z);
		return pdfAtCosineWeightedHemisphere(x,y,z,out);
	}
	

	static inline Real pdfAtCosineWeightedLobe(const Vector3& x,const Vector3& y,const Vector3& z,Real weight,const Vector3& dir)
	{		
		if(z.dot(dir)>0.0f)
			return (weight+2.0f)*powf(dir.dot(z),weight)/(2.0f*R_PI);
		else
			return 0.0f;
	}

	static inline Real cosineWeightedLobe(const Vector2& uniform,const Vector3& x,const Vector3& y,const Vector3& z,Real weight,Vector3& out)
	{
		Real phi = uniform.x() * R_PI*2.0f;
		Real cos_theta = powf(uniform.y(),1.0f/(weight + 1.0f));
		Real theta = acosf(cos_theta);


		out = sphericalToCartesian(phi,theta,x,y,z);
		return pdfAtCosineWeightedLobe(x,y,z,weight,out);
	}
	
	static inline Real pdfAtUniformSphere(const Vector3& x,const Vector3& y,const Vector3& z,const Vector3& dir)
	{		
		return 1.0f/(4.0f*R_PI);
	}
	static inline Real uniformSphere(const Vector2& uniform,const Vector3& x,const Vector3& y,const Vector3& z,Vector3& out)
	{
		
		Real phi = uniform.x() * R_PI*2.0f;

		
		Real theta = 2.0f*asinf(  sqrtf(uniform.y()) );
			
		out = sphericalToCartesian(phi,theta,x,y,z);

		//pdf for uniform over full sphere
		return pdfAtUniformSphere(x,y,z,out);
	}
	
	static inline bool GetSpecularDirection(const Vector3& toIncident,const Vector3& normal,Vector3& out)
	{
		out = (2.0f*toIncident.dot(normal)*normal - toIncident).normalized();
		return true;
	}

	static inline bool GetRefractedDirection(const Vector3& toLight,const Vector3& normal, Real ior,Vector3& out)
	{
		
		Real n;
				
		Real cosTheta1 = normal.dot(toLight);

		Vector3 norm;

		if(cosTheta1 > 0.0f)
		{
			n = 1.0f/ior;
			norm = normal;
		}
		else
		{
			n = ior;
			norm = -normal;
			cosTheta1 = -cosTheta1;
		}
		
		Real cosTheta2sq =  1.0f - n*n*(1.0f - cosTheta1*cosTheta1);
			
		if(cosTheta2sq >= 0.0f)
		{
			Real cosTheta2 = sqrt(cosTheta2sq);
			/*
			if(cosTheta1 >= 0.0f)
				out = (n*cosTheta1* normal - cosTheta2* normal )  - n * toIncident ;
			else
				out = (-n*cosTheta1 - cosTheta2 ) * normal - n * toIncident ;*/

			out = norm*n*cosTheta1 - n*toLight - cosTheta2*norm;
			
			//out = (-2.0f*out.dot(-normal)*normal - out).normalized();
			/*
			Real nDL = normal.dot(toLight);
			Real nDO = normal.dot(out);

			char temp[512];
			sprintf(temp,
				"In: %f/%f/%f - %f\n"
				"Out: %f/%f/%f - %f\n"
				"Cos Theta In: %f\n"
				"Cos Theta Out: %f\n",
				toLight.x(),toLight.y(),toLight.z(),toLight.norm(),
				out.x(),out.y(),out.z(),out.norm(),
				nDL,
				nDO);

			if(nDL > 0.0f)
				if(nDO > 0.0f)
					MessageBoxA(NULL,temp,"ERROR",MB_OK);
			else if(nDL < 0.0f)
				if(nDO < 0.0f)
					MessageBoxA(NULL,temp,"ERROR",MB_OK);*/

			//out.normalize();
			
			return true;
		}
		else
		{
			out = Vector3(0.0f,0.0f,0.0f);
			return false;
		}
	}

	inline ColorArray GetReflectedFactor(const MaterialSettings& material,const PrimitiveData& primitive,const Vector3& toLight,const Vector3& toViewer)
	{
		ColorArray brdf;
		BRDF(material,primitive,toLight,toViewer,brdf);

		/*
		assert(brdf.x() >= 0.0f && brdf.x() <= 1.0f);
		assert(brdf.y() >= 0.0f && brdf.y() <= 1.0f);
		assert(brdf.z() >= 0.0f && brdf.z() <= 1.0f);*/
		return brdf;
	}
	
	inline ColorArray GetReflectedFactorPoint(const MaterialSettings& material,const PrimitiveData& primitive,const Vector3& toLight,const Vector3& toViewer)
	{
		ColorArray brdf;
		BRDFPoint(material,primitive,toLight,toViewer,brdf);
		/*
		assert(brdf.x() >= 0.0f && brdf.x() <= 1.0f);
		assert(brdf.y() >= 0.0f && brdf.y() <= 1.0f);
		assert(brdf.z() >= 0.0f && brdf.z() <= 1.0f);*/
		return brdf;
	}

	inline void BRDF(const MaterialSettings& material,const PrimitiveData& primitive,const Vector3& toLight,const Vector3& toViewer,ColorArray& brdf)
	{
		Real pdf = 0.0f;
		brdf = ColorArray(0.0f,0.0f,0.0f);
		// diffuse
		
		Real cosThetaLight = toLight.dot(primitive._normal);
		Real cosThetaView = toViewer.dot(primitive._normal);
		
		if(cosThetaLight > 0.0f && cosThetaView > 0.0f)
			brdf += (material._color * material._diffuseReflect)*fabs(cosThetaLight)/(float)R_PI;
		
		Vector3 vSpec;

		if( material._specularReflect > 0.0f && GetSpecularDirection(toLight,primitive._normal,vSpec))
		{
			Real n = material._specularPower;
			Real vSpecCos = vSpec.dot(toViewer);
			if(vSpecCos > 0.0f)
				brdf += (material._specularReflect * material._mirrorColor)*(n+1)* powf(vSpecCos,n) / (2.0f*R_PI);
		}

		Vector3 vRefracted;

		if(material._transparency > 0.0f && GetRefractedDirection(toLight,primitive._normal,material._indexOfRefraction,vRefracted) )
		{
			//refraction
			Real n = material._specularPower;	
			Real vRefCos = vRefracted.dot(toViewer);
			if(vRefCos > 0.0f)
				brdf += material._transparency * material._filterColor*(n+1)* powf(vRefCos,n) / (2.0f*R_PI);
		}

	}
	
	inline void BRDFPoint(const MaterialSettings& material,const PrimitiveData& primitive,const Vector3& toLight,const Vector3& toViewer,ColorArray& brdf)
	{
		Real pdf = 0.0f;
		brdf = ColorArray(0.0f,0.0f,0.0f);
		// diffuse
		
		Real cosThetaLight = toLight.dot(primitive._normal);
		Real cosThetaView = toViewer.dot(primitive._normal);
		
		if(cosThetaLight > 0.0f && cosThetaView > 0.0f)
			brdf += (material._color * material._diffuseReflect)*fabs(cosThetaLight);
		
		Vector3 vSpec;

		if( material._specularReflect > 0.0f && GetSpecularDirection(toLight,primitive._normal,vSpec))
		{
			Real n = material._specularPower;
			Real vSpecCos = vSpec.dot(toViewer);
			if(vSpecCos > 0.0f)
				brdf += (material._specularReflect * material._mirrorColor)* powf(vSpecCos,n) ;
		}

		Vector3 vRefracted;

		if(material._transparency > 0.0f && GetRefractedDirection(toLight,primitive._normal,material._indexOfRefraction,vRefracted) )
		{
			//refraction
			Real n = material._specularPower;	
			Real vRefCos = vRefracted.dot(toViewer);
			if(vRefCos > 0.0f)
				brdf += material._transparency * material._filterColor* powf(vRefCos,n);
		}

	}

	inline ColorArray Albedo(const MaterialSettings& material,const PrimitiveData& primitive,const Vector3& toViewer)
	{
		ColorArray diffusePart = material._color * material._diffuseReflect;

		ColorArray specularPart = material._specularReflect * material._mirrorColor;

		ColorArray refractedPart = material._transparency * material._filterColor;

		ColorArray albedo = diffusePart + specularPart + refractedPart;

		albedo.x() = std::max(0.0f,std::min(1.0f,albedo.x()));
		albedo.y() = std::max(0.0f,std::min(1.0f,albedo.y()));
		albedo.z() = std::max(0.0f,std::min(1.0f,albedo.z()));

		return albedo;
	}

	inline Real getImportanceLookupSpherical(Vector2 random, Vector3& dir)
	{
		size_t iy, ix;

		// todo: rewrite from 2n to 2logn

		for(iy = 0; 
			(iy+1) < _backgroundSize.y();
			++iy)
		{
			if(_backgroundImportanceLookupYSlices[iy] >= random.x())
				break;
			else
				random.x() -= _backgroundImportanceLookupYSlices[iy];
		}

		Real pdf = _backgroundImportanceLookupYSlices[iy];

		for(ix = 0; 
			(ix+1) < _backgroundSize.x();
			++ix)
		{
			if(_backgroundImportanceLookupXSlices[iy * _backgroundSize.x() + ix] >= random.y())
				break;
			else
				random.y() -= _backgroundImportanceLookupXSlices[iy * _backgroundSize.x() + ix];
		}
		
		pdf = _backgroundData[iy * _backgroundSize.x() + ix].head<3>().sum()/3.0f / _backgroundIntegral;

		Real phi = (Real)ix / (Real)_backgroundSize.x() * 2.0f * R_PI;
		Real theta = (Real)iy / (Real)_backgroundSize.y() * R_PI;

		Vector3 direction = sphericalToCartesian(Vector2(phi,theta));
		dir = (_view * Vector4(direction.x(),direction.y(),direction.z(),0.0f)).head<3>();

		return pdf;
	}

	inline Real getBackgroundEmissive()
	{
		return _backgroundEmissive;
	}

	void generateImportanceLookup()
	{
		_backgroundImportanceLookupXSlices.resize(_backgroundSize.x() * _backgroundSize.y());
		_backgroundImportanceLookupYSlices.resize(_backgroundSize.y());
		float totalImportance = 0.0f;
		_backgroundEmissive = 0.0f; 
		_backgroundIntegral = 0.0f;
		for(int iy = 0; iy < _backgroundSize.y(); ++iy)
		{
			float areaSlice = 2.0f * R_PI * (cosf( (float)(iy)/(float)_backgroundSize.y() * M_PI ) - cosf( (float)(iy+1)/(float)_backgroundSize.y() * M_PI ));
			float sliceImportance = 0.0f;

			for(int ix = 0; ix < _backgroundSize.x(); ++ix)
				sliceImportance += _backgroundImportanceLookupXSlices[iy * _backgroundSize.x() + ix] =
					_backgroundData[iy * _backgroundSize.x() + ix].head<3>().sum()/3.0f;
			
			Real sum_check = 0.0f;

			if(sliceImportance > 0.0f)
				for(int ix = 0; ix < _backgroundSize.x(); ++ix)
				{
					_backgroundImportanceLookupXSlices[iy * _backgroundSize.x() + ix] /= sliceImportance;
					sum_check += _backgroundImportanceLookupXSlices[iy * _backgroundSize.x() + ix];
				}

			_backgroundEmissive += sliceImportance;
			_backgroundIntegral += sliceImportance * areaSlice / (Real)_backgroundSize.x();
			totalImportance += _backgroundImportanceLookupYSlices[iy] = sliceImportance * areaSlice;
		}
		Real sum_check = 0.0f;
		for(int iy = 0; iy < _backgroundSize.y(); ++iy)
		{
			_backgroundImportanceLookupYSlices[iy] /= totalImportance;
			sum_check += _backgroundImportanceLookupYSlices[iy];
		}
		_backgroundEmissive/= (float)(_backgroundSize.x() * _backgroundSize.y());

		std::cout << "Background Integral: " << _backgroundIntegral << std::endl;
	}

	Real getBackgroundPdf(const Vector3& x,const Vector3& y,const Vector3& z,const Vector3& dir)
	{
		
		Real pdf = getBackgroundColor(dir).sum()/3.0f/ _backgroundIntegral;
		//std::cout << "Direction: " << dir << " pdf" << pdf  << " normalized: " << (pdf*R_PI*4.0f) << std::endl;
		return pdf;
	}

	std::vector<PrimitiveData>		_primitives;
	std::vector<MaterialSettings>	_materials;
	std::vector<Light>				_lights;

	Matrix4							_invView;
	Matrix4							_view;

	Real							_backgroundEmissive;
	Real							_backgroundIntegral;
	Vector2u						_backgroundSize;
	std::vector<Vector4>			_backgroundData;
	std::vector<Real>				_backgroundImportanceLookupYSlices;
	std::vector<Real>				_backgroundImportanceLookupXSlices;
};

}
#endif