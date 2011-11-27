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
#include "SIMDType.h"

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


	namespace detail
	{
		template<bool _bA,bool _bB,class _A,class _B> struct _findCommonType
		{
			typedef void type;
		};
		
		template<class _A,class _B> struct _findCommonType<false,true,_A,_B>
		{
			typedef _A type;
		};
		
		template<class _A,class _B> struct _findCommonType<true,false,_A,_B>
		{
			typedef _B type;
		};
		
		template<class _A,class _B> struct _findCommonType<true,true,_A,_B>
		{
			typedef _A type;
		};
	};

	template<class _A,class _B> struct findCommonType
	{
		typedef typename detail::_findCommonType<std::is_convertible<_A,_B>::value,std::is_convertible<_B,_A>::value,_A,_B>::type type;
	};
	/*
	inline bool Intersect(const RayAccel& ray,const TriAccel& acc,Real& t,Vector2& barycentric)
	{
		__declspec(align(64)) static const unsigned int modulo[] = {0,1,2,0,1};
		static const Real EPSILON = 0.00001f;

		const int ku = modulo[acc._k+1];
		const int kv = modulo[acc._k+2];
		// don’t prefetch here, assume data has already been prefetched
		// start high-latency division as early as possible
		const Real nd = 1.f/(ray.direction()[acc._k]
		+ acc._n_u * ray.direction()[ku] + acc._n_v * ray.direction()[kv]);
		const Real f = (acc._n_d - ray.origin()[acc._k]
		- acc._n_u * ray.origin()[ku] - acc._n_v * ray.origin()[kv]) * nd;
		// check for valid distance.

		if ( t < f || f < EPSILON ) return false;

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
	}*/
	/*
	template<int _Width> inline int Intersect(
		const RayAccelBase<_Width>& ray,
		const TriAccelBase<_Width>& tri,
		SimdType<float,_Width>& t,
		typename Vector2v<_Width>::type& barycentric,
		SimdType<int,_Width>& triId)
	{
		typedef typename Vector2v<_Width>::type Vector2_T;
		typedef typename Vector3v<_Width>::type Vector3_T;
		typedef SimdType<float,_Width> Scalar_T;
		typedef typename SimdType<float,_Width>::Boolean Boolean;

		static const Scalar_T infinity(std::numeric_limits<float>::infinity());

		//calculate ray plane intersection
		Scalar_T det = ray.direction().dot( tri._n_v );

		Scalar_T inv_det = det.Reciprocal();

		//Boolean valid = (det > Scalar_T::Zero());

		Scalar_T t_ = -tri._n_d - (ray.origin().dot( tri._n_v) );
		Vector3_T P_ = det*ray.origin() + t_ * ray.direction();

		// calculate impact values
		t_ *= inv_det;

		Boolean valid = (t_ >= Scalar_T::Epsilon()) & (t_ < t);

		Scalar_T u_ = (P_.dot( tri._u_v ) + det * tri._u_d)*inv_det;

		valid &= (u_ >= Scalar_T::Zero());

		Scalar_T v_ = (P_.dot( tri._v_v ) + det * tri._v_d)*inv_det;

		valid &= ( (u_ + v_) <= Scalar_T::One() ) & ( v_ >= Scalar_T::Zero() );

		// check if we found any results
		if(valid.mask())
		{
			t.ConditionalAssign(valid,t_,t);
			t = t.Min();
			Scalar_T::Boolean resultMask = (t == t_);
			triId = tri._user & resultMask;
			barycentric.x() = u_ & resultMask;
			barycentric.y() =v_ & resultMask;
		}

		return valid.mask();
	}*/
	/*
	template<class _RayClass,class _TriangleClass> inline void IntersectTriangleCore(
		const _RayClass& ray,
		const _TriangleClass& tri,
		typename _RayClass::Scalar_T& t_,
		Eigen::Matrix<typename _RayClass::Scalar_T,2,1>& uv,
		typename _RayClass::Scalar_T::Boolean& valid)
	{
		typedef Eigen::Matrix<typename _RayClass::Scalar_T,2,1> Vector2_T;
		typedef Eigen::Matrix<typename _RayClass::Scalar_T,3,1> Vector3_T;
		typedef _RayClass::Scalar_T Scalar_T;

		//calculate ray plane intersection
		Scalar_T inv_det = ray.direction().dot( tri._n_v ).ReciprocalHighPrecision();
		t_ = -tri._n_d - (ray.origin().dot( tri._n_v) );
		t_ *= inv_det;
		Vector3_T P_ = ray.origin() + t_ * ray.direction();

		// calculate impact values
		uv.x() = P_.dot( tri._u_v ) + tri._u_d;
		uv.y() = P_.dot( tri._v_v ) + tri._v_d;

		// check if we found any results

		//Boolean d_valid = (det > Scalar_T::Zero());
		valid = ( (uv.x() + uv.y()) <= Scalar_T::One() ) & ( uv.x() >= Scalar_T::Zero() ) & ( uv.y() >= Scalar_T::Zero() );
	}
	/*
	template<int _Width> inline int Intersect(
		const RayAccelBase<_Width>& ray,
		const TriAccelBase<_Width>& tri,
		SimdType<float,_Width>& t,
		typename Vector2v<_Width>::type& barycentric,
		SimdType<int,_Width>& triId)
	{
		typedef typename Vector2v<_Width>::type Vector2_T;
		typedef SimdType<float,_Width> Scalar_T;
		typedef typename SimdType<float,_Width>::Boolean Boolean;

		Vector2_T uv;
		Scalar_T t_;

		Boolean valid;
		IntersectTriangleCore(ray,tri,t_,uv,valid);

		valid &= (t_ >= Scalar_T::Epsilon()) & (t_ < t);

		// update result
		if(valid.mask())
		{
			t.ConditionalAssign(valid,t_,t);
			t = t.Min();
			Scalar_T::Boolean resultMask = (t == t_);
			triId = tri._user & resultMask;
			barycentric.x() = uv.x() & resultMask;
			barycentric.y() = uv.y() & resultMask;
		}

		return valid.mask();
	}

	template<int _Width> inline int Intersect(
		const RayAccelBase<_Width>& ray,
		const TriAccelBase<_Width>& tri,
		const SimdType<float,_Width>& t)
	{
		typedef typename Vector2v<_Width>::type Vector2_T;
		typedef SimdType<float,_Width> Scalar_T;
		typedef typename SimdType<float,_Width>::Boolean Boolean;

		Vector2_T uv;
		Scalar_T t_;
		
		Boolean valid;
		IntersectTriangleCore(ray,tri,t_,uv,valid);

		valid &= (t_ >= Scalar_T::Epsilon()) & (t_ < t);
		
		return valid.mask();
	}

	template<int _Width,int _ArraySize> inline void IntersectTriangleArrayCore(
		const RayAccelBase<_Width>& ray,
		const std::array<TriAccelBase<_Width>,_ArraySize>& tri,
		std::array<SimdType<float,_Width>,_ArraySize>& t_,
		std::array<typename Vector2v<_Width>::type,_ArraySize>& uv,
		std::array<typename SimdType<float,_Width>::Boolean,_ArraySize>& valid)
	{
		typedef std::array<typename Vector2v<_Width>::type,_ArraySize> Vector2_T;
		typedef std::array<typename Vector3v<_Width>::type,_ArraySize> Vector3_T;
		typedef std::array<SimdType<float,_Width>,_ArraySize> Scalar_T;

		//calculate ray plane intersection
		Scalar_T inv_det;
		Vector3_T P_;

		for(int i = 0; i < _ArraySize; ++i)
			inv_det[i] = ray.direction().dot( tri[i]._n_v );
		
		for(int i = 0; i < _ArraySize; ++i)
			inv_det[i] = inv_det[i].ReciprocalHighPrecision();

		for(int i = 0; i < _ArraySize; ++i)
			t_[i] = -tri[i]._n_d - (ray.origin().dot( tri[i]._n_v) );

		for(int i = 0; i < _ArraySize; ++i)
			t_[i] *= inv_det[i];
		
		for(int i = 0; i < _ArraySize; ++i)
			P_[i] = ray.origin() + t_ [i]* ray.direction();

		// calculate impact values
		for(int i = 0; i < _ArraySize; ++i)
			uv[i].x() = P_[i].dot( tri[i]._u_v ) + tri[i]._u_d;

		for(int i = 0; i < _ArraySize; ++i)
			uv[i].y() = P_[i].dot( tri[i]._v_v ) + tri[i]._v_d;

		// check if we found any results

		//Boolean d_valid = (det > Scalar_T::Zero());
		for(int i = 0; i < _ArraySize; ++i)
			valid[i] = ( (uv[i].x() + uv[i].y()) <= SimdType<float,_Width>::One() ) & ( uv[i].x() >= SimdType<float,_Width>::Zero() ) & ( uv[i].y() >= SimdType<float,_Width>::Zero() );
	}

	template<int _Width,int _ArraySize> inline bool IntersectArray(
		const RayAccelBase<_Width>& ray,
		const std::array<TriAccelBase<_Width>,_ArraySize>& tri,
		SimdType<float,_Width>& t,
		typename Vector2v<_Width>::type& barycentric,
		SimdType<int,_Width>& triId)
	{
		typedef typename Vector2v<_Width>::type Vector2_T;
		typedef SimdType<float,_Width> Scalar_T;
		typedef typename SimdType<float,_Width>::Boolean Boolean;

		std::array<Vector2_T,_ArraySize> uv;
		std::array<Scalar_T,_ArraySize> t_;
		std::array<Boolean,_ArraySize> valid;

		IntersectTriangleArrayCore(ray,tri,t_,uv,valid);
		
		bool result = false;
		
		for(int i = 0; i < _ArraySize; ++i)
		{
			valid[i] &= (t_[i] >= Scalar_T::Epsilon()) & (t_[i] < t[i]);

			if(valid[i].mask())
			{
				t.ConditionalAssign(valid[i],t_[i],t);
				t = t.Min();
				Scalar_T::Boolean resultMask = (t == t_[i]);
				triId = tri[i]._user & resultMask;
				barycentric.x() = uv[i].x() & resultMask;
				barycentric.y() = uv[i].y() & resultMask;
				result = true;
			}
		}

		return result;
	}

	template<int _Width,int _ArraySize> inline bool IntersectArray(
		const RayAccelBase<_Width>& ray,
		const std::array<TriAccelBase<_Width>,_ArraySize>& tri,
		const SimdType<float,_Width>& t)
	{
		typedef typename Vector2v<_Width>::type Vector2_T;
		typedef SimdType<float,_Width> Scalar_T;
		typedef typename SimdType<float,_Width>::Boolean Boolean;
		
		std::array<Vector2_T,_ArraySize> uv;
		std::array<Scalar_T,_ArraySize> t_;
		std::array<Boolean,_ArraySize> valid;

		IntersectTriangleArrayCore(ray,tri,t_,uv,valid);
		
		for(int i = 0; i < _ArraySize; ++i)
		{
			valid[i] &= (t_[i] >= Scalar_T::Epsilon()) & (t_[i] < t[i]);

			if(valid[i].mask())
				return true;
		}
		return false;
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
	/*
	template<int _Width,int _Array> inline typename int IntersectArray(const RayAccelBase<_Width>& ray,const std::array<AABBAccelBase<_Width>,_Array>& aabb, std::array<SimdType<float,_Width>,_Array>& t) 
	{
		typedef typename Vector2v<_Width>::type Vector2_T;
		typedef typename Vector3v<_Width>::type Vector3_T;
		typedef SimdType<float,_Width> Scalar_T;
		typedef typename SimdType<float,_Width>::Boolean Boolean;
		
		// X

		std::array<Scalar_T,_Array> tXmax,tXmin,tYmax,tYmin,tZmax,tZmin;
		std::array<Boolean,_Array> valid;

		for(int i = 0; i < _Array; ++i)
			tXmax[i] = (aabb[i].max().x() - ray.origin().x())* ray.invDirection().x();
		
		for(int i = 0; i < _Array; ++i)
			tXmin[i] = (aabb[i].min().x() - ray.origin().x())* ray.invDirection().x();

		for(int i = 0; i < _Array; ++i)
			Scalar_T::ConditionalSwap( ray.sign(0), tXmax[i], tXmin[i] );
		
		// Y
		
		for(int i = 0; i < _Array; ++i)
			tYmax[i] = (aabb[i].max().y() - ray.origin().y())* ray.invDirection().y();
		for(int i = 0; i < _Array; ++i)
			tYmin[i] = (aabb[i].min().y() - ray.origin().y())* ray.invDirection().y();

		for(int i = 0; i < _Array; ++i)
			Scalar_T::ConditionalSwap( ray.sign(1), tYmax[i], tYmin[i] );
		
		for(int i = 0; i < _Array; ++i)
			valid[i] = (tXmin[i] <= tYmax[i]); 

		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tYmin[i] <= tXmax[i]);
		
		for(int i = 0; i < _Array; ++i)
			tXmin[i] = tXmin[i].Max( tYmin[i] );

		for(int i = 0; i < _Array; ++i)
			tXmax[i] = tXmax[i].Min( tYmax[i] );

		// Z
		
		for(int i = 0; i < _Array; ++i)
			tZmax[i] = (aabb[i].max().z() - ray.origin().z())* ray.invDirection().z();
		
		for(int i = 0; i < _Array; ++i)
			tZmin[i] = (aabb[i].min().z() - ray.origin().z())* ray.invDirection().z();
		
		for(int i = 0; i < _Array; ++i)
			Scalar_T::ConditionalSwap( ray.sign(2), tZmax[i], tZmin[i] );
		
		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tXmin[i] <= tZmax[i]); 
		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tZmin[i] <= tXmax[i]);
		
		for(int i = 0; i < _Array; ++i)
			tXmin[i] = tXmin[i].Max( tZmin[i] );
		for(int i = 0; i < _Array; ++i)
			tXmax[i] = tXmax[i].Min( tZmax[i] );
		
		// check for hit
		
		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tXmin[i] < t[i] );

		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tXmax[i] > Scalar_T::Zero());
		
		for(int i = 0; i < _Array; ++i)
			t[i].ConditionalAssign(valid[i], tXmin[i].Max(Scalar_T::Zero()), t[i]);

		int result = 0;

		for(int i = 0; i < _Array; ++i)
			result |= valid[i].mask() << (i * _Width);
		return result;
	}

	template<int _Width,int _Array> inline typename int IntersectArray(const RayAccelBase<_Width>& ray,const std::array<AABBAccelBase<_Width>,_Array>& aabb, const SimdType<float,_Width>& t) 
	{
		typedef typename Vector2v<_Width>::type Vector2_T;
		typedef typename Vector3v<_Width>::type Vector3_T;
		typedef SimdType<float,_Width> Scalar_T;
		typedef typename SimdType<float,_Width>::Boolean Boolean;
		
		// X

		std::array<Scalar_T,_Array> tXmax,tXmin,tYmax,tYmin,tZmax,tZmin;
		std::array<Boolean,_Array> valid;

		for(int i = 0; i < _Array; ++i)
			tXmax[i] = (aabb[i].max().x() - ray.origin().x())* ray.invDirection().x();
		
		for(int i = 0; i < _Array; ++i)
			tXmin[i] = (aabb[i].min().x() - ray.origin().x())* ray.invDirection().x();

		for(int i = 0; i < _Array; ++i)
			Scalar_T::ConditionalSwap( ray.sign(0), tXmax[i], tXmin[i] );
		
		// Y
		
		for(int i = 0; i < _Array; ++i)
			tYmax[i] = (aabb[i].max().y() - ray.origin().y())* ray.invDirection().y();
		for(int i = 0; i < _Array; ++i)
			tYmin[i] = (aabb[i].min().y() - ray.origin().y())* ray.invDirection().y();

		for(int i = 0; i < _Array; ++i)
			Scalar_T::ConditionalSwap( ray.sign(1), tYmax[i], tYmin[i] );
		
		for(int i = 0; i < _Array; ++i)
			valid[i] = (tXmin[i] <= tYmax[i]); 

		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tYmin[i] <= tXmax[i]);
		
		for(int i = 0; i < _Array; ++i)
			tXmin[i] = tXmin[i].Max( tYmin[i] );

		for(int i = 0; i < _Array; ++i)
			tXmax[i] = tXmax[i].Min( tYmax[i] );

		// Z
		
		for(int i = 0; i < _Array; ++i)
			tZmax[i] = (aabb[i].max().z() - ray.origin().z())* ray.invDirection().z();
		
		for(int i = 0; i < _Array; ++i)
			tZmin[i] = (aabb[i].min().z() - ray.origin().z())* ray.invDirection().z();
		
		for(int i = 0; i < _Array; ++i)
			Scalar_T::ConditionalSwap( ray.sign(2), tZmax[i], tZmin[i] );
		
		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tXmin[i] <= tZmax[i]); 
		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tZmin[i] <= tXmax[i]);
		
		for(int i = 0; i < _Array; ++i)
			tXmin[i] = tXmin[i].Max( tZmin[i] );
		for(int i = 0; i < _Array; ++i)
			tXmax[i] = tXmax[i].Min( tZmax[i] );
		
		// check for hit
		
		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tXmin[i] < t );

		for(int i = 0; i < _Array; ++i)
			valid[i] &= (tXmax[i] > Scalar_T::Zero());

		int result = 0;

		for(int i = 0; i < _Array; ++i)
			result |= valid[i].mask() << (i * _Width);
		return result;
	}

	template<int _XSign,int _YSign,int _ZSign,int _Width,int _Array> inline typename int IntersectArraySplit(const RayAccelBaseSlope<_Width>& ray,const std::array<AABBAccelBase<_Width>,_Array>& aabb, const SimdType<float,_Width>& t) 
	{
		typedef typename Vector2v<_Width>::type Vector2_T;
		typedef typename Vector3v<_Width>::type Vector3_T;
		typedef SimdType<float,_Width> Scalar_T;
		typedef typename SimdType<float,_Width>::Boolean Boolean;

		std::array<Vector3_T,_Array> min,max;
		std::array<Boolean,_Array> valid;
		
		if(_XSign > 0)
		{
			for(int i = 0; i < _Array; ++i)
				valid[i] = ray.origin().x() >= aabb[i].min().x();
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_xy * aabb[i].max().x() >= aabb[i].min().y() + ray._c_xy;
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_xz * aabb[i].max().x() >= aabb[i].min().z() + ray._c_xz;
		}
		else if(_XSign == 0)
		{
			for(int i = 0; i < _Array; ++i)
				valid[i] = ray.origin().x() <= aabb[i].max().x() & ray.origin().x() >= aabb[i].min().x();
		}
		else
		{
			for(int i = 0; i < _Array; ++i)
				valid[i] = ray.origin().x() <= aabb[i].max().x();
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_xy * aabb[i].min().x() <= aabb[i].max().y() + ray._c_xy;
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_xz * aabb[i].min().x() <= aabb[i].max().z() + ray._c_xz;
		}
		
		if(_YSign > 0)
		{
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray.origin().y() >= aabb[i].min().y();
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_yx * aabb[i].max().y() >= aabb[i].min().x() + ray._c_yx;
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_yz * aabb[i].max().y() >= aabb[i].min().z() + ray._c_yz;
		}
		else if(_YSign == 0)
			for(int i = 0; i < _Array; ++i)
				valid[i] = ray.origin().y() <= aabb[i].max().y() & ray.origin().y() >= aabb[i].min().y();
		else
		{
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray.origin().y() <= aabb[i].max().y();
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_yx * aabb[i].min().y() <= aabb[i].max().x() + ray._c_yx;
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_yz * aabb[i].min().y() <= aabb[i].max().z() + ray._c_yz;
		}
		
		if(_ZSign > 0)
		{
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray.origin().z() >= aabb[i].min().z();
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_zx * aabb[i].max().z() >= aabb[i].min().x() + ray._c_zx;
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_zy * aabb[i].max().z() >= aabb[i].min().y() + ray._c_zy;
		}
		else if(_ZSign == 0)
			for(int i = 0; i < _Array; ++i)
				valid[i] = ray.origin().z() <= aabb[i].max().z() & ray.origin().z() >= aabb[i].min().z();
		else
		{
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray.origin().z() <= aabb[i].max().z();
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_zx * aabb[i].min().z() <= aabb[i].max().x() + ray._c_zx;
			for(int i = 0; i < _Array; ++i)
				valid[i] &= ray._s_zy * aabb[i].min().z() <= aabb[i].max().y() + ray._c_zy;
		}
		



		int result = 0;

		for(int i = 0; i < _Array; ++i)
			result |= valid[i].mask() << (i * _Width);

		return result;
	}

	template<int _Width> inline typename int Intersect(const RayAccelBase<_Width>& ray,const AABBAccelBase<_Width>& aabb, SimdType<float,_Width>& t) 
	{
		typedef typename Vector2v<_Width>::type Vector2_T;
		typedef typename Vector3v<_Width>::type Vector3_T;
		typedef SimdType<float,_Width> Scalar_T;
		typedef typename SimdType<float,_Width>::Boolean Boolean;
		
	}*/
}

#endif
