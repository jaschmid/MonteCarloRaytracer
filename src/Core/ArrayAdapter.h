/********************************************************/
// FILE: ArrayAdapter.h
// DESCRIPTION: array adapter that allows generic usage of static arrays of any kind
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

#ifndef RAYTRACE_ARRAY_ADAPTER_GUARD
#define RAYTRACE_ARRAY_ADAPTER_GUARD

#include <RaytraceCommon.h>
#include <array>

namespace Raytrace {
	
	template<class _Base,int _Width> struct SimdType;
	

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
			typedef _C result;

			inline _C& operator()(_C& val) const
			{
				return val;
			}

			inline const _C& operator()(const _C& val) const
			{
				return val;
			}
		};

		template<class _Signature> struct get
		{
			typedef Reader<_Signature> type;
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
			typedef typename OuterArray::IndexType IndexTypeOuter;
			typedef typename InnerArray::IndexType IndexType;

			/*template<class _A,class _B> struct test
			{
				static_assert(std::is_same<_A,_B>::value, "pop");
			};*/
		
			inline const Element& operator()(IndexType i,const Array& val) const
			{
				//test<InnerArray,OuterArray> fail;
				IndexTypeOuter i_outer = ((IndexTypeOuter)i)/((IndexTypeOuter)InnerArray::Size);
				IndexType i_inner = ((IndexType)i)%((IndexType)InnerArray::Size);
				return InnerArray()(i_inner,OuterArray()(i_outer,val));
			}

			inline Element& operator()(IndexType i,Array& val)
			{
				//test<InnerArray,OuterArray> fail;
				IndexTypeOuter i_outer = ((IndexTypeOuter)i)/((IndexTypeOuter)InnerArray::Size);
				IndexType i_inner = ((IndexType)i)%((IndexType)InnerArray::Size);
				return InnerArray()(i_inner,OuterArray()(i_outer,val));
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
				return InnerArray()(i%InnerArray::Size,OuterArray()(i/InnerArray::Size,val));
			}

			inline Element& operator()(IndexType i,Array& val)
			{
				return InnerArray()(i%InnerArray::Size,OuterArray()(i/InnerArray::Size,val));
			}
		};

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

		template<class Fn, class Arg> struct result_of
		{
			typedef decltype( boost::declval<Fn>()( boost::declval<Arg>() ) ) type;
		};

		template<class _ArrayAdapter,class _Reader = ReaderNull,class _Modifier = ArrayModifierNull> struct ArrayTransform
		{
			typedef typename _Modifier::template Size<_ArrayAdapter::Size> ModifiedSize;
			static const size_t Size = ModifiedSize::value;
			
			typedef typename _ArrayAdapter::Element IntermediateElement;
			typedef typename _Reader::template get<IntermediateElement>::type ReaderSpec;
			typedef typename std::remove_const< 
				typename std::remove_reference<
					typename ReaderSpec::result
				>::type
			>::type Element;
			typedef typename _ArrayAdapter::Array Array;

			typedef typename _ArrayAdapter::IndexType IndexType;
			
		
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
	
	
	
	template<class _C> struct ArrayAdapter<detail::RecursiveArrayAdapter<_C>> : public detail::RecursiveArrayAdapter<_C>
	{
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
	};*/


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
		
		inline operator Element() const
		{
			static_assert(Size == 1,"can only cast to single element if array of size 1");
			return Adapter()(0,_val);
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
		
		inline operator Element() const
		{
			static_assert(Size == 1,"can only cast to single element if array of size 1");
			return Adapter()(0,_val);
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
		typedef int IndexType;
		
		inline const Element& operator()(IndexType i,const Array& val) const
		{
			return val[i];
		}

		inline Element& operator()(IndexType i,Array& val) const
		{
			return val[i];
		}
	};
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

	template<class _Output> struct ConvertAoSToSoA
	{
		template<class _Input> inline _Output operator()(const _Input& input) const
		{
			return _Output(input[0]);
		}
	};

	
	namespace detail
	{
		// Signature
		template<class _Signature,int _Index> struct ReadVectorElement;

		template<class _Element,int _Size,int _Index> struct ReadVectorElement<Eigen::Matrix<_Element,_Size,1>,_Index>
		{
			
			static_assert(_Index < _Size,"Invalid Vector Index");

			typedef _Element result;

			inline const _Element& operator()(const Eigen::Matrix<_Element,_Size,1>& v) const
			{
				return v[_Index];
			}

			inline _Element& operator()(Eigen::Matrix<_Element,_Size,1>& v) const
			{
				return v[_Index];
			}
		};

		template<int _Index> struct VectorReader
		{
			template<class _Signature> struct get
			{
				typedef ReadVectorElement<_Signature,_Index> type;
			};
		};
	

		template<class _Element, int _Size,class _Input,class _InputElement,int _Dimension> struct RecursiveProcess
		{
			typedef Eigen::Matrix<_Element,_Size,1> Output;
			inline void operator()(Output& output,const _Input& input) const
			{
				typedef ArrayWrapper<_Element> OutArray;
				typedef ConstArrayWrapper<typename _Input::Element,void,0,VectorReader<_Dimension>> InArray;

				OutArray outArray(output[_Dimension]);

				for(auto i = 0; i < OutArray::Size; ++i)
					outArray[i] = typename OutArray::Element(InArray(input[i])[0]);

				RecursiveProcess<_Element,_Size,_Input,_InputElement,_Dimension-1>()(output,input);
			}
		};
		
		template<class _Element, int _Size,class _Input,class _InputElement> struct RecursiveProcess<_Element,_Size,_Input,_InputElement,-1>
		{
			typedef Eigen::Matrix<_Element,_Size,1> Output;
			inline void operator()(Output& output,const _Input& input) const
			{
			}
		};
		
		template<class _Element, int _Size,class _Input,int _N> struct RecursiveProcess<_Element,_Size,_Input,Eigen::Matrix<_Element,_Size,1>,_N>
		{	
			typedef Eigen::Matrix<_Element,_Size,1> Output;
			inline void operator()(Output& output,const _Input& input) const
			{
				output = input[0];
			}
		};
	}

	template<class _Element, int _Size> struct ConvertAoSToSoA<Eigen::Matrix<_Element,_Size,1>>
	{
		typedef Eigen::Matrix<_Element,_Size,1> Output;

		template<class _Input> inline Output operator()(const _Input& input) const
		{
			Output output;				
			detail::RecursiveProcess<_Element,_Size,_Input,typename _Input::Element,_Size-1>()(output,input);
			/*for(int id = 0; id < _Size; id++)
				for(int ia = 0; ia < _Input::Size; ia++)
					output[id][ia] = input[ia][id];*/
			return output;
		}
	};

	
	template<class _Base,class _Container,int _ContainerSize,int _Multiplier> struct ContainerMultiplier : public std::array<_Container,_Multiplier>
	{
		typedef std::array<_Container,_Multiplier> BaseClass;
		typedef typename std::array<_Container,_Multiplier>::size_type IndexType;

		template<class _Array> inline ContainerMultiplier(const ConstArrayWrapper<_Array>& right)
		{
			std::array<_Base,_ContainerSize> subarray;

			for(int i = 0; i < _Multiplier; ++i)
			{			
				for(int j = 0; j < _ContainerSize; ++j)
					subarray[j] = right[i*_ContainerSize+j];

				BaseClass::operator[](i) = _Container(ConstArrayWrapper<std::array<_Base,_ContainerSize>>(subarray));
			}
		}
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
}

#endif