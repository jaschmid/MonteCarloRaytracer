/********************************************************/
// FILE: MathHelper.h
// DESCRIPTION: MathHelpers for the engine
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

#ifndef RAYTRACE_MATH_HELPER_GUARD
#define RAYTRACE_MATH_HELPER_GUARD

#include <boost/shared_ptr.hpp>
#include <RaytraceCommon.h>
#include "Triangle.h"
#include "Ray.h"
#include "AABB.h"

namespace Raytrace {
	
	static Matrix4 FromLookAt(const Vector3& eye,const Vector3& at,const Vector3& up)
	{
		Vector3 zaxis = (at - eye).normalized();
		Vector3 xaxis = (up.cross(zaxis)).normalized();
		Vector3 yaxis = (zaxis.cross(xaxis));
		Matrix4 result;
		result.row(0) = ::Eigen::Matrix<Real,4,1>(xaxis.x(),xaxis.y(),xaxis.z(),-(xaxis.dot(eye)));
		result.row(1) = ::Eigen::Matrix<Real,4,1>(yaxis.x(),yaxis.y(),yaxis.z(),-(yaxis.dot(eye)));
		result.row(2) = ::Eigen::Matrix<Real,4,1>(zaxis.x(),zaxis.y(),zaxis.z(),-(zaxis.dot(eye)));
		result.row(3) = ::Eigen::Matrix<Real,4,1>(0.0f,0.0f,0.0f,1.0f);
		return result;
	}

	// from http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf
	// http://www.openprocessing.org/visuals/?visualID=14228#

	__declspec(align(16)) struct TriAccel
	{
		inline TriAccel() {}
		inline TriAccel(const Triangle& tri,int user1 = 0,int user2 = 0)
		{// calc normal
			const Vector3& A = tri.point(0);
			Vector3 B = tri.point(2) - tri.point(0);
			Vector3 C = tri.point(1) - tri.point(0);
			Vector3  N = C.cross(B);
 
			if ( abs(N.x()) > abs(N.y()))
			  if (abs(N.x()) > abs(N.z()))
				_k = 0; // X
			  else
				_k=2; // Z
			else
			  if (abs(N.y()) > abs(N.z()))
				_k = 1; // Y
			  else
				_k=2; // Z
			int u = (_k+1) %3;
			int v = (_k+2) % 3;
			/*
 
			if ( N[_k] == 0.0)
			{
			  println("Degenerate Tri " + A + B + C + N);
			}*/
			//assert( abs(N[_k])>0.00000f);
 
			_n_u = N[u]/N[_k]; //!< == normal.u / normal.k
			_n_v = N[v]/N[_k]; //!< == normal.v / normal.k
			_n_d = A[u]*_n_u + A[v]*_n_v + A[_k]; //!< constant of plane equation
 
			// do B
			float bdenom = (B[u]*C[v]-B[v]*C[u]);
			//assert( abs(bdenom)>0.00000f);
			bdenom = 1.f/bdenom;
 
			_b_nu = -B[v]*bdenom;
			_b_nv = B[u]*bdenom;
			_b_d = (B[v]*A[u]-B[u]*A[v])*bdenom;
 
			// do C
			_c_nu = C[v]*bdenom;
			_c_nv = -C[u]*bdenom;
			_c_d = (-C[v]*A[u]+C[u]*A[v])*bdenom;
			
			_user1 = user1;
			_user2 = user2;
		}

		// first 16 byte half cache line
		// plane:
		Real _n_u; //!< == normal.u / normal.k
		Real _n_v; //!< == normal.v / normal.k
		Real _n_d; //!< constant of plane equation
		int _k; // projection dimension
		// second 16 byte half cache line
		// line equation for line ac
		Real _b_nu;
		Real _b_nv;
		Real _b_d;
		int _user1; // pad to next cache line
		// third 16 byte half cache line
		// line equation for line ab
		Real _c_nu;
		Real _c_nv;
		Real _c_d;
		int _user2; // pad to 48 bytes for cache alignment purposes
	};
	
	__declspec(align(16)) struct RayAccel
	{
		RayAccel(const Ray& ray) 
		{
			_origin = ray.origin();
			_direction = ray.direction();
			_length = ray.length();
			_invDirection = Vector3( 1.0f / direction().x(), 1.0f / direction().y(), 1.0f / direction().z());
		}

		const Vector3& origin() const { return _origin; }
		const Vector3& direction() const { return _direction; }
		const Vector3& invDirection() const { return _invDirection; }
		const Real& length() const { return _length; }
		int sign(int i) const { return _invDirection[i] < .0f; }

		Vector3		_origin;
		Real		_length;
		Vector3		_direction;
		Real		_padding;
		Vector3		_invDirection;
		Real		_padding2;
	};

	inline bool Intersect(const RayAccel& ray,const TriAccel& acc,Real& t,Vector2& barycentric)
	{
		__declspec(align(64)) static const unsigned int modulo[] = {0,1,2,0,1};
		static const Real EPSILON = 0.000001f;

		const int ku = modulo[acc._k+1];
		const int kv = modulo[acc._k+2];
		// don’t prefetch here, assume data has already been prefetched
		// start high-latency division as early as possible
		const Real nd = 1.f/(ray.direction()[acc._k]
		+ acc._n_u * ray.direction()[ku] + acc._n_v * ray.direction()[kv]);
		const Real f = (acc._n_d - ray.origin()[acc._k]
		- acc._n_u * ray.origin()[ku] - acc._n_v * ray.origin()[kv]) * nd;
		// check for valid distance.

		if (!(( ray.length() < 0.0f || ray.length() > f ) && f > EPSILON )) return false;

		// compute hitpoint positions on uv plane
		const Real hu = (ray.origin()[ku] + f * ray.direction()[ku]);
		const Real hv = (ray.origin()[kv] + f * ray.direction()[kv]);
		// check first barycentric coordinate
		const Real lambda = (hu * acc._b_nu + hv * acc._b_nv + acc._b_d);
		if (lambda < 0.0f) return false;
		// check second barycentric coordinate
		const Real mue = (hu * acc._c_nu + hv * acc._c_nv + acc._c_d);
		if (mue < 0.0f) return false;
		// check third barycentric coordinate
		if (lambda+mue > 1.0f) return false;
		// have a valid hitpoint here. store it.
		t = f;
		barycentric.x() = lambda;
		barycentric.y() = mue;
		return true;
	}

	inline bool Intersect(const Ray& ray,const Triangle& triangle,Real& t,Vector2& barycentric)
	{
		static const Real EPSILON = 0.000001f;

		Vector3 edge1 = triangle.point(1) - triangle.point(0);
		Vector3 edge2 = triangle.point(2) - triangle.point(0);

		Vector3 pvec = ray.direction().cross( edge2 );

		Real det = edge1.dot( pvec );

		if( std::abs(det) < EPSILON )
			return false;
		
		Vector3 tvec = ray.origin() - triangle.point(0);
		
		Real inv_det = 1.0f / det;

		Vector3 qvec = tvec.cross( edge1 );
		
		barycentric.x() = tvec.dot( pvec ) * inv_det;
		barycentric.y() = qvec.dot( ray.direction() ) * inv_det;

		if(barycentric.y() < 0.0f || barycentric.x() < 0.0f || barycentric.x() + barycentric.y() > 1.0f)
			return false;

		t = edge2.dot(qvec) * inv_det;
		
		if(t>= 0.0f)
			return true;
		else
			return false;
	}
	
	inline bool Intersect(const RayAccel& ray,const AABB& aabb, Real& t) 
	{  
		Real tmin, tmax, tymin, tymax, tzmin, tzmax;

		tmin = (aabb._data[ray.sign(0)].x() - ray.origin().x()) * ray.invDirection().x();
		tmax = (aabb._data[1-ray.sign(0)].x() - ray.origin().x()) * ray.invDirection().x();
		tymin = (aabb._data[ray.sign(1)].y() - ray.origin().y()) * ray.invDirection().y();
		tymax = (aabb._data[1-ray.sign(1)].y() - ray.origin().y()) * ray.invDirection().y();

		if ( (tmin > tymax) || (tymin > tmax) ) 
			return false;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;

		tzmin = (aabb._data[ray.sign(2)].z() - ray.origin().z()) * ray.invDirection().z();
		tzmax = (aabb._data[1-ray.sign(2)].z() - ray.origin().z()) * ray.invDirection().z();

		if ( (tmin > tzmax) || (tzmin > tmax) ) 
			return false;
		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;

		t = tmin;
		return ( (ray.length() <= 0.0f || (tmin < ray.length())) && (tmax > .0) );
	}
}

#endif
