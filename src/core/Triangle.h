/********************************************************/
/* FILE: Triangle.h                                   */
/* DESCRIPTION: Raytrace Triangle class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include <RaytraceCommon.h>
#include <boost/mpl/vector.hpp>
#include "TemplateOptions.h"
#include "ArrayAdapter.h"
#include <array>
#include "SIMDType.h"
#include "IntersectorBase.h"

#ifndef Raytrace_TRIANGLE_H_INCLUDED
#define Raytrace_TRIANGLE_H_INCLUDED

namespace Raytrace {
	
	struct Tag_TriangleBaseMode;
	struct Tag_TriangleScalarType;
	struct Tag_TriangleDimensions;
	struct Tag_TriangleUserDataType;
	
	struct TriangleBaseModePoints;
	struct TriangleBaseModeBarycentricPlane;
	template<class _Type> struct TriangleScalarType;
	template<int _Dimension> struct TriangleDimensions;
	template<class _Type> struct TriangleUserDataType;
	
	// Triangle Base Mode

	struct Tag_TriangleBaseMode
	{
		typedef TriangleBaseModePoints DefaultValue;
	};
	
	struct TriangleBaseModePoints
	{
		typedef Tag_TriangleBaseMode Tag;
		static const bool optionsDefined = true;
	};

	struct TriangleBaseModeBarycentricPlane
	{
		typedef Tag_TriangleBaseMode Tag;
		static const bool optionsDefined = true;
	};
	
	// Triangle Base Mode

	struct Tag_TriangleUserDataType
	{
		typedef TriangleUserDataType<void> DefaultValue;
	};
	
	template<class _Type> struct TriangleUserDataType
	{
		typedef Tag_TriangleUserDataType Tag;
		static const bool optionsDefined = true;
		typedef _Type type;
	};

	// Triangle Scalar Type

	struct Tag_TriangleScalarType
	{
		typedef TriangleScalarType<f32> DefaultValue;
	};
	
	template<class _Type> struct TriangleScalarType
	{
		typedef Tag_TriangleScalarType Tag;
		static const bool optionsDefined = true;
		typedef _Type type;
	};

	// Triangle Dimension Count

	struct Tag_TriangleDimensions
	{
		typedef TriangleDimensions<3> DefaultValue;
	};
	template<int _Dimension> struct TriangleDimensions
	{
		typedef Tag_TriangleDimensions Tag;
		static const bool optionsDefined = true;
		static const size_t value = _Dimension;
	};	

	namespace mpl = boost::mpl;

	template<class _Options = mpl::vector<>> struct Triangle;

	namespace detail
	{
		template<int _I> struct FTrianglePointReader
		{ 
			template<class _TriangleBase> struct Reader
			{
				typedef typename _TriangleBase::Vector_T result;

				const typename _TriangleBase::Vector_T& operator()(const _TriangleBase& tri)
				{
					return tri.point(_I);
				}
			};

			template<class _Signature> struct get
			{
				typedef Reader<_Signature> type;
			};
		};
		
		struct FTriangleUserDataReader
		{ 
			template<class _TriangleBase> struct Reader
			{
				typedef typename _TriangleBase::UserData result;

				const typename _TriangleBase::UserData& operator()(const _TriangleBase& tri)
				{
					return tri.user();
				}
			};

			template<class _Signature> struct get
			{
				typedef Reader<_Signature> type;
			};
		};

		// Signature
		template<class _Scalar,class _Dimension,class _BaseMode> struct TriangleBase;

		template<class _Scalar,int _Dimension> struct TriangleBase<TriangleScalarType<_Scalar>,TriangleDimensions<_Dimension>,TriangleBaseModePoints>
		{
			typedef Triangle<>	Minimum;
			typedef PrimitiveClassTriangle PrimitiveClass;
			typedef Eigen::Matrix<_Scalar,_Dimension,1> Vector_T;
			typedef Eigen::Matrix<_Scalar,2,1> Vector2_T;
			typedef Vector2_T RelativeLocation;
			typedef _Scalar Element;
			typedef Element Scalar_T;
			typedef TriangleBaseModePoints TriangleBaseMode;
			static const int Dimensions = _Dimension;
			typedef TriangleBase<TriangleScalarType<_Scalar>,TriangleDimensions<_Dimension>,TriangleBaseModePoints> ThisType;

			template<class _Options> struct adapt
			{
				typedef Triangle<_Options> type;
			};
		
			inline TriangleBase()
			{
			}

			template<class _BaseTriangle> inline TriangleBase(const _BaseTriangle& tri)
			{
				_points[0] = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<_BaseTriangle,void,0,FTrianglePointReader<0>>(tri));
				_points[1] = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<_BaseTriangle,void,0,FTrianglePointReader<1>>(tri));
				_points[2] = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<_BaseTriangle,void,0,FTrianglePointReader<2>>(tri));
			}

			inline const Vector_T& point(size_t i) const
			{
				return _points[i];
			}

			inline void setPoint(size_t i,const Vector_T& v)
			{
				_points[i] = v;
			}

			ALIGN_SIMD std::array<Vector_T,3>	_points;
		} ;
		
		template<class _Scalar,int _Dimension> struct TriangleBase<TriangleScalarType<_Scalar>,TriangleDimensions<_Dimension>,TriangleBaseModeBarycentricPlane>
		{
			typedef Triangle<>	Minimum;
			typedef PrimitiveClassTriangle PrimitiveClass;
			typedef Eigen::Matrix<_Scalar,_Dimension,1> Vector_T;
			typedef Eigen::Matrix<_Scalar,2,1> Vector2_T;
			typedef Vector2_T RelativeLocation;
			typedef _Scalar Element;
			typedef Element Scalar_T;
			typedef TriangleBaseModeBarycentricPlane TriangleBaseMode;
			static const int Dimensions = _Dimension;
			typedef TriangleBase<TriangleScalarType<_Scalar>,TriangleDimensions<_Dimension>,TriangleBaseModeBarycentricPlane> ThisType;

			template<class _Options> struct adapt
			{
				typedef Triangle<_Options> type;
			};
		
			inline TriangleBase()
			{
			}
			
			template<class _Scalar2,int _Dimension2> inline TriangleBase(const TriangleBase<TriangleScalarType<_Scalar2>,TriangleDimensions<_Dimension2>,TriangleBaseModeBarycentricPlane>& triBase)
			{
				static_assert(_Dimension2 == Dimensions, "Triangle Dimensions don't match!");

				_n_v = triBase._n_v;
				_n_d = triBase._n_d;
				_u_v = triBase._u_v;
				_u_d = triBase._u_d;
				_v_v = triBase._v_v;
				_v_d = triBase._v_d;
			}

			template<class _Scalar2,int _Dimension2> inline TriangleBase(const  TriangleBase<TriangleScalarType<_Scalar2>,TriangleDimensions<_Dimension2>,TriangleBaseModePoints>& triBase)
			{
				static_assert(_Dimension2 == Dimensions, "Triangle Dimensions don't match!");
				const Vector_T& p0 = triBase.point(0);
				const Vector_T& p1 = triBase.point(1);
				const Vector_T& p2 = triBase.point(2);
				
				const Vector_T vAC(p2-p0);
				const Vector_T vAB(p1-p0);

				_n_v = vAB.cross(vAC);
			
				const Scalar_T n_len = _n_v.dot(_n_v).ReciprocalHighPrecision();


				_u_v = vAC.cross(_n_v) * n_len;
				_u_d = -_u_v.dot(p0);
			
				_v_v = _n_v.cross(vAB) * n_len;
				_v_d = -_v_v.dot(p0);

				//_n_v *= n_len.Sqrt();
				_n_d = -_n_v.dot(p0);
			}
			
			template<class _Array,class _Element,int _Size,class _Reader,class _Modifier,class _Adapter> 
			inline TriangleBase(const ConstArrayWrapper<_Array,_Element,_Size,_Reader,_Modifier,_Adapter>& arrayBase)
			{
				const Vector_T p0 = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<ConstArrayWrapper<_Array,_Element,_Size,_Reader,_Modifier,_Adapter>,void,0,FTrianglePointReader<0>>(arrayBase));
				const Vector_T p1 = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<ConstArrayWrapper<_Array,_Element,_Size,_Reader,_Modifier,_Adapter>,void,0,FTrianglePointReader<1>>(arrayBase));
				const Vector_T p2 = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<ConstArrayWrapper<_Array,_Element,_Size,_Reader,_Modifier,_Adapter>,void,0,FTrianglePointReader<2>>(arrayBase));
				
				const Vector_T vAC(p2-p0);
				const Vector_T vAB(p1-p0);

				_n_v = vAB.cross(vAC);
			
				const Scalar_T n_len = _n_v.dot(_n_v).ReciprocalHighPrecision();


				_u_v = vAC.cross(_n_v) * n_len;
				_u_d = -_u_v.dot(p0);
			
				_v_v = _n_v.cross(vAB) * n_len;
				_v_d = -_v_v.dot(p0);

				//_n_v *= n_len.Sqrt();
				_n_d = -_n_v.dot(p0);
			}

			ALIGN_SIMD Vector_T _n_v;
			Scalar_T _n_d;

			Vector_T _u_v;
			Scalar_T _u_d;

			Vector_T _v_v;
			Scalar_T _v_d;
		} ;

		// Signature
		template<class _Options,class _UserDataType> struct TriangleUserData ;
		
		template<class _Options> struct TriangleUserData<_Options,TriangleUserDataType<void>>
			: public TriangleBase<
				typename getOptionByTag<_Options,Tag_TriangleScalarType>::type,
				typename getOptionByTag<_Options,Tag_TriangleDimensions>::type,
				typename getOptionByTag<_Options,Tag_TriangleBaseMode>::type
			>
		{
			typedef TriangleBase<
				typename getOptionByTag<_Options,Tag_TriangleScalarType>::type,
				typename getOptionByTag<_Options,Tag_TriangleDimensions>::type,
				typename getOptionByTag<_Options,Tag_TriangleBaseMode>::type
			> Base;
			typedef TriangleUserData<_Options,TriangleUserDataType<void>> ThisType;
			typedef TriangleUserDataType<void> UserDataType;
			typedef size_t UserData;
			
			template<class _Options,class _TriangleUserType> inline TriangleUserData(const TriangleUserData<_Options,_TriangleUserType>& tri) : Base(tri)
			{
			}
			
			template<class _Array,class _Element,int _Size,class _Reader,class _Modifier,class _Adapter> 
			inline TriangleUserData(const ConstArrayWrapper<_Array,_Element,_Size,_Reader,_Modifier,_Adapter>& arrayBase) : Base(triBase)
			{
			}

			inline TriangleUserData() : Base()
			{
			}

			inline const size_t& user() const
			{
				static size_t _user = 0;
				return _user;
			}
		};
		
		template<class _Options,class _UserDataType> struct TriangleUserData<_Options,TriangleUserDataType<_UserDataType>>
			: public TriangleBase<
				typename getOptionByTag<_Options,Tag_TriangleScalarType>::type,
				typename getOptionByTag<_Options,Tag_TriangleDimensions>::type,
				typename getOptionByTag<_Options,Tag_TriangleBaseMode>::type
			>
		{
			typedef TriangleBase<
				typename getOptionByTag<_Options,Tag_TriangleScalarType>::type,
				typename getOptionByTag<_Options,Tag_TriangleDimensions>::type,
				typename getOptionByTag<_Options,Tag_TriangleBaseMode>::type
			> Base;
			typedef TriangleUserData<_Options,TriangleUserDataType<_UserDataType>> ThisType;
			typedef TriangleUserDataType<_UserDataType> UserDataType;
			typedef _UserDataType UserData;
			

			//single constructor
			template<class _Options,class _TriangleUserType> inline TriangleUserData(const TriangleUserData<_Options,_TriangleUserType>& tri) : Base(tri)
			{
				_userData = tri.user();
			}
			
			// array constructor
			template<class _Array,class _Element,int _Size,class _Reader,class _Modifier,class _Adapter> 
			inline TriangleUserData(const ConstArrayWrapper<_Array,_Element,_Size,_Reader,_Modifier,_Adapter>& arrayBase) : Base(arrayBase)
			{
				_userData = ConstArrayWrapper<ConstArrayWrapper<_Array,_Element,_Size,_Reader,_Modifier,_Adapter>,void,0,FTriangleUserDataReader >(arrayBase);
			}

			inline TriangleUserData() : Base()
			{
			}

			inline const UserData& user() const
			{
				return _userData;
			}

			inline void setUser(const UserData& data)
			{
				_userData= data;
			}

		private:

			UserData _userData;
		};
	}
	
	template<class _Options> struct Triangle : public detail::TriangleUserData<_Options,typename getOptionByTag<_Options,Tag_TriangleUserDataType>::type >
	{
		typedef detail::TriangleUserData<_Options,typename getOptionByTag<_Options,Tag_TriangleUserDataType>::type > Base;
		typedef Triangle<_Options> ThisType;
		
		inline Triangle() : Base()
		{
		}

		//single constructor
		template<class _Options> inline Triangle(const Triangle<_Options>& tri) : Base(tri)
		{
		}
			
		// array constructor
		template<class _BaseTriangle> inline Triangle(const ConstArrayWrapper<_BaseTriangle>& arrayBase) : Base(arrayBase)
		{
		}

		static ThisType Empty()
		{
			ThisType b;
			b.setPoint(0,Vector_T(0.0f,0.0f,0.0f));
			b.setPoint(1,Vector_T(0.0f,0.0f,0.0f));
			b.setPoint(2,Vector_T(0.0f,0.0f,0.0f));
			return b;
		}

		public:
			EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};
	/*
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
/*	template<int _Width> struct TriAccel
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
	};*/

}

#endif