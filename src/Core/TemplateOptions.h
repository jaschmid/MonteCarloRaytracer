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
	
	template<class _base, int _size> struct static_vector : private std::array<_base,_size>
	{
		typedef std::array<_base,_size> Base;
		typedef typename Base::value_type value_type;
		typedef typename Base::size_type size_type;
		typedef typename Base::reference reference;
		typedef typename Base::const_reference const_reference;
		typedef typename Base::iterator iterator;
		typedef typename Base::const_iterator const_iterator;

		static_vector() :_size (0)
		{
		}

		inline iterator begin()
		{
			return Base::begin();
		}
			
		inline iterator end()
		{
			return Base::begin() + (_size);
		}
			
		inline const_iterator begin() const
		{
			return Base::begin();
		}
			
		inline const_iterator end() const
		{
			return Base::begin() + (_size);
		}
	
		inline bool empty() const
		{
			return _size == 0;
		}

		inline size_type size() const
		{
			return _size;
		}

		inline reference operator[](size_type t)
		{
			return Base::operator[](t);
		}
	
		inline const_reference operator[](size_type t) const
		{
			return Base::operator[](t);
		}

		inline const_reference front() const
		{
			return Base::operator[](0);
		}
			
		inline reference front()
		{
			return Base::operator[](0);
		}
			
		inline const_reference back() const
		{
			return Base::operator[](_size-1);
		}
			
		inline reference back()
		{
			return Base::operator[](_size-1);
		}

		inline void push_back(const_reference element)
		{
			Base::operator[](_size) = element;
			++_size;
		}

		inline void conditional_push_back(bool condition,const_reference element)
		{
			Base::operator[](_size) = element;
			_size = condition ? _size +1 : _size;
		}

		inline void resize(size_t size) 
		{
			_size = size;;
		}
			
		inline void pop_back()
		{
			--_size;
		}

		size_t _size;
	};

	template<class _Base,class _Container,int _ContainerSize,int _Multiplier> struct ContainerMultiplier : public std::array<_Container,_Multiplier>
	{
		typedef std::array<_Container,_Multiplier> BaseClass;
		typedef typename std::array<_Container,_Multiplier>::size_type IndexType;

		inline ContainerMultiplier(const std::array<_Base,_ContainerSize*_Multiplier>& right)
		{
			std::array<_Base,_ContainerSize> subarray;

			for(int i = 0; i < _Multiplier; ++i)
			{			
				for(int j = 0; j < _ContainerSize; ++j)
					subarray[j] = right[i*_ContainerSize+j];

				BaseClass::operator[](i) = _Container(subarray);
			}
		}
	};

	template<class _C> struct ArrayAdapter
	{
		static const bool array = false;
		static const size_t Size = 1;
		typedef _C Array;
		typedef _C Element;
		typedef size_t IndexType;
		
		inline const Element& operator()(IndexType i,const Array& val) const
		{
			return val;
		}

		inline Element& operator()(IndexType i,Array& val) const
		{
			return val;
		}
	};

	template<class _C> struct EmptyArray{};
	
	template<class _C> struct ArrayAdapter<EmptyArray<_C>>
	{
		static const bool array = false;
		static const size_t Size = 1;
		typedef _C Array;
		typedef _C Element;
		typedef size_t IndexType;
		
		inline const Element& operator()(IndexType i,const Array& val) const
		{
			return val;
		}

		inline Element& operator()(IndexType i,Array& val) const
		{
			return val;
		}
	};

	template<class _Element,int _Size> struct ArrayAdapter<std::array<_Element,_Size>>
	{
		static const bool array = true;
		static const size_t Size = _Size;
		typedef _Element Element;
		typedef std::array<Element,Size> Array;
		typedef typename std::array<Element,Size>::size_type IndexType;
		
		inline const Element& operator()(IndexType i,const Array& val) const
		{
			return val[i];
		}

		inline Element& operator()(IndexType i,Array& val)
		{
			return val[i];
		}
	};

	// non recursive
	template<class _C> struct ArrayAdapter<ArrayAdapter<_C>> : public ArrayAdapter<_C>
	{
	};
	
	// array modifiers
	struct ArrayModifierNull
	{		
		template<int _Size> struct Size
		{
			static const int value = _Size;
		};
		
		template<class _Indextype> inline _Indextype operator()(_Indextype i) const
		{
			return i;
		}
	};
	
	template<int _Begin,int _End,int _Stagger> struct ArrayModifierSub
	{
		static_assert(_End == 0, "End really not supported yet");

		template<int _Size> struct Size
		{
			static const int value = (_Size - _Begin -1)/_Stagger +1;
		};
		
		template<class _Indextype> inline _Indextype operator()(_Indextype i) const
		{
			return _Begin+i*_Stagger;
		}
	};

	// element readers
	
	struct ReaderNull
	{ 
		template<class _C> struct Reader
		{
			inline _C& operator()(_C& val) const
			{
				return val;
			}

			inline const _C& operator()(const _C& val) const
			{
				return val;
			}
		};
	};

	namespace detail
	{

	// recursive array adapter
		template<class _C> struct RecursiveArrayAdapter : public RecursiveArrayAdapter<ArrayAdapter<_C>>
		{
		};

		template<class _C> struct RecursiveArrayAdapter<ArrayAdapter<_C>>
		{
			typedef ArrayAdapter<_C> OuterArray;
			typedef ArrayAdapter<typename OuterArray::Element> InnerArray;

			static const bool array = OuterArray::array;
			static const size_t Size = OuterArray::Size * InnerArray::Size;
			typedef typename InnerArray::Element Element;
			typedef typename OuterArray::Array Array;
			typedef typename InnerArray::IndexType IndexType;

			/*template<class _A,class _B> struct test
			{
				static_assert(std::is_same<_A,_B>::value, "pop");
			};*/
		
			inline const Element& operator()(IndexType i,const Array& val) const
			{
				//test<InnerArray,OuterArray> fail;
				return InnerArray()(i%InnerArray::Size,OuterArray()(i/InnerArray::Size,val));
			}

			inline Element& operator()(IndexType i,Array& val)
			{
				//test<InnerArray,OuterArray> fail;
				return InnerArray()(i%InnerArray::Size,OuterArray()(i/InnerArray::Size,val));
			}
		};

		template<class _C> struct RecursiveArrayAdapter<RecursiveArrayAdapter<_C>>
		{
			typedef RecursiveArrayAdapter<_C> OuterArray;
			typedef ArrayAdapter<typename OuterArray::Element> InnerArray;

			static const bool array = OuterArray::array;
			static const size_t Size = OuterArray::Size * InnerArray::Size;

			typedef typename InnerArray::Element Element;
			typedef typename OuterArray::Array Array;
			typedef typename InnerArray::IndexType IndexType;
		
			inline const Element& operator()(IndexType i,const Array& val) const
			{
				return InnerArray(i%InnerArray,OuterArray(i/InnerArray::Size,val));
			}

			inline Element& operator()(IndexType i,Array& val)
			{
				return InnerArray(i%InnerArray,OuterArray(i/InnerArray::Size,val));
			}
		};
	}
	
	template<class _C> struct ArrayAdapter<detail::RecursiveArrayAdapter<_C>> : public detail::RecursiveArrayAdapter<_C>
	{
	};

	template<class _Base,class _Container,int _ContainerSize,int _Multiplier> struct ArrayAdapter<ContainerMultiplier<_Base,_Container,_ContainerSize,_Multiplier>>
	{
		static const bool array = true;
		static const size_t Size = _Multiplier;
		typedef _Container Element;
		typedef ContainerMultiplier<_Base,_Container,_ContainerSize,_Multiplier> Array;
		typedef typename Array::IndexType IndexType;
		
		inline const Element& operator()(IndexType i,const Array& val) const
		{
			return val[i];
		}

		inline Element& operator()(IndexType i,Array& val) const
		{
			return val[i];
		}
	};

	/*
	Matrices are NOT arrays, this is a bad idea

	template<class _Element,int _Columns,int _Rows> struct ArrayAdapter<Eigen::Matrix<_Element,_Columns,_Rows>>
	{
		static const bool array = true;
		static const size_t Size = _Columns*_Rows;
		typedef _Element Element;
		typedef Eigen::Matrix<_Element,_Columns,_Rows> Array;
		typedef size_t IndexType;
		
		inline const Element& operator()(IndexType i,const Array& val) const
		{
			return val(i);
		}

		inline Element& operator()(IndexType i,Array& val) const
		{
			return val(i);
		}
	};
	*/
		
	namespace detail
	{

		template<class _SearchElement,int _SearchSize,class _CurArray,class _CurElement,int _CurSize,class _LastElement> struct ConditionalArrayAdapterImp
		{
			typedef typename ConditionalArrayAdapterImp<  _SearchElement, _SearchSize, RecursiveArrayAdapter<_CurArray>, typename RecursiveArrayAdapter<_CurArray>::Element, RecursiveArrayAdapter<_CurArray>::Size * _CurSize, _CurElement>::type type;
		};
			
		template<class _SearchElement,int _SearchSize,class _CurArray,class _LastElement>
			struct ConditionalArrayAdapterImp<_SearchElement,_SearchSize,_CurArray,_SearchElement,_SearchSize,_LastElement>
		{
			typedef _CurArray type;
		};

		template<class _SearchElement,int _SearchSize,class _CurArray,int _CurSize,class _LastElement> 
			struct ConditionalArrayAdapterImp<_SearchElement,_SearchSize,_CurArray,_SearchElement,_CurSize,_LastElement>
		{
			static_assert( _SearchSize == 0, "Found matching Array, but Size does not match");
			typedef _CurArray type;
		};
			
		template<class _SearchElement,int _SearchSize,class _CurArray,class _CurElement,class _LastElement> 
			struct ConditionalArrayAdapterImp<_SearchElement,_SearchSize,_CurArray,_CurElement,_SearchSize,_LastElement>
		{
			static_assert( std::is_same<_SearchElement,void>::value, "Found matching Size, but Type does not match");
			typedef _CurArray type;
		};
			
		template<class _SearchElement,int _SearchSize,class _CurArray,class _CurElement,int _CurSize> 
			struct ConditionalArrayAdapterImp<_SearchElement,_SearchSize,_CurArray,_CurElement,_CurSize,_CurElement>
		{
			static_assert( std::is_same<_SearchElement,void>::value && _SearchSize == 0, "End of array tree while searching for size or type");
			typedef _CurArray type;
		};

		template<class _Array,class _Element,int _Size> struct ConditionalArrayAdapter
		{
			typedef typename ConditionalArrayAdapterImp< _Element, _Size, ArrayAdapter<_Array>, typename ArrayAdapter<_Array>::Element, ArrayAdapter<_Array>::Size,_Array>::type type;
		};
		
		template<class _Array> struct ConditionalArrayAdapter<_Array,_Array,1>
		{
			typedef ArrayAdapter<EmptyArray<_Array>> type;
		};

		template<class _Array,int _Size> struct ConditionalArrayAdapter<_Array,_Array,_Size>
		{
			static_assert( _Size == 0, "Found matching Array, but Size does not match");
			typedef ArrayAdapter<EmptyArray<_Array>> type;
		};

		template<class> class result_of;

		template<class Fn, class Arg>
			struct result_of<Fn(Arg)> 
		{
		  typedef decltype(boost::declval<Fn>()(boost::declval<Arg>())) type;
		};

		template<class _ArrayAdapter,class _Reader = ReaderNull,class _Modifier = ArrayModifierNull> struct ArrayTransform
		{
			typedef typename _Modifier::Size<_ArrayAdapter::Size> ModifiedSize;
			static const size_t Size = ModifiedSize::value;
			
			typedef typename _ArrayAdapter::Element IntermediateElement;
			typedef typename _Reader::Reader<IntermediateElement> ReaderSpec;
			typedef typename std::remove_const< 
				typename std::remove_reference<
					typename result_of<ReaderSpec(const IntermediateElement&)>::type
				>::type
			>::type Element;
			typedef typename _ArrayAdapter::Array Array;

			typedef typename typename _ArrayAdapter::IndexType IndexType;
			
		
			inline const Element& operator()(IndexType i,const Array& val) const
			{
				return ReaderSpec()(_ArrayAdapter()(_Modifier()(i),val));
			}

			inline Element& operator()(IndexType i,Array& val)
			{
				return ReaderSpec()(_ArrayAdapter()(_Modifier()(i),val));
			}

			typedef Element Result;
		};

	}
	

	template<class _Array,class _Element = void,int _Count = 0,class _Reader = ReaderNull,class _Modifier = ArrayModifierNull,
		class _Adapter = detail::ArrayTransform<typename detail::ConditionalArrayAdapter<_Array,_Element,_Count>::type,_Reader,_Modifier> > struct ConstArrayWrapper
	{
		typedef _Adapter Adapter;
		typedef typename Adapter::Element Element;
		typedef typename Adapter::Array Array;
		typedef typename Adapter::IndexType IndexType;
		static const size_t Size = Adapter::Size;

		inline ConstArrayWrapper(const Array& data) : _val(data)
		{
		}
		
		inline const Element& operator [](IndexType i) const
		{
			return Adapter()(i,_val);
		}
		
		inline IndexType size() const
		{
			return Size;
		}

		const Array& _val;
	};
	
	template<class _Array,class _Element = void,int _Count = 0,class _Reader = ReaderNull,class _Modifier = ArrayModifierNull,
		class _Adapter = detail::ArrayTransform<typename detail::ConditionalArrayAdapter<_Array,_Element,_Count>::type,_Reader,_Modifier> > struct ArrayWrapper
	{
		typedef _Adapter Adapter;
		typedef typename Adapter::Element Element;
		typedef typename Adapter::Array Array;
		typedef typename Adapter::IndexType IndexType;
		static const size_t Size = Adapter::Size;

		inline ArrayWrapper(Array& data) : _val(data)
		{
		}

		inline Element& operator [](IndexType i)
		{
			return Adapter()(i,_val);
		}
		
		inline const Element& operator [](IndexType i) const
		{
			return Adapter()(i,_val);
		}
		
		inline IndexType size() const
		{
			return Size;
		}

		Array& _val;
	};

	
	template<class _Array,class _Element,int _Count,class _Reader,class _Modifier,class _Adapter> 
		struct ArrayAdapter<ConstArrayWrapper<_Array,_Element,_Count,_Reader,_Modifier,_Adapter>>
	{
		typedef ConstArrayWrapper<_Array,_Element,_Count,_Reader,_Modifier,_Adapter> Array;
		static const bool array = true;
		static const size_t Size = Array::Size;
		typedef typename Array::Element Element;
		typedef typename Array::IndexType IndexType;
		
		inline const Element& operator()(IndexType i,const Array& val) const
		{
			return val[i];
		}
	};
	
	template<class _Array,class _Element,int _Count,class _Reader,class _Modifier,class _Adapter> 
		struct ArrayAdapter<ArrayWrapper<_Array,_Element,_Count,_Reader,_Modifier,_Adapter>>
	{
		typedef ArrayWrapper<_Array,_Element,_Count,_Reader,_Modifier,_Adapter> Array;
		static const bool array = Array::array;
		static const size_t Size = Array::Size;
		typedef typename Array::Element Element;
		typedef typename Array::IndexType IndexType;
		
		inline const Element& operator()(IndexType i,const Array& val) const
		{
			return val[i];
		}

		inline Element& operator()(IndexType i,Array& val) const
		{
			return val[i];
		}
	};
		
	template<class _Element,int _Size> 
		struct ArrayAdapter<SimdType<_Element,_Size>>
	{
		typedef SimdType<_Element,_Size> Array;
		static const bool array = true;
		static const size_t Size = Array::Width;
		typedef typename Array::Base Element;
		typedef typename int IndexType;
		
		inline const Element& operator()(IndexType i,const Array& val) const
		{
			return val[i];
		}

		inline Element& operator()(IndexType i,Array& val) const
		{
			return val[i];
		}
	};

	template<class _Output> struct ConvertAoSToSoA
	{
		template<class _Input> inline void convert(const _Input& input) const
		{
		}
	};


	template<class _Element, int _Size> struct ConvertAoSToSoA<Eigen::Matrix<_Element,_Size,1>>
	{
		typedef Eigen::Matrix<_Element,_Size,1> Output;

		template<class _Input> struct RecursiveProcess
		{
			
			template<int _Index> struct ReadVectorElement
			{
				template<class _Signature> struct Reader
				{
				};

				template<class _Element, int _Size> struct Reader<Eigen::Matrix<_Element,_Size,1>>
				{
					static_assert(_Index < _Size,"Invalid Vector Index");

					inline const _Element& operator()(const Eigen::Matrix<_Element,_Size,1>& v) const
					{
						return v[_Index];
					}
				};
			};

			template<int _N> inline void recursivelySetElement(Output& output,const _Input& input) const
			{
				typedef ConstArrayWrapper<_Input,void,0,ReadVectorElement<_N>> InArray;
				typedef ArrayWrapper<_Element> OutArray;

				OutArray outArray(output[_N]);
				InArray inArray(input);

				for(auto i = 0; i < OutArray::Size; ++i)
					outArray[i] = inArray[i];

				recursivelySetElement<_N-1>(output,input);
			}

			template<> inline void recursivelySetElement<-1>(Output& output,const _Input& input) const
			{
			}

		};

		template<class _Input> inline Output operator()(const _Input& input) const
		{
			Output output;
			RecursiveProcess<_Input> process;
				
			process.recursivelySetElement<_Size-1>(output,input);
			return output;
		}
	};

}

#endif
