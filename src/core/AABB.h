/********************************************************/
/* FILE: Ray.h                                   */
/* DESCRIPTION: Raytrace Ray class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include <RaytraceCommon.h>
#include <array>

#ifndef RAYTRACE_AABB_H_INCLUDED
#define RAYTRACE_AABB_H_INCLUDED

namespace Raytrace {
	

	struct AABB
	{
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

		inline AABB(const Triangle& tri)
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

}

#endif