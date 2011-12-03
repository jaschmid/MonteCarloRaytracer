/********************************************************/
// FILE: TemplateOptions.h
// DESCRIPTION: Specifiy Options for templates using a vector
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

#ifndef RAYTRACE_TEMPLATE_OPTIONS_GUARD
#define RAYTRACE_TEMPLATE_OPTIONS_GUARD

#include <RaytraceCommon.h>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/utility/declval.hpp>

namespace Raytrace {

	namespace detail
	{
		template<bool _Option,class _A,class _B> struct conditional
		{
			typedef _A type;
		};
		
		template<class _A,class _B> struct conditional<false,_A,_B>
		{
			typedef _B type;
		};

		template<class _Base,class _New,class _Tag> struct matchTag
		{
			typedef typename conditional< std::is_same<typename _New::Tag, _Tag>::value, _New, _Base>::type type;
		};
	}

	template<class _ArgList,class _Tag> struct getOptionByTag
	{
		typedef typename
		  boost::mpl::fold<
			_ArgList,
			typename _Tag::DefaultValue,
			detail::matchTag<boost::mpl::_1,boost::mpl::_2,_Tag>
		  >::type	type;
	};

}

#endif
