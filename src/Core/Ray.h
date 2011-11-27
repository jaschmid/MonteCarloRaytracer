/********************************************************/
/* FILE: Ray.h                                   */
/* DESCRIPTION: Raytrace Ray class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include <RaytraceCommon.h>
#include <array>
#include "SIMDType.h"
#include "IntersectorBase.h"

#ifndef RAYTRACE_RAY_H_INCLUDED
#define RAYTRACE_RAY_H_INCLUDED

namespace Raytrace {

	struct Ray
	{
	public:
		typedef Ray Minimum;
		typedef PrimitiveClassRay PrimitiveClass;
		typedef Vector3 Vector_T;
		typedef Vector3i Integer_T;
		typedef f32 Scalar_T;

		inline Ray(const Vector3& origin_,const Vector3& direction_,float length_) :
			_origin(origin_),
			_direction(direction_),
			_length(length_)
		{
		}

		inline Ray(const Ray& other) : 
			_origin(other._origin),
			_direction(other._direction),
			_length(other._length)
		{
		}

		inline Ray()
		{
		}

		inline ~Ray()
		{
		}
		
		inline const Vector3& origin() const
		{
			return _origin;
		}

		inline const Vector3& direction() const
		{
			return _direction;
		}

		inline Vector3 end() const
		{
			return origin()+direction()*length();
		}

		inline const Real& length() const
		{
			return _length;
		}
		
		inline void setOrigin(const Vector3& origin)
		{
			_origin = origin;
		}

		inline void setDirection(const Vector3& direction)
		{
			_direction = direction;
		}

		inline void setLength(Real length)
		{
			_length = length;
		}

		inline Ray operator +(const Vector3& offset) const
		{
			return Ray(origin() + offset,direction(),length());
		}

		inline Ray& operator +=(const Vector3& offset)
		{
			setOrigin(origin() + offset);
			return *this;
		}
		
		inline Ray operator *(const Real& scale) const
		{
			return Ray(origin(),direction(),length()*scale);
		}

		inline Ray& operator *=(const Real& scale)
		{
			setLength(length() * scale);
			return *this;
		}

		inline Ray operator *(const Matrix4& matrix) const
		{
			Vector4 origin_ext(origin().x(),origin().y(),origin().z(),1.0f);
			Vector4 direction_ext(direction().x(),direction().y(),direction().z(),0.0f);

			origin_ext = matrix*origin_ext;
			direction_ext = matrix*direction_ext;

			Real dir_length = direction_ext.norm();
			direction_ext /= dir_length;

			return Ray(origin_ext.head<3>(),direction_ext.head<3>(),dir_length*_length);
		}

		inline Ray& operator *=(const Matrix4& matrix)
		{
			Vector4 origin_ext(origin().x(),origin().y(),origin().z(),1.0f);
			Vector4 direction_ext(direction().x(),direction().y(),direction().z(),0.0f);

			origin_ext = matrix*origin_ext;
			direction_ext = matrix*direction_ext;

			Real dir_length = direction_ext.norm();
			direction_ext /= dir_length;

			_origin = origin_ext.head<3>();
			_direction = direction_ext.head<3>();
			_length *= dir_length;

			return *this;
		}

		inline const Vector3 operator ()(const Real& t) const
		{
			return origin() + direction()*t;
		}

		Vector3	_origin;
		Vector3 _direction;
		Real	_length;
	};
	
	struct SignModeRuntime {};
	struct SignModePrecompute {};
	template<int _X,int _Y,int _Z> struct SignModeStatic{
		static const int X = _X;
		static const int Y = _Y;
		static const int Z = _Z;
	};

	struct InvDirModeRuntime {};
	struct InvDirModePrecompute {};
	
	struct FRayOriginReader
	{ 
		template<class _RayBase> struct Reader
		{
			inline const typename _RayBase::Vector_T& operator()(const _RayBase& ray)
			{
				return ray.origin();
			}
		};
	};

	struct FRayDirectionReader
	{
		template<class _RayBase> struct Reader
		{
			inline const typename _RayBase::Vector_T& operator()(const _RayBase& ray)
			{
				return ray.direction();
			}
		};
	};

	template<class _Element,int _Dimensions> struct RayBase
	{
		typedef Ray Minimum;
		typedef PrimitiveClassRay PrimitiveClass;
		typedef Eigen::Matrix<_Element,_Dimensions,1> Vector_T;
		typedef _Element Element;
		typedef Element Scalar_T;
		static const int Dimensions = _Dimensions;

		typedef void Base;
		typedef RayBase<Element,Dimensions> ThisType;


		template<class _RayBase> inline RayBase(const _RayBase& ray) 
		{
			for(int i = 0; i < Dimensions; ++i)
			{
				_origin = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<_RayBase,void,0,FRayOriginReader>(ray));
				_direction = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<_RayBase,void,0,FRayDirectionReader>(ray));
			}
		}
		
		inline RayBase()
		{
		}

		inline const Vector_T& origin() const { return _origin; }
		inline const Vector_T& direction() const { return _direction; }

	private:
		Vector_T		_origin;
		Vector_T		_direction;
	};
	
	template<class _Element,int _Dimensions,class _SignMode> struct RaySign : public RayBase<_Element,_Dimensions>
	{
		typedef _SignMode SignMode;
		typedef ThisType Base;
		typedef RaySign<Element,Dimensions,SignMode> ThisType;

		template<class _RayBase> inline RaySign(const _RayBase& ray) : Base(ray)
		{
			static_assert(false,"Unsupported Sign Mode");
		}
		
		inline RaySign() : public Base()
		{
		}
	};

	template<class _Element,int _Dimensions> struct RaySign<_Element,_Dimensions,SignModeRuntime> : public RayBase<_Element,_Dimensions>
	{
		typedef SignModeRuntime SignMode;
		typedef ThisType Base;
		typedef RaySign<Element,Dimensions,SignMode> ThisType;
		
		template<class _RayBase> inline RaySign(const _RayBase& ray) : Base(ray)
		{
		}
		
		inline RaySign() : public Base()
		{
		}

		inline const typename Scalar_T::Boolean& sign(int i) const { return direction()[i] <= Scalar_T::Zero(); }
	};

	template<class _Element,int _Dimensions> struct RaySign<_Element,_Dimensions,SignModePrecompute> : public RayBase<_Element,_Dimensions>
	{
		typedef SignModePrecompute SignMode;
		typedef ThisType Base;
		typedef RaySign<Element,Dimensions,SignMode> ThisType;
		
		template<class _RayBase> inline RaySign(const _RayBase& ray) : Base(ray)
		{
			for(int i = 0; i < Dimensions; ++i)
				_sign[i] = direction()[i] <= Scalar_T::Zero();
		}
		
		inline RaySign(const ThisType& ray) : Base(ray)
		{
			for(int i = 0; i < Dimensions; ++i)
				_sign[i] = ray.sign(i);
		}

		inline RaySign() : public Base()
		{
		}

		inline const typename Scalar_T::Boolean& sign(int i) const { return _sign[i]; }
		
	private:
		std::array<typename Scalar_T::Boolean,3>		_sign;
	};
	
	template<class _Element,int _Dimensions,int _X,int _Y,int _Z> struct RaySign<_Element,_Dimensions,SignModeStatic<_X,_Y,_Z>> : public RayBase<_Element,_Dimensions>
	{
		typedef SignModeStatic<_X,_Y,_Z> SignMode;
		typedef ThisType Base;
		typedef RaySign<Element,Dimensions,SignMode> ThisType;

		template<class _RayBase> inline RaySign(const _RayBase& ray) : Base(ray)
		{
		}
		
		inline static const typename Scalar_T::Boolean& sign(int i) { return Signs[i]; }
		
	private:
		static const std::array<int,3> Signs;
	};

	template<class _Element,int _Dimensions,int _X,int _Y,int _Z> const std::array<int,3> RaySign<_Element,_Dimensions,SignModeStatic<_X,_Y,_Z>>::Signs = { _X, _Y, _Z };

	template<class _Element,int _Dimensions,class _SignMode, class _InvDirMode> struct RayInvDir : public RaySign<_Element,_Dimensions,_SignMode>
	{
		typedef _InvDirMode InvDirMode;
		typedef ThisType Base;
		typedef RayInvDir<Element,Dimensions,SignMode,InvDirMode> ThisType;

		template<class _RayBase> inline RayInvDir(const _RayBase& ray) : Base(ray)
		{
			static_assert(false,"Unsupported InvDir Mode");
		}
	};
	
	template<class _Element,int _Dimensions,class _SignMode> struct RayInvDir<_Element,_Dimensions,_SignMode,InvDirModeRuntime> : public RaySign<_Element,_Dimensions,_SignMode>
	{
		typedef InvDirModeRuntime InvDirMode;
		typedef ThisType Base;
		typedef RayInvDir<Element,Dimensions,SignMode,InvDirMode> ThisType;

		template<class _RayBase> inline RayInvDir(const _RayBase& ray) : Base(ray)
		{
		}
		
		inline const Vector_T invDirection() const 
		{ 
			Vector_T result;
			for(int i = 0; i < Dimensions; ++i)
				result[i] = direction()[i].ReciprocalHighAccuracy();
			return result;
		}
	};

	template<class _Element,int _Dimensions,class _SignMode> struct RayInvDir<_Element,_Dimensions,_SignMode,InvDirModePrecompute> : public RaySign<_Element,_Dimensions,_SignMode>
	{
		typedef InvDirModeRuntime InvDirMode;
		typedef ThisType Base;
		typedef RayInvDir<Element,Dimensions,SignMode,InvDirMode> ThisType;

		template<class _RayBase> inline RayInvDir(const _RayBase& ray) : Base(ray)
		{
			for(int i = 0; i < Dimensions; ++i)
				_invDirection[i] = direction()[i].ReciprocalHighPrecision();
		}
		
		inline RayInvDir(const ThisType& ray) : Base(ray)
		{
			for(int i = 0; i < Dimensions; ++i)
				_invDirection[i] = ray.invDirection()[i];
		}

		inline const Vector_T& invDirection() const { return _invDirection; }
	private:
		Vector_T		_invDirection;
	};

	template<class _Element,int _Dimensions,class _SignMode, class _InvDirMode> struct RayAccel : public RayInvDir<_Element,_Dimensions,_SignMode,_InvDirMode>
	{
		template<class _RayBase> inline RayAccel(const _RayBase& ray) : RayInvDir<_Element,_Dimensions,_SignMode,_InvDirMode>(ray)
		{
		}
		
		inline RayAccel() : RayInvDir<_Element,_Dimensions,_SignMode,_InvDirMode>()
		{
		}
	};

	// A slope ray accel class... effectively useless because it uses way too much bandwidth
	/*
	template<int _Width> struct RayAccelBaseSlope : public RayAccelBase<_Width>
	{
		typedef typename Vector3v<_Width>::type Vector_T;
		typedef SimdType<float,_Width> Scalar_T;
		static const int Width = _Width;
		typedef RayAccelBase<_Width> Base;

		inline RayAccelBaseSlope(const std::array<Ray,_Width>& ray) : Base(ray)
		{
			//slopes
			_s_yx = Base::direction().x() * Base::invDirection().y();
			_s_xy = Base::direction().y() * Base::invDirection().x();
			_s_zy = Base::direction().y() * Base::invDirection().z();
			_s_yz = Base::direction().z() * Base::invDirection().y();
			_s_xz = Base::direction().z() * Base::invDirection().x();
			_s_zx = Base::direction().x() * Base::invDirection().z();

			//precomp
			_c_yx = origin().x() - _s_yx * Base::direction().y();
			_c_xy = origin().y() - _s_xy * Base::direction().x();
			_c_zx = origin().x() - _s_zx * Base::direction().z();
			_c_xz = origin().z() - _s_xz * Base::direction().x();
			_c_zy = origin().y() - _s_zy * Base::direction().z();
			_c_yz = origin().z() - _s_yz * Base::direction().y();

		}

		Scalar_T							_s_yx,_s_xy,_s_zy,_s_yz,_s_xz,_s_zx;
		Scalar_T							_c_yx,_c_xy,_c_zy,_c_yz,_c_xz,_c_zx;
	};*/
	
}

#endif