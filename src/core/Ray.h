/********************************************************/
/* FILE: Ray.h                                   */
/* DESCRIPTION: Raytrace Ray class                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include <RaytraceCommon.h>
#include <array>

#ifndef RAYTRACE_RAY_H_INCLUDED
#define RAYTRACE_RAY_H_INCLUDED

namespace Raytrace {

	struct Ray
	{
	public:
		inline Ray(const Vector3& origin_,const Vector3& direction_,float length_) :
			_origin(origin_),
			_direction(direction_),
			_length(length_),
			_end(origin_+direction_*length_)
		{
		}

		inline Ray(const Ray& other) : 
			_origin(other._origin),
			_direction(other._direction),
			_length(other._length),
			_end(other._origin+other._direction*other._length)
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

		inline const Vector3& end() const
		{
			return _end;
		}

		inline const Real& length() const
		{
			return _length;
		}
		
		inline void setOrigin(const Vector3& origin)
		{
			_origin = origin;
			_end = _origin + _length*_direction;
		}

		inline void setDirection(const Vector3& direction)
		{
			_direction = direction;
			_end = _origin + _length*_direction;
		}

		inline void setLength(Real length)
		{
			_length = length;
			_end = _origin + _length*_direction;
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
			_end = _origin + _length*_direction;

			return *this;
		}

		inline const Vector3 operator ()(const Real& t) const
		{
			return origin() + direction()*t;
		}

		Vector3	_origin;
		Vector3 _direction;
		Vector3	_end;
		Real	_length;
	};

}

#endif