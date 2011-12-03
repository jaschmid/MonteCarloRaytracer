/********************************************************/
/* FILE: Ray.h                                   */
/* DESCRIPTION: Raytrace Ray class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include <RaytraceCommon.h>
#include <array>
#include "SIMDType.h"
#include "IntersectorBase.h"

#ifndef RAYTRACE_AABB_H_INCLUDED
#define RAYTRACE_AABB_H_INCLUDED

namespace Raytrace {
	

	struct AABB
	{
		typedef AABB Minimum;
		typedef PrimitiveClassAxisAlignedBox PrimitiveClass;
		typedef Vector3 Vector_T;
		typedef Vector3i Integer_T;
		typedef f32 Scalar_T;

		inline AABB(const Vector3& min_,const Vector3& max_)
		{
			min() = min_;
			max() = max_;
		}

		inline AABB() {}

		inline AABB(const AABB& a,const AABB& b)
		{
			min() = Vector3(std::min(a.min().x(),b.min().x()) , std::min(a.min().y(),b.min().y()), std::min(a.min().z(),b.min().z()));
			max() = Vector3(std::max(a.max().x(),b.max().x()) , std::max(a.max().y(),b.max().y()), std::max(a.max().z(),b.max().z()));
		}

		inline AABB(const AABB& a,const Vector3& b)
		{
			min() = Vector3( std::min(a.min().x(),b.x()) , std::min(a.min().y(),b.y()), std::min(a.min().z(),b.z()) );
			max() = Vector3( std::max(a.max().x(),b.x()) , std::max(a.max().y(),b.y()), std::max(a.max().z(),b.z()) );
		}

		template<class _Base> inline AABB(const _Base& tri)
		{
			*this = Empty();

			for(int ip = 0; ip < 3; ++ip)
				for(int id= 0; id< 3; ++id)
				{
					if(tri.point(ip)[id] < min()[id])
						min()[id] = tri.point(ip)[id];
					if(tri.point(ip)[id] > max()[id])
						max()[id] = tri.point(ip)[id];
				}
		}

		static inline AABB Empty() 
		{
			return AABB(
				Vector3(std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::infinity(),std::numeric_limits<Real>::infinity()),
				Vector3(-std::numeric_limits<Real>::infinity(),-std::numeric_limits<Real>::infinity(),-std::numeric_limits<Real>::infinity())
				);
		}

		inline bool contains(const Vector3& location) const
		{
			if( location.x() < min().x() || location.x() > max().x() ||
				location.y() < min().y() || location.y() > max().y() ||
				location.z() < min().z() || location.z() > max().z())
				return false;
			else
				return true;
		}

		inline bool isEmpty() const
		{
			Real dx =  (max().x() - min().x());
			Real dy =  (max().y() - min().y());
			Real dz =  (max().z() - min().z());
			return dx < 0.0f || dy < 0.0f || dz < 0.0f;
		}

		inline Real SAH() const
		{
			Real dx =  (max().x() - min().x());
			Real dy =  (max().y() - min().y());
			Real dz =  (max().z() - min().z());
			return dx*dy*2 + dx*dz*2 +dy*dz*2;
		}

		inline const Vector3& min() const {return _data[0];}
		inline Vector3& min() {return _data[0];}

		inline const Vector3& max() const {return _data[1];}
		inline Vector3& max() {return _data[1];}

		std::array<Vector3,2> _data;
	};

	template<int _Width> struct AABBAccel
	{
		typedef AABB Minimum;
		typedef PrimitiveClassAxisAlignedBox PrimitiveClass;

		typedef typename Vector3v<_Width>::type Vector_T;
		typedef SimdType<float,_Width> Scalar_T;
		static const int Width = _Width;

		inline AABBAccel()
		{
		}

		template<class _Base> inline AABBAccel(const ConstArrayWrapper<_Base>& right)
		{
			Scalar_T min_x,min_y,min_z;
			Scalar_T max_x,max_y,max_z;

			for(int i = 0; i < Width; ++i)
			{
				min_x[i] = right[i].min().x();
				min_y[i] = right[i].min().y();
				min_z[i] = right[i].min().z();
				
				max_x[i] = right[i].max().x();
				max_y[i] = right[i].max().y();
				max_z[i] = right[i].max().z();
			}

			_data[0] = Vector_T(min_x,min_y,min_z);
			_data[1] = Vector_T(max_x,max_y,max_z);
		}

		inline const Vector_T& min() const
		{
			return _data[0];
		}
		inline const Vector_T& max() const
		{
			return _data[1];
		}

		ALIGN_SIMD Vector_T _data[2];
	};

	
}

#endif