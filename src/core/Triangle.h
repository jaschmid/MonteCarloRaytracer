/********************************************************/
/* FILE: Triangle.h                                   */
/* DESCRIPTION: Raytrace Triangle class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include <RaytraceCommon.h>
#include <array>


#ifndef Raytrace_TRIANGLE_H_INCLUDED
#define Raytrace_TRIANGLE_H_INCLUDED

namespace Raytrace {
	
	struct Triangle
	{
	public:
		
		inline Triangle(const Vector3& p1_,const Vector3& p2_,const Vector3& p3_)
		{
			setPoint(0,p1_);
			setPoint(1,p1_);
			setPoint(2,p1_);
		}

		inline Triangle(const Triangle& other) : _data(other._data)
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
			_data[i].head<3>() = val;
		}
		
		inline const Vector3 point(int i) const
		{
			return _data[i].head<3>();
		}

		std::array<Vector4,3>					_data;
	};
}

#endif