/********************************************************/
/* FILE: Triangle.h                                   */
/* DESCRIPTION: Raytrace Triangle class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include <RaytraceCommon.h>
#include <array>
#include "SIMDType.h"
#include "IntersectorBase.h"

#ifndef Raytrace_TRIANGLE_H_INCLUDED
#define Raytrace_TRIANGLE_H_INCLUDED

namespace Raytrace {
	
	struct Triangle
	{
	public:
		typedef PrimitiveClassTriangle PrimitiveClass;
		typedef Triangle Minimum;
		typedef i32 IndexType;
		typedef Vector3 Vector_T;
		typedef Vector3i Integer_T;
		typedef f32 Scalar_T;
		
		inline Triangle(const Vector3& p1_,const Vector3& p2_,const Vector3& p3_,int user = -1) : _user(user)
		{
			setPoint(0,p1_);
			setPoint(1,p1_);
			setPoint(2,p1_);
		}

		inline Triangle(const Triangle& other) : _data(other._data),_user(other._user)
		{
		}

		inline Triangle()
		{
		}

		inline ~Triangle()
		{
		}

		inline void setPoint(int i,const Vector3& val)
		{
			_data[i] = val;
		}
		
		inline const Vector3 point(int i) const
		{
			return _data[i];
		}

		static inline Triangle Empty()
		{
			static const Triangle empty = Triangle(Vector3(0.0f,0.0f,0.0f),Vector3(0.0f,0.0f,0.0f),Vector3(0.0f,0.0f,0.0f),-1);
			return empty;
		}

		inline IndexType index() const
		{
			return _user;
		}

		std::array<Vector3,3>					_data;
		IndexType								_user;
	};

	// from http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf
	// http://www.openprocessing.org/visuals/?visualID=14228#
	/*
	template<int _Width> struct TriAccelBase
	{
		typedef typename Vector3v<_Width>::type Vector_T;
		typedef SimdType<float,_Width> Scalar_T;

		inline TriAccel() {}
		inline TriAccel(const Triangle& tri,std::array<int,_Width> user)
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
			
 
			if ( N[_k] == 0.0)
			{
			  println("Degenerate Tri " + A + B + C + N);
			}
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
		Scalar_T _n_u; //!< == normal.u / normal.k
		Scalar_T _n_v; //!< == normal.v / normal.k
		Scalar_T _n_d; //!< constant of plane equation
		// second 16 byte half cache line
		// line equation for line ac
		Scalar_T _b_nu;
		Scalar_T _b_nv;
		Scalar_T _b_d;
		// third 16 byte half cache line
		// line equation for line ab
		Scalar_T _c_nu;
		Scalar_T _c_nv;
		Scalar_T _c_d;
		// extra
		std::array<int,_Width> _k;
		std::array<int,_Width> _user;
	};*/
	
	template<int _Width> struct TriAccel
	{
		typedef PrimitiveClassTriangle PrimitiveClass;
		typedef Triangle Minimum;
		typedef typename Vector3v<_Width>::type Vector_T;
		typedef SimdType<int,_Width> Integer_T;
		typedef SimdType<float,_Width> Scalar_T;
		static const int Width = _Width;
		typedef Integer_T IndexType;
		
		inline TriAccel() {}
		inline TriAccel(const std::array<Triangle,_Width>& tri)
		{
			std::array<float,_Width> A_x,A_y,A_z;
			std::array<float,_Width> B_x,B_y,B_z;
			std::array<float,_Width> C_x,C_y,C_z;

			for(int i = 0; i< _Width; ++i)
			{
				A_x[i] = tri[i].point(0).x(); A_y[i] = tri[i].point(0).y(); A_z[i] = tri[i].point(0).z();
				B_x[i] = tri[i].point(1).x(); B_y[i] = tri[i].point(1).y(); B_z[i] = tri[i].point(1).z();
				C_x[i] = tri[i].point(2).x(); C_y[i] = tri[i].point(2).y(); C_z[i] = tri[i].point(2).z();
				_user[i] = tri[i]._user;
			}

			const Vector_T	vA(A_x,A_y,A_z),
							vB(B_x,B_y,B_z),
							vC(C_x,C_y,C_z);
			const Vector_T vAC(vC-vA);
			const Vector_T vAB(vB-vA);

			_n_v = vAB.cross(vAC);
			
			const Scalar_T n_len = _n_v.dot(_n_v).ReciprocalHighPrecision();


			_u_v = vAC.cross(_n_v) * n_len;
			_u_d = -_u_v.dot(vA);
			
			_v_v = _n_v.cross(vAB) * n_len;
			_v_d = -_v_v.dot(vA);

			//_n_v *= n_len.Sqrt();
			_n_d = -_n_v.dot(vA);
		}

		inline int user(int i) const
		{
			return _user[i];
		}
		
		inline Integer_T index() const
		{
			return _user;
		}

		__declspec(align(64)) Vector_T _n_v;
		Scalar_T _n_d;

		Vector_T _u_v;
		Scalar_T _u_d;

		Vector_T _v_v;
		Scalar_T _v_d;

		Integer_T _user;
	};

}

#endif