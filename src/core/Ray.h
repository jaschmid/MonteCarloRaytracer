/********************************************************/
/* FILE: Ray.h                                   */
/* DESCRIPTION: Raytrace Ray class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include <RaytraceCommon.h>
#include <boost/mpl/vector.hpp>
#include "TemplateOptions.h"
#include "ArrayAdapter.h"
#include <array>
#include "SIMDType.h"
#include "IntersectorBase.h"
#include "MathHelper.h"

#ifndef RAYTRACE_RAY_H_INCLUDED
#define RAYTRACE_RAY_H_INCLUDED

namespace Raytrace {
	
	struct Tag_RaySignMode;
	struct Tag_RayLengthMode;
	struct Tag_RayInvDirMode;
	struct Tag_RayScalarType;
	struct Tag_RayDimensions;

	struct RaySignModeRuntime;
	struct RaySignModePrecompute;
	template<int _X,int _Y,int _Z> struct RaySignModeStatic;

	struct RayLengthModeStored;
	struct RayLengthModeNone;

	struct RayInvDirModeRuntime;
	struct RayInvDirModePrecompute;

	template<class _Type> struct RayScalarType;
	template<int _Dimension> struct RayDimensions;

	struct Tag_RaySignMode
	{
		typedef RaySignModeRuntime DefaultValue;
	};
	
	struct Tag_RayLengthMode
	{
		typedef RayLengthModeNone DefaultValue;
	};

	struct Tag_RayInvDirMode
	{
		typedef RayInvDirModeRuntime DefaultValue;
	};

	struct Tag_RayScalarType
	{
		typedef RayScalarType<f32> DefaultValue;
	};

	struct Tag_RayDimensions
	{
		typedef RayDimensions<3> DefaultValue;
	};

	
	struct RaySignModeRuntime
	{
		typedef Tag_RaySignMode Tag;
		static const bool optionsDefined = true;
	};
	struct RaySignModePrecompute
	{
		typedef Tag_RaySignMode Tag;
		static const bool optionsDefined = true;
	};
	template<int _X,int _Y,int _Z> struct RaySignModeStatic
	{
		typedef Tag_RaySignMode Tag;
		static const size_t X = _X;
		static const size_t Y = _Y;
		static const size_t Z = _Z;
		static const bool optionsDefined = true;
	};

	struct RayLengthModeStored
	{
		typedef Tag_RayLengthMode Tag;
		static const bool optionsDefined = true;
	};
	struct RayLengthModeNone
	{
		typedef Tag_RayLengthMode Tag;
		static const bool optionsDefined = true;
	};

	struct RayInvDirModeRuntime
	{
		typedef Tag_RayInvDirMode Tag;
		static const bool optionsDefined = true;
	};
	struct RayInvDirModePrecompute
	{
		typedef Tag_RayInvDirMode Tag;
		static const bool optionsDefined = true;
	};

	template<class _Type> struct RayScalarType
	{
		typedef Tag_RayScalarType Tag;
		static const bool optionsDefined = true;
		typedef _Type type;
	};
	template<int _Dimension> struct RayDimensions
	{
		typedef Tag_RayDimensions Tag;
		static const bool optionsDefined = true;
		static const size_t value = _Dimension;
	};	


	namespace mpl = boost::mpl;
	
	template<class _Options = mpl::vector<>> struct Ray;

	namespace detail
	{

		struct FRayOriginReader
		{ 
			template<class _RayBase> struct Reader
			{
				typedef typename typename _RayBase::Vector_T result;
				inline const typename _RayBase::Vector_T& operator()(const _RayBase& ray)
				{
					return ray.origin();
				}
			};

			template<class _Signature> struct get
			{
				typedef Reader<_Signature> type;
			};
		};

		struct FRayDirectionReader
		{
			template<class _RayBase> struct Reader
			{
				typedef typename typename _RayBase::Vector_T result;
				inline const typename _RayBase::Vector_T& operator()(const _RayBase& ray)
				{
					return ray.direction();
				}
			};

			template<class _Signature> struct get
			{
				typedef Reader<_Signature> type;
			};
		};

		template<class _Element,int _Dimensions> struct RayBase
		{
			typedef Ray<> Minimum;
			typedef PrimitiveClassRay PrimitiveClass;
			typedef Eigen::Matrix<_Element,_Dimensions,1> Vector_T;
			typedef _Element Element;
			typedef Element Scalar_T;
			typedef Scalar_T RelativeLocation;
			static const int Dimensions = _Dimensions;

			typedef void Base;
			typedef RayBase<Element,Dimensions> ThisType;

			template<class _Options> struct adapt
			{
				typedef Ray<_Options> type;
			};


			template<class _RayBase> inline RayBase(const _RayBase& ray) 
			{
				_origin = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<_RayBase,void,0,FRayOriginReader>(ray));
				_direction = ConvertAoSToSoA<Vector_T>()(ConstArrayWrapper<_RayBase,void,0,FRayDirectionReader>(ray));
			}
		
			inline RayBase()
			{
			}

			inline void setDirection(const Vector_T& direction)
			{
				_direction = direction;
			}
			
			inline void setOrigin(const Vector_T& origin)
			{
				_origin = origin;
			}

			inline const Vector_T& origin() const { return _origin; }
			inline const Vector_T& direction() const { return _direction; }

		private:
			ALIGN_SIMD Vector_T		_origin;
			Vector_T		_direction;
		};
	
		// Signature
		template<class _Options,class _SignMode> struct RaySign;

		// RaySignModeRuntime
		template<class _Options> struct RaySign<_Options,RaySignModeRuntime> 
			: public RayBase<typename getOptionByTag<_Options,Tag_RayScalarType>::type::type,getOptionByTag<_Options,Tag_RayDimensions>::type::value>
		{
			typedef RaySignModeRuntime SignMode;
			typedef RayBase<typename getOptionByTag<_Options,Tag_RayScalarType>::type::type,getOptionByTag<_Options,Tag_RayDimensions>::type::value> Base;
			typedef RaySign<_Options,SignMode> ThisType;
		
			template<class _RayBase> inline RaySign(const _RayBase& ray) : Base(ray)
			{
			}
		
			inline RaySign() : Base()
			{
			}

			inline const typename BooleanOf<typename Base::Scalar_T>::type& sign(int i) const { return Base::direction()[i] <= Zero<typename Base::Scalar_T>(); }
		};

		// RaySignModePrecompute
		template<class _Options> struct RaySign<_Options,RaySignModePrecompute>
			: public RayBase<typename getOptionByTag<_Options,Tag_RayScalarType>::type::type,getOptionByTag<_Options,Tag_RayDimensions>::type::value>
		{
			typedef RaySignModePrecompute SignMode;
			typedef RayBase<typename getOptionByTag<_Options,Tag_RayScalarType>::type::type,getOptionByTag<_Options,Tag_RayDimensions>::type::value> Base;
			typedef RaySign<_Options,SignMode> ThisType;
		
			template<class _RayBase> inline RaySign(const _RayBase& ray) : Base(ray)
			{
				for(int i = 0; i < Base::Dimensions; ++i)
					_sign[i] = Base::direction()[i] <= Zero<typename Base::Scalar_T>();
			}

			inline RaySign() : Base()
			{
			}

			inline void setDirection(const Base::Vector_T& direction)
			{
				Base::setDirection(direction);
				
				for(int i = 0; i < Base::Dimensions; ++i)
					_sign[i] = Base::direction()[i] <= Zero<typename Base::Scalar_T>();
			}

			inline const typename BooleanOf<typename Base::Scalar_T>::type& sign(int i) const { return _sign[i]; }
		
		private:
			std::array<typename BooleanOf<typename Base::Scalar_T>::type,3>		_sign;
		};
	
		// RaySignModeStatic
		template<class _Options,int _X,int _Y,int _Z> struct RaySign<_Options,RaySignModeStatic<_X,_Y,_Z>> 
			: public RayBase<typename getOptionByTag<_Options,Tag_RayScalarType>::type::type,getOptionByTag<_Options,Tag_RayDimensions>::type::value>
		{
			typedef RaySignModeStatic<_X,_Y,_Z> SignMode;
			typedef RayBase<typename getOptionByTag<_Options,Tag_RayScalarType>::type::type,getOptionByTag<_Options,Tag_RayDimensions>::type::value> Base;
			typedef RaySign<_Options,SignMode> ThisType;

			template<class _RayBase> inline RaySign(const _RayBase& ray) : Base(ray)
			{
			}

			inline RaySign() : Base()
			{
			}
		
			inline static const typename Base::Scalar_T::Boolean& sign(int i) { return Signs[i]; }
		
		private:
			static const std::array<int,3> Signs;
		};

		template<class _Options,int _X,int _Y,int _Z> const std::array<int,3> RaySign<_Options,RaySignModeStatic<_X,_Y,_Z>>::Signs = { _X, _Y, _Z };

		// Signature
		template<class _Options, class _InvDirMode> struct RayInvDir;
	
		// RayInvDirModeRuntime
		template<class _Options> struct RayInvDir<_Options,RayInvDirModeRuntime> : public RaySign<_Options,typename getOptionByTag<_Options,Tag_RaySignMode>::type>
		{
			typedef RayInvDirModeRuntime InvDirMode;
			typedef RaySign<_Options,typename getOptionByTag<_Options,Tag_RaySignMode>::type> Base;
			typedef RayInvDir<_Options,InvDirMode> ThisType;

			template<class _RayBase> inline RayInvDir(const _RayBase& ray) : Base(ray)
			{
			}

			inline RayInvDir() : Base()
			{
			}
		
			inline const typename Base::Vector_T invDirection() const 
			{ 
				typename Base::Vector_T result;
				for(int i = 0; i < Base::Dimensions; ++i)
					result[i] = Base::direction()[i].ReciprocalHighAccuracy();
				return result;
			}
		};

		// RayInvDirModePrecompute
		template<class _Options> struct RayInvDir<_Options,RayInvDirModePrecompute> : public RaySign<_Options,typename getOptionByTag<_Options,Tag_RaySignMode>::type>
		{
			typedef RayInvDirModePrecompute InvDirMode;
			typedef RaySign<_Options,typename getOptionByTag<_Options,Tag_RaySignMode>::type> Base;
			typedef RayInvDir<_Options,InvDirMode> ThisType;

			template<class _RayBase> inline RayInvDir(const _RayBase& ray) : Base(ray)
			{
				for(int i = 0; i < Base::Dimensions; ++i)
					_invDirection[i] = Base::direction()[i].ReciprocalHighPrecision();
			}

			inline RayInvDir() : Base()
			{
			}

			inline void setDirection(const typename Base::Vector_T& direction)
			{
				Base::setDirection(direction);

				for(int i = 0; i < Base::Dimensions; ++i)
					_invDirection[i] = Base::direction()[i].ReciprocalHighPrecision();
			}

			inline const typename Base::Vector_T& invDirection() const { return _invDirection; }
		private:
			typename Base::Vector_T		_invDirection;
		};

		//Signature
		template<class _Options,class _LengthMode> struct RayLength;
	
		// RayLengthModeStored
		template<class _Options> struct RayLength<_Options,RayLengthModeStored> : public RayInvDir<_Options,typename getOptionByTag<_Options,Tag_RayInvDirMode>::type>
		{
			typedef RayLengthModeStored LengthMode;
			typedef RayInvDir<_Options,typename getOptionByTag<_Options,Tag_RayInvDirMode>::type> Base;
			typedef RayLength<_Options,LengthMode> ThisType;

			template<class _RayBase> inline RayLength(const _RayBase& ray) : Base(ray)
			{
				_length = ray.length();
			}

			inline RayLength() : Base()
			{
			}
		
			inline void setLength(const typename Base::Scalar_T& length)
			{
				_length = length;
			}

			inline const typename Base::Scalar_T& length() const 
			{ 
				return _length;
			}
		private:
			typename Base::Scalar_T	_length;
		};

		// RayLengthModeNone
		template<class _Options> struct RayLength<_Options,RayLengthModeNone> : public RayInvDir<_Options,typename getOptionByTag<_Options,Tag_RayInvDirMode>::type>
		{
			typedef RayLengthModeNone LengthMode;
			typedef RayInvDir<_Options,typename getOptionByTag<_Options,Tag_RayInvDirMode>::type> Base;
			typedef RayLength<_Options,LengthMode> ThisType;

			template<class _RayBase> inline RayLength(const _RayBase& ray) : Base(ray)
			{
			}

			inline RayLength() : Base()
			{
			}

			inline void setLength(const typename Base::Scalar_T& length) const
			{
			}
		
			inline const typename Base::Scalar_T& length() const 
			{ 
				return 1.0f;
			}
		};
	}

	template<class _Options> struct Ray : public detail::RayLength<_Options,typename getOptionByTag<_Options,Tag_RayLengthMode>::type >
	{
		typedef detail::RayLength<_Options,typename getOptionByTag<_Options,Tag_RayLengthMode>::type > Base;
		typedef Ray<_Options> ThisType;

		template<class _RayBase> inline Ray(const _RayBase& ray) : Base(ray)
		{
		}
		
		inline Ray() : Base()
		{
		}		
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};
	
}

#endif