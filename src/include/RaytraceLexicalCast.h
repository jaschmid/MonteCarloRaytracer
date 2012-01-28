/********************************************************/
// FILE: RaytraceLexicalCast.h
// DESCRIPTION: Raytracer Lexical Casts
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

#ifndef RAYTRACE_LEXICAL_CAST_GUARD
#define RAYTRACE_LEXICAL_CAST_GUARD

#include "RaytraceCommon.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>

namespace Raytrace {

	template<class _Output,class _Input> inline bool LexicalCast(const _Input& input,_Output& output)
	{
		output = boost::lexical_cast<_Output,_Input>(input);
		return true;
	}
	
	template<> inline bool LexicalCast<bool,String>(const String& input,bool& output)
	{
		if(boost::iequals(input,"true"))
			output = true;
		else if(boost::iequals(input,"false"))
			output = false;
		else
			return false;
		return true;
	}
	
	template<> inline bool LexicalCast<String,bool>(const bool& input,String& output)
	{
		if(input)
			output = String("true");
		else
			output = String("false");
		return true;
	}

	template<> inline bool LexicalCast<String,Vector3>(const Vector3& input,String& output)
	{
		output = boost::lexical_cast<String>(input.x()) + " " + boost::lexical_cast<String>(input.y()) + " " + boost::lexical_cast<String>(input.z());
		return true;
	}
	
	template<> inline bool LexicalCast<Vector3,String>(const String& input,Vector3& output)
	{
		std::vector<String> elements;
		String trimmed = boost::algorithm::trim_copy(input);
		boost::algorithm::split(elements,trimmed,boost::algorithm::is_space(),boost::algorithm::token_compress_on);
		if(elements.size() != 3)
			return false;
		output.x() = boost::lexical_cast<Real,String>(elements[0]);
		output.y() = boost::lexical_cast<Real,String>(elements[1]);
		output.z() = boost::lexical_cast<Real,String>(elements[2]);
		return true;
	}
	
	template<> inline bool LexicalCast<String,Vector2>(const Vector2& input,String& output)
	{
		output = boost::lexical_cast<String>(input.x()) + " " + boost::lexical_cast<String>(input.y());
		return true;
	}
	
	template<> inline bool LexicalCast<Vector2,String>(const String& input,Vector2& output)
	{
		std::vector<String> elements;
		String trimmed = boost::algorithm::trim_copy(input);
		boost::algorithm::split(elements,trimmed,boost::algorithm::is_space(),boost::algorithm::token_compress_on);
		if(elements.size() != 2)
			return false;
		output.x() = boost::lexical_cast<Real,String>(elements[0]);
		output.y() = boost::lexical_cast<Real,String>(elements[1]);
		return true;
	}
	
	template<> inline bool LexicalCast<String,Vector2u>(const Vector2u& input,String& output)
	{
		output = boost::lexical_cast<String>(input.x()) + " " + boost::lexical_cast<String>(input.y());
		return true;
	}
	
	template<> inline bool LexicalCast<Vector2u,String>(const String& input,Vector2u& output)
	{
		std::vector<String> elements;
		String trimmed = boost::algorithm::trim_copy(input);
		boost::algorithm::split(elements,trimmed,boost::algorithm::is_space(),boost::algorithm::token_compress_on);
		if(elements.size() != 2)
			return false;
		output.x() = boost::lexical_cast<unsigned int,String>(elements[0]);
		output.y() = boost::lexical_cast<unsigned int,String>(elements[1]);
		return true;
	}

	template<> inline bool LexicalCast<String,Vector4>(const Vector4& input,String& output)
	{
		output = boost::lexical_cast<String>(input.x()) + " " + boost::lexical_cast<String>(input.y()) + " " + boost::lexical_cast<String>(input.z()) + " " + boost::lexical_cast<String>(input.w());
		return true;
	}
	
	template<> inline bool LexicalCast<Vector4,String>(const String& input,Vector4& output)
	{
		std::vector<String> elements;
		String trimmed = boost::algorithm::trim_copy(input);
		boost::algorithm::split(elements,trimmed,boost::algorithm::is_space(),boost::algorithm::token_compress_on);
		if(elements.size() != 4)
			return false;
		output.x() = boost::lexical_cast<Real,String>(elements[0]);
		output.y() = boost::lexical_cast<Real,String>(elements[1]);
		output.z() = boost::lexical_cast<Real,String>(elements[2]);
		output.w() = boost::lexical_cast<Real,String>(elements[3]);
		return true;
	}
};

#endif