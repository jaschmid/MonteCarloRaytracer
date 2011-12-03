/********************************************************/
// FILE: ImageWriter.h
// DESCRIPTION: ImageWriters for engine output
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

#ifndef RAYTRACE_IMAGE_WRITER_GUARD
#define RAYTRACE_IMAGE_WRITER_GUARD

#include <boost/shared_ptr.hpp>
#include <RaytraceCommon.h>
#include <types.h>
#include <array>
#include <vector>


namespace Raytrace {

	//basically the editable image code stolen from HAGE


	typedef enum
	{
		R5G6B5,
		R8G8B8A8,
		A8R8G8B8,
		RGBA_FLOAT32,
		DXTC1,
		DXTC3,
		DXTC5
	} IMAGE_FORMAT;

	// Signature
	template<IMAGE_FORMAT _format> class Pixel;

	// Signature
	template<IMAGE_FORMAT source,IMAGE_FORMAT dest> class PixelConverter;

	template<> class Pixel<R8G8B8A8>
	{
	public:

		Pixel(){}
		Pixel(u8 r,u8 g,u8 b,u8 a):red(r),green(g),blue(b),alpha(a) {}
		Pixel(u32 color):red((color>>24)&0xff),green((color>>16)&0xff),blue((color>>8)&0xff),alpha((color>>0)&0xff) {}

		u8 Red() const {return red;}
		u8 Green() const {return green;}
		u8 Blue() const {return blue;}
		u8 Alpha() const {return alpha;}
		
		u32 Data() const {return *(u32*)this;}

		void SetRed(u8 r) {red = r;}
		void SetGreen(u8 g) {green = g;}
		void SetBlue(u8 b) {blue = b;}
		void SetAlpha(u8 a) {alpha = a;}
		void SetData(u32 d) {*(u32*)this = d;}

		bool operator ==(const Pixel<R8G8B8A8>& other)
		{ return other.Data() == Data(); }

		static const u32 RedMax = 0xff;
		static const u32 GreenMax = 0xff;
		static const u32 BlueMax = 0xff;
		static const u32 AlphaMax = 0xff;

		template<IMAGE_FORMAT _Out> operator Pixel<_Out>() const
		{
			return PixelConverter<R8G8B8A8,_Out>().Convert(*this);
		}

		typedef u8 RedType;
		typedef u8 GreenType;
		typedef u8 BlueType;
		typedef u8 AlphaType;

		static const IMAGE_FORMAT Format = R8G8B8A8;

	private:

		u8 alpha,blue,green,red;
	};
	
	template<> class Pixel<A8R8G8B8>
	{
	public:

		Pixel(){}
		Pixel(u8 r,u8 g,u8 b,u8 a):red(r),green(g),blue(b),alpha(a) {}
		Pixel(u32 color):red((color>>16)&0xff),green((color>>8)&0xff),blue((color>>0)&0xff),alpha((color>>24)&0xff) {}

		u8 Red() const {return red;}
		u8 Green() const {return green;}
		u8 Blue() const {return blue;}
		u8 Alpha() const {return alpha;}
		
		u32 Data() const {return *(u32*)this;}

		void SetRed(u8 r) {red = r;}
		void SetGreen(u8 g) {green = g;}
		void SetBlue(u8 b) {blue = b;}
		void SetAlpha(u8 a) {alpha = a;}
		void SetData(u32 d) {*(u32*)this = d;}

		bool operator ==(const Pixel<A8R8G8B8>& other)
		{ return other.Data() == Data(); }

		static const u32 RedMax = 0xff;
		static const u32 GreenMax = 0xff;
		static const u32 BlueMax = 0xff;
		static const u32 AlphaMax = 0xff;
		
		template<IMAGE_FORMAT _Out> operator Pixel<_Out>() const
		{
			
			return PixelConverter<A8R8G8B8,_Out>().Convert(*this);
		}

		typedef u8 RedType;
		typedef u8 GreenType;
		typedef u8 BlueType;
		typedef u8 AlphaType;

		static const IMAGE_FORMAT Format = A8R8G8B8;

	private:

		u8 blue,green,red,alpha;
	};

	template<> class Pixel<R5G6B5>
	{
	public:

		Pixel(){}
		Pixel(u8 r,u8 g,u8 b):data( (((u16)r)<<RedOffset) | (((u16)g)<<GreenOffset) | (((u16)b)<<BlueOffset)) {assert(r <= RedMax);assert(g <= GreenMax);assert(b <= BlueMax);}
		Pixel(u16 color):data(color) {}

		u8 Red() const {return (data >> RedOffset) & RedMax ;}
		u8 Green() const {return (data >> GreenOffset) & GreenMax;}
		u8 Blue() const {return (data >> BlueOffset) & BlueMax;}

		u16 Data() const {return data;}

		void SetRed(u8 r) {assert(r <= RedMax); data = (data & ~(RedMax << RedOffset)) | (r << RedOffset); }
		void SetGreen(u8 g) {assert(g <= GreenMax); data = (data & ~(GreenMax << GreenOffset)) | (g << GreenOffset); }
		void SetBlue(u8 b) {assert(b <= BlueMax); data = (data & ~(BlueMax << BlueOffset)) | (b << BlueOffset); }
		void SetData(u16 d) {data=d;}

		static const u32 RedMax = 0x1f;
		static const u32 GreenMax = 0x3f;
		static const u32 BlueMax = 0x1f;
		static const u32 AlphaMax = 0x0;
		
		template<IMAGE_FORMAT _Out> operator Pixel<_Out>() const
		{
			return PixelConverter<R5G6B5,_Out>().Convert(*this);
		}
		
		typedef u8 RedType;
		typedef u8 GreenType;
		typedef u8 BlueType;
		typedef void AlphaType;

		static const IMAGE_FORMAT Format = R5G6B5;

	private:

		static const u32 RedOffset = 11;
		static const u32 GreenOffset = 5;
		static const u32 BlueOffset = 0;

		u16 data;
	};

	template<> class Pixel<RGBA_FLOAT32>
	{
	public:

		Pixel(){}
		Pixel(f32 r,f32 g,f32 b,f32 a):_data(r,g,b,a) {assert(r <= RedMax);assert(g <= GreenMax);assert(b <= BlueMax);}
		Pixel(const ::Eigen::Matrix<f32,4,1>& color):_data(color) {}

		f32 Red() const {return _data.x();}
		f32 Green() const {return _data.y();}
		f32 Blue() const {return _data.z();}
		f32 Alpha() const {return _data.w();}

		const ::Eigen::Matrix<f32,4,1>& Data() const {return _data;}

		void SetRed(f32 r) {assert(r <= RedMax); _data.x() = r; }
		void SetGreen(f32 g) {assert(g <= GreenMax); _data.y() = g; }
		void SetBlue(f32 b) {assert(b <= BlueMax); _data.z() = b; }
		void SetAlpha(f32 a) {assert(a <= BlueMax); _data.w() = a; }
		void SetData(const ::Eigen::Matrix<f32,4,1>& d) {_data=d;}

		static const f32 RedMax;
		static const f32 GreenMax;
		static const f32 BlueMax;
		static const f32 AlphaMax;
		
		template<IMAGE_FORMAT _Out> operator Pixel<_Out>() const
		{
			return PixelConverter<RGBA_FLOAT32,_Out>().Convert(*this);
		}
		
		typedef f32 RedType;
		typedef f32 GreenType;
		typedef f32 BlueType;
		typedef f32 AlphaType;

		static const IMAGE_FORMAT Format = RGBA_FLOAT32;

	private:

		::Eigen::Matrix<f32,4,1> _data;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	/* Alpha Converter */

	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _sourceType,class _destType> class ConvertAlpha 
	{
	public:
		static void SetAlpha(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest)
		{
			u64 val = (u64)source.Alpha();
			val *= Pixel<_destFormat>::AlphaMax;
			val /= Pixel<_sourceFormat>::AlphaMax;
			dest.SetAlpha(static_cast<typename Pixel<_destFormat>::AlphaType>(val));
		}
	};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _sourceType> class ConvertAlpha<_sourceFormat,_destFormat,_sourceType,void>
	{public: static void SetAlpha(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest){}};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat> class ConvertAlpha<_sourceFormat,_destFormat,void,void>
	{public: static void SetAlpha(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest){}};

	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _destType> class ConvertAlpha<_sourceFormat,_destFormat,void,_destType>
	{
	public:
		static void SetAlpha(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest)
		{
			dest.SetAlpha(static_cast<typename Pixel<_destFormat>::AlphaType>(Pixel<_destFormat>::AlphaMax));
		}
	};
	
	/* Red Converter */

	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _sourceType,class _destType> class ConvertRed 
	{
	public:
		static void SetRed(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest)
		{
			u64 val = (u64)source.Red();
			val *= Pixel<_destFormat>::RedMax;
			val /= Pixel<_sourceFormat>::RedMax;
			dest.SetRed(static_cast<typename Pixel<_destFormat>::RedType>(val));
		}
	};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _sourceType> class ConvertRed<_sourceFormat,_destFormat,_sourceType,void>
	{public: static void SetRed(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest){}};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat> class ConvertRed<_sourceFormat,_destFormat,void,void>
	{public: static void SetRed(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest){}};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _destType> class ConvertRed<_sourceFormat,_destFormat,void,_destType>
	{
	public:
		static void SetRed(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest)
		{
			dest.SetRed(static_cast<typename Pixel<_destFormat>::RedType>(Pixel<_destFormat>::RedMax));
		}
	};
	
	/* Green Converter */

	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _sourceType,class _destType> class ConvertGreen 
	{
	public:
		static void SetGreen(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest)
		{
			u64 val = (u64)source.Green();
			val *= Pixel<_destFormat>::GreenMax;
			val /= Pixel<_sourceFormat>::GreenMax;
			dest.SetGreen(static_cast<typename Pixel<_destFormat>::GreenType>(val));
		}
	};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _sourceType> class ConvertGreen<_sourceFormat,_destFormat,_sourceType,void>
	{public: static void SetGreen(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest){}};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat> class ConvertGreen<_sourceFormat,_destFormat,void,void>
	{public: static void SetGreen(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest){}};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _destType> class ConvertGreen<_sourceFormat,_destFormat,void,_destType>
	{
	public:
		static void SetGreen(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest)
		{
			dest.SetGreen(static_cast<typename Pixel<_destFormat>::GreenType>(Pixel<_destFormat>::GreenMax));
		}
	};

	/* Blue Converter */

	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _sourceType,class _destType> class ConvertBlue 
	{
	public:
		static void SetBlue(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest)
		{
			u64 val = (u64)source.Blue();
			val *= Pixel<_destFormat>::BlueMax;
			val /= Pixel<_sourceFormat>::BlueMax;
			dest.SetBlue(static_cast<typename Pixel<_destFormat>::BlueType>(val));
		}
	};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _sourceType> class ConvertBlue<_sourceFormat,_destFormat,_sourceType,void>
	{public: static void SetBlue(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest){}};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat> class ConvertBlue<_sourceFormat,_destFormat,void,void>
	{public: static void SetBlue(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest){}};
	template<IMAGE_FORMAT _sourceFormat,IMAGE_FORMAT _destFormat,class _destType> class ConvertBlue<_sourceFormat,_destFormat,void,_destType>
	{
	public:
		static void SetBlue(const Pixel<_sourceFormat>& source,Pixel<_destFormat>& dest)
		{
			dest.SetBlue(static_cast<typename Pixel<_destFormat>::BlueType>(Pixel<_destFormat>::BlueMax));
		}
	};
	
	template<IMAGE_FORMAT source,IMAGE_FORMAT dest> class PixelConverter
	{
	public:

		static Pixel<dest> Convert(const Pixel<source>& s)
		{
			Pixel<dest> d;
			ConvertRed<source,dest,typename Pixel<source>::RedType,typename Pixel<dest>::RedType>::SetRed(s,d);
			ConvertGreen<source,dest,typename Pixel<source>::GreenType,typename Pixel<dest>::GreenType>::SetGreen(s,d);
			ConvertBlue<source,dest,typename Pixel<source>::BlueType,typename Pixel<dest>::BlueType>::SetBlue(s,d);
			ConvertAlpha<source,dest,typename Pixel<source>::AlphaType,typename Pixel<dest>::AlphaType>::SetAlpha(s,d);
			return d;
		}

	};

	template<IMAGE_FORMAT dest> class PixelConverter<RGBA_FLOAT32,dest>
	{
	public:
		static Pixel<dest> Convert(const Pixel<RGBA_FLOAT32>& s)
		{
			Pixel<R8G8B8A8> i;
			i.SetRed((u8)(s.Red()*255.0f));
			i.SetGreen((u8)(s.Green()*255.0f));
			i.SetBlue((u8)(s.Blue()*255.0f));
			i.SetAlpha((u8)(s.Alpha()*255.0f));
			return PixelConverter<dest,R8G8B8A8>::Convert(i);
		}
	};


	
	template<bool Alpha = true> class DXTC1Block
	{
	public:

		/* Public Types */
		
		typedef Pixel<R8G8B8A8> PixelType;
		const static IMAGE_FORMAT PixelFormat = PixelType::Format;

		/* Public Functions */

		PixelType operator () (u32 x,u32 y) const
		{
			return getColor(getInterpolant(x,y));
		}

		/* Public Members */

		static const u32 BlockWidth = 4;
		static const u32 BlockHeight = 4;

	private:

		u32 getInterpolant(u32 x,u32 y) const
		{
			u32 loc = y*BlockWidth+x;
			u32 code = (interpolants >> (loc*InterpolantSize) ) & InterpolantMax;
			return code;
		}

		PixelType getColor(u32 interpolant) const
		{
			if(interpolant == 0)
				return PixelConverter<R5G6B5,PixelFormat>::Convert(color1);
			else if(interpolant == 1)
				return PixelConverter<R5G6B5,PixelFormat>::Convert(color2);
			else if(Alpha && color1.Data() < color2.Data() )
			{
				if(interpolant == 2)
				{
					PixelType c0 = PixelConverter<R5G6B5,PixelFormat>::Convert(color1);
					PixelType c1 = PixelConverter<R5G6B5,PixelFormat>::Convert(color2);
					return PixelType(
							(u8)(((u32)c0.Red() + (u32)c1.Red()) /2),
							(u8)(((u32)c0.Green() + (u32)c1.Green()) /2),
							(u8)(((u32)c0.Blue() + (u32)c1.Blue()) /2),
							0xff);
				}
				else
					return PixelType(0,0,0,0);
			}
			else if(interpolant == 2)
			{
				PixelType c0 = PixelConverter<R5G6B5,PixelFormat>::Convert(color1);
				PixelType c1 = PixelConverter<R5G6B5,PixelFormat>::Convert(color2);
				return PixelType(
						(u8)(((u32)c0.Red() * 2 + (u32)c1.Red()) /3),
						(u8)(((u32)c0.Green() * 2 + (u32)c1.Green()) /3),
						(u8)(((u32)c0.Blue() * 2 + (u32)c1.Blue()) /3),
						0xff);
			}
			else
			{
				PixelType c0 = PixelConverter<R5G6B5,PixelFormat>::Convert(color1);
				PixelType c1 = PixelConverter<R5G6B5,PixelFormat>::Convert(color2);
				return PixelType(
						(u8)(((u32)c0.Red() + (u32)c1.Red() * 2 ) /3),
						(u8)(((u32)c0.Green() + (u32)c1.Green() * 2 ) /3),
						(u8)(((u32)c0.Blue() + (u32)c1.Blue() * 2 ) /3),
						0xff);
			}
			
		}

		static const u32 InterpolantSize = 2;
		static const u32 InterpolantMax = (1<< (InterpolantSize))-1;

		Pixel<R5G6B5>						color1;
		Pixel<R5G6B5>						color2;
		u32									interpolants;
	};

	class DXTC3Block
	{
	public:

		/* Public Types */
		
		typedef Pixel<R8G8B8A8> PixelType;
		const static IMAGE_FORMAT PixelFormat = PixelType::Format;

		/* Public Functions */

		PixelType operator () (u32 x,u32 y) const
		{
			return _colorData(x,y);
		}

		/* Public Members */

		static const u32 BlockWidth = DXTC1Block<false>::BlockWidth;
		static const u32 BlockHeight = DXTC1Block<false>::BlockHeight;

	private:
		
		std::array<u8,BlockWidth*BlockHeight/2> _alphaData;
		DXTC1Block<false> _colorData;
	};

	class DXTC5Block
	{
	public:

		/* Public Types */
		
		typedef Pixel<R8G8B8A8> PixelType;
		const static IMAGE_FORMAT PixelFormat = PixelType::Format;

		/* Public Functions */

		PixelType operator () (u32 x,u32 y) const
		{
			return _colorData(x,y);
		}

		/* Public Members */

		static const u32 BlockWidth = DXTC1Block<false>::BlockWidth;
		static const u32 BlockHeight = DXTC1Block<false>::BlockHeight;

	private:

		u8					_alpha1;
		u8					_alpha2;
		std::array<u8,BlockWidth*BlockHeight*3/8> _alphaData;
		DXTC1Block<false> _colorData;
	};
	
	
	template<IMAGE_FORMAT _format> class ConstImageRect
	{
	public:
		
		/*public types*/
		typedef Pixel<_format> PixelType;
		
		/*public coonstructors*/
		ConstImageRect(const void* pData,const Vector2u& size,const u32& stride):
			_rect(0,0,size.x(),size.y()),_data((const PixelType*)pData),_size(size.x(),size.y()),_stride(stride)
		{
		}

		/*public functions*/

		const PixelType& operator()(u32 x,u32 y) const {assert(x<_size.x()); assert(y<_size.y()); return getPixel(x,y);}
		u32 XSize() const{return _size.x();}
		u32 YSize() const{return _size.y();}
		
		ConstImageRect<_format> Rect(const Vector4u& sub) const
		{
			assert(sub.x() < sub.z());
			assert(sub.y() < sub.w());
			assert(sub.z() <= XSize());
			assert(sub.w() <= YSize());
			return ConstImageRect<_format>(_data,Vector4i(sub.x()+_rect.x(),sub.y()+_rect.y(),sub.z()+_rect.x(),sub.w()+_rect.y()),_stride);
		}

	private:


		/*private coonstructors*/
		ConstImageRect(const PixelType* pData,const Vector4u& rect,const u32& stride):
			_rect(rect),_data(pData),_size(rect.z()-rect.x(),rect.w()-rect.y()),_stride(stride)
		{
		}

		/*private functions*/
		const PixelType& getPixel(u32 x,u32 y) const {return _data[(y+_rect.y())*_stride+(x+_rect.x())];}

		/*private members*/
		const Vector2u	_size;
		const Vector4u	_rect;
		const u32			_stride;
		const PixelType*	_data;

		template<class _PixelType> friend class UncompressedImageData;
		template<IMAGE_FORMAT _format2> friend class ImageRect;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};
	
	template<class _PixelType> class UncompressedImageData;

	template<IMAGE_FORMAT _format> class ImageRect
	{
	public:
		
		/*public types*/
		typedef Pixel<_format> PixelType;
		
		/*public coonstructors*/
		ImageRect(void* pData,const Vector2u& size,u32 stride):
			_rect(0,0,size.x(),size.y()),_data((PixelType*)pData),_size(size.x(),size.y()),_stride(stride)
		{
		}

		/*public functions*/
		template<IMAGE_FORMAT _format2> void CopyFrom(const ImageRect<_format2>& source)
		{
			assert(source.XSize() == XSize());
			assert(source.YSize() == YSize());
			
			for(u32 y = 0; y < _size.y(); ++y)
				for(u32 x = 0; x < _size.x(); ++x)
					getPixel(x,y) = PixelConverter<_format,_format2>::Convert(source.getPixel(x,y));
		}
		
		template<> void CopyFrom(const ImageRect<_format>& source)
		{
			assert(source.XSize() == XSize());
			assert(source.YSize() == YSize());
			
			for(u32 y = 0; y < _size.y(); ++y)
				for(u32 x = 0; x < _size.x(); ++x)
					getPixel(x,y) = source.getPixel(x,y);
		}

		template<IMAGE_FORMAT _format2> void CopyFrom(const ConstImageRect<_format2>& source)
		{
			assert(source.XSize() == XSize());
			assert(source.YSize() == YSize());
			
			for(u32 y = 0; y < _size.y(); ++y)
				for(u32 x = 0; x < _size.x(); ++x)
					getPixel(x,y) = PixelConverter<_format,_format2>::Convert(source.getPixel(x,y));
		}
		
		template<> void CopyFrom(const ConstImageRect<_format>& source)
		{
			assert(source.XSize() == XSize());
			assert(source.YSize() == YSize());
			
			for(u32 y = 0; y < _size.y(); ++y)
				for(u32 x = 0; x < _size.x(); ++x)
					getPixel(x,y) = source.getPixel(x,y);
		}

		PixelType& operator()(u32 x,u32 y) {assert(x<_size.x()); assert(y<_size.y()); return getPixel(x,y);}
		const PixelType& operator()(u32 x,u32 y) const {assert(x<_size.x()); assert(y<_size.y()); return getPixel(x,y);}
		u32 XSize() const{return _size.x();}
		u32 YSize() const{return _size.y();}

		ImageRect<_format> Rect(const Vector4u& sub)
		{
			assert(sub.x() < sub.z());
			assert(sub.y() < sub.w());
			assert(sub.z() <= XSize());
			assert(sub.w() <= YSize());
			return ImageRect<_format>(_data,Vector4u(sub.x()+_rect.x(),sub.y()+_rect.y(),sub.z()+_rect.x(),sub.w()+_rect.y()),_stride);
		}
		
		ConstImageRect<_format> Rect(const Vector4u& sub) const
		{
			assert(sub.x() < sub.z());
			assert(sub.y() < sub.w());
			assert(sub.z() <= XSize());
			assert(sub.w() <= YSize());
			return ConstImageRect<_format>(_data,Vector4u(sub.x()+_rect.x(),sub.y()+_rect.y(),sub.z()+_rect.x(),sub.w()+_rect.y()),_stride);
		}

	private:



		/*private coonstructors*/
		ImageRect(PixelType* pData,const Vector4u& rect,const u32& stride):
			_rect(rect),_data(pData),_size(rect.z()-rect.x(),rect.w()-rect.y()),_stride(stride)
		{
		}

		/*private functions*/
		PixelType& getPixel(u32 x,u32 y) {return _data[(y+_rect.y())*_stride+(x+_rect.x())];}
		const PixelType& getPixel(u32 x,u32 y) const {return _data[(y+_rect.y())*_stride+(x+_rect.x())];}

		/*private members*/
		const Vector2u	_size;
		const Vector4u	_rect;
		const u32			_stride;
		PixelType*			_data;

		template<class _PixelType> friend class UncompressedImageData;
	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};

	template<class _PixelType> class UncompressedImageData
	{
	public:

		typedef _PixelType PixelType;

		/*public constructors*/

		UncompressedImageData(u32 xSize,u32 ySize) : _xSize(xSize),_ySize(ySize),_Data(_xSize*_ySize) {}
		UncompressedImageData(u32 xSize,u32 ySize,const _PixelType* pData) : _xSize(xSize),_ySize(ySize),_Data(_xSize*_ySize) { memcpy(_Data.data(),pData,_Data.size()*sizeof(_PixelType));}

		template<class _srcType> UncompressedImageData(const UncompressedImageData<_srcType>& source)  : _xSize(source.XSize()),_ySize(source.YSize())
		{
			_Data = source.convertTo<_PixelType>().getData();
		}

		UncompressedImageData(const UncompressedImageData<_PixelType>& source)  : _xSize(source.XSize()),_ySize(source.YSize())
		{
			_Data = source.getData();
		}

		/*public functions*/

		UncompressedImageData<_PixelType>& operator = (const UncompressedImageData<_PixelType>& other)
		{
			assert(_xSize == other._xSize);
			assert(_ySize == other._ySize);
			_Data = other._Data;
			assert(_Data.size() == _xSize*_ySize);
		}

		_PixelType& operator()(u32 x,u32 y) {return _Data[y*_xSize+x];}
		const _PixelType& operator()(u32 x,u32 y) const {return _Data[y*_xSize+x];}
		u32 XSize() const{return _xSize;}
		u32 YSize() const{return _ySize;}
		
		ImageRect<_PixelType::Format> GetRect(const Vector4u& rect)
		{
			assert(rect.x < rect.z);
			assert(rect.y < rect.w);
			assert(rect.z <= XSize());
			assert(rect.w <= YSize());
			return ImageRect<_PixelType::Format>(_Data.data(),rect,XSize());
		}

		ImageRect<_PixelType::Format> GetRect()
		{
			return ImageRect<_PixelType::Format>(_Data.data(),Vector4u(0,0,XSize(),YSize()),XSize());
		}

		ImageRect<_PixelType::Format> GetRect(const Vector4u& rect) const
		{
			assert(rect.x < rect.z);
			assert(rect.y < rect.w);
			assert(rect.z <= XSize());
			assert(rect.w <= YSize());
			return ConstImageRect<_PixelType::Format>(_Data.data(),rect,XSize());
		}

		ImageRect<_PixelType::Format> GetRect() const
		{
			return ConstImageRect<_PixelType::Format>(_Data.data(),Vector4u(0,0,XSize(),YSize()),XSize());
		}

		const void* GetData() const{return _Data.data();}
		const u32 GetDataSize() const{return (u32)(sizeof(_PixelType)*_Data.size());}
	private:

		/*private functions*/

		template<class _destType> typename UncompressedImageData<_destType> convertTo() const
		{
			UncompressedImageData<_destType> result(_xSize,_ySize);
			for(int iy = 0; iy < _ySize; ++iy)
				for(int ix = 0; ix < _xSize; ++ix)
					result(ix,iy) = PixelConverter<_PixelType::Format,_destType::Format>::Convert((*this)(ix,iy));
			return result;
		}

		const std::vector<_PixelType>& getData() const
		{
			return _Data;
		}

		const u32			_xSize;
		const u32			_ySize;
		std::vector<_PixelType>	_Data;

		template<class _FriendFormat> friend class UncompressedImageData;
		template<IMAGE_FORMAT _FriendFormat> friend class ImageData;
	};

	template<class _BlockType> class BlockCompressedImageData
	{
	public:

		/*public types*/

		typedef _BlockType BlockBase;
		
		/*public constructors*/

		BlockCompressedImageData(u32 xSize,u32 ySize,const void* pData) : _xSize(xSize),_ySize(ySize),_xBlocks(xSize / BlockBase::BlockWidth)
		{ 
			u32 nBlocks = _xSize*_ySize / BlockBase::BlockHeight /BlockBase::BlockWidth;
			_Data.resize(nBlocks);
			memcpy(_Data.data(),pData,nBlocks*sizeof(BlockBase));
		}

		/*public functions*/

		// Pixel<_format>& operator()(u32 x,u32 y) {return _Data[y*_xSize+x];} can't write directly
		typename _BlockType::PixelType operator()(u32 x,u32 y) const 
		{
			u32 xBlock = x/BlockBase::BlockWidth;
			u32 yBlock = y/BlockBase::BlockHeight;
			
			return _Data[yBlock*_xBlocks+xBlock](x%BlockBase::BlockWidth,y%BlockBase::BlockHeight);
		}
		u32 XSize() const{return _xSize;}
		u32 YSize() const{return _ySize;}

		const void* GetData() const{return _Data.data();}
		const u32 GetDataSize() const{return sizeof(BlockBase)*_Data.size();}

	private:

		/*private functions*/

		template<class _destType> UncompressedImageData<_destType> convertTo() const
		{
			UncompressedImageData<_destType> result(_xSize,_ySize);
			for(u32 iy = 0; iy < _ySize; ++iy)
				for(u32 ix = 0; ix < _xSize; ++ix)
					result(ix,iy) = PixelConverter<_BlockType::PixelFormat,_destType::Format>::Convert((*this)(ix,iy));
			return result;
		}

		const std::vector<BlockBase>& getData() const
		{
			return _Data;
		}

		const u32			_xSize;
		const u32			_ySize;
		const u32			_xBlocks;
		std::vector<BlockBase>	_Data;
		
		template<class _FriendFormat> friend class UncompressedImageData;
		template<IMAGE_FORMAT _FriendFormat> friend class ImageData;
	};
	
	template<IMAGE_FORMAT _format> class ImageData : public UncompressedImageData<typename Pixel<_format>>
	{
	public:

		/*public constructors*/

		ImageData(u32 xSize,u32 ySize) : UncompressedImageData<typename Pixel<_format>>(xSize,ySize) {}
		ImageData(u32 xSize,u32 ySize,const Pixel<_format>* pData) : UncompressedImageData<typename Pixel<_format>>(xSize,ySize,pData) {}

		template<IMAGE_FORMAT _srcFormat> ImageData(const ImageData<_srcFormat>& source)  : UncompressedImageData<typename Pixel<_format>>( (UncompressedImageData<Pixel<_format>>) source.convertTo<Pixel<_format>>() )
		{
		}

	private:
	};

	template<> class ImageData<DXTC1> : public BlockCompressedImageData<DXTC1Block<>>
	{
	public:
		/*public constructors*/

		ImageData(u32 xSize,u32 ySize,const void* pData) : BlockCompressedImageData<DXTC1Block<>>(xSize,ySize,pData) {}
		
	private:
	};

	template<> class ImageData<DXTC3> : public BlockCompressedImageData<DXTC3Block>
	{
	public:
		/*public constructors*/

		ImageData(u32 xSize,u32 ySize,const void* pData) : BlockCompressedImageData<DXTC3Block>(xSize,ySize,pData) {}
		
	private:
	};

	template<> class ImageData<DXTC5> : public BlockCompressedImageData<DXTC5Block>
	{
	public:
		/*public constructors*/

		ImageData(u32 xSize,u32 ySize,const void* pData) : BlockCompressedImageData<DXTC5Block>(xSize,ySize,pData) {}
		
	private:
	};

	template<class _ImageRect> inline void FillRectangle(_ImageRect& rect,const typename _ImageRect::PixelType& color)
	{
		for(unsigned int ix = 0; ix < rect.XSize(); ++ix)
			for(unsigned int iy = 0; iy < rect.YSize(); ++iy)
				rect(ix,iy) = color;
	}

}

#endif