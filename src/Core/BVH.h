/********************************************************/
/* FILE: BVH.h                                   */
/* DESCRIPTION: Bounding Volume Hierarchy                   */
/* AUTHOR: Jan Schmid (jaschmid@eml.cc)                 */
/********************************************************/ 

#include <RaytraceCommon.h>
#include <vector>
#include <deque>
#include <array>
#include <limits>
#include <cmath>
#include <algorithm>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include "Triangle.h"
#include "AABB.h"
#include "BVHConstructor.h"
#include "ArrayAdapter.h"

#ifndef RAYTRACE_BVH_H_INCLUDED
#define RAYTRACE_BVH_H_INCLUDED

namespace Raytrace {
	

	template<class _Leaf = Triangle,class _Volume = AABB,int _LeafSize = 2,int _NodeSize = 2,class _LeafContainer = std::array<_Leaf,_LeafSize>,class _VolumeContainer = std::array<_Volume,_NodeSize>> struct BVH
	{
		static const int NodeSize = _NodeSize;
		static const int LeafSize = _LeafSize;
		
		typedef _Leaf													LeafItem;
		typedef _Volume													VolumeItem;
		
		typedef up														EncodedElement;

		typedef _VolumeContainer										VolumeElement;
		typedef _LeafContainer											LeafElement;
		typedef struct _TreeElement
		{
			template<class _Array> inline _TreeElement(const ConstArrayWrapper<_Array>& volumes) : _volumes(volumes) {}

			VolumeElement							_volumes;
			std::array<EncodedElement,NodeSize>		_children;
		}	TreeElement;

		
		typedef BVHConstructor<LeafItem,VolumeItem,NodeSize,LeafSize>		Constructor;
		


		friend struct nodeIterator;
		
		/*
		typedef u32 ChildNodeIndex;
		static const size_t CHILD_NODE_INDEX_SIZE = 2; // TO DO, GENERIC
		static const size_t CHILD_NODE_INDEX_MASK = (1 << CHILD_NODE_INDEX_SIZE) -1;

		typedef typename boost::uint_t<CHILD_NODE_INDEX_SIZE*NodeSize>::fast  OrderElement;

		static const size_t NUM_ORDER_ELEMENTS = 8; //8 possible sign combinations

		typedef std::array<OrderElement,NUM_ORDER_ELEMENTS> OrderHolder;*/


		inline BVH(Constructor& init)
		{
			init.constructFinal();

			size_t numLeafs = 0;
			size_t numNodes = 1; // root node!

			for(auto it = init._nodes.begin(); it != init._nodes.end(); ++it)
			{
				if(it->_numChildNodes > 0)
					numNodes ++;
				else
					numLeafs ++;
			}

			_numUsed = 0;
			_totalMem = numLeafs * sizeof(LeafElement) + numNodes * sizeof(TreeElement);
			_memory = _aligned_malloc( _totalMem, 64);

			_root = encodeConstructorNodeBreadthFirst( init._rootNode, init );

		}

		inline ~BVH()
		{
			_aligned_free(_memory);
		}
		/*
		struct traverseOrder
		{
			inline traverseOrder(const OrderElement& element) : _curr(element)
			{
			}

			inline ChildNodeIndex getNext()
			{
				int res = _curr & CHILD_NODE_INDEX_MASK;
				_curr >>= CHILD_NODE_INDEX_SIZE;
				return res;
			}

			OrderElement								_curr;
		};*/

		struct nodeIterator
		{
			inline nodeIterator(const nodeIterator& other) : _encoded(other._encoded)
			{
			}

			inline nodeIterator() {}

			inline nodeIterator& operator = (const nodeIterator& other)
			{
				_encoded = other._encoded;
				return *this;
			}

			inline operator bool() const
			{
				return valid();
			}

			inline bool valid() const
			{
				return BVH::isValid(_encoded);
			}

			inline const VolumeElement& volumes() const
			{
				RAY_ASSERT( !isLeaf() );
				return BVH::getNode(_encoded)._volumes;
			}
			/*
			inline traverseOrder getTraverseOrder(int raydir)
			{
				return traverseOrder(BVH::getNode(_data)._childOrder[raydir]);
			}*/

			inline bool isLeaf() const
			{
				return BVH::isLeaf(_encoded);
			}

			inline const LeafElement& leaf() const
			{
				RAY_ASSERT( isLeaf() );
				return BVH::getLeaf(_encoded);
			}

			inline nodeIterator node(/*ChildNodeIndex*/int index) const
			{
				RAY_ASSERT( index < NodeSize );
				RAY_ASSERT( !isLeaf() );
				//return nodeIterator(((const TreeElement*)_data)->_children[index]);
				return nodeIterator(BVH::getNode(_encoded)._children[index]);
			}

			inline void prefetch() const
			{
				_mm_prefetch( (char*)( &BVH::getNode(_encoded) ), _MM_HINT_T0 );
				//_mm_prefetch( (char*)( &BVH::getNode(_encoded) ) +64, _MM_HINT_T0 );
			}

			inline void prefetchChild(int index) const
			{
				_mm_prefetch( (char*)( &BVH::getNode(BVH::getNode(_encoded)._children[index]) ), _MM_HINT_T0 );
			}

		private:
			typedef typename BVH::EncodedElement EncodedElement;


			friend struct BVH;

			inline nodeIterator(const EncodedElement& element) : _encoded(element)
			{
			}

			EncodedElement	_encoded;
			/*
			i32				_leaf;
			const void*		_data;
			*/
		};
		
		inline nodeIterator root() const
		{
			return nodeIterator(_root);
		}

	private:
		static const EncodedElement LEAF_MASK = 0x00000001;
		static const EncodedElement INVALID_ELEMENT = 0;
		
		static inline bool isValid(EncodedElement index)
		{
			return index != INVALID_ELEMENT;
		}

		static inline bool isLeaf(EncodedElement index)
		{
			return (index & LEAF_MASK) != 0;
		}

		static inline const void* getData(EncodedElement index)
		{
			return (const void*)(index & (~LEAF_MASK));
		}
		
		static inline const TreeElement& getNode(EncodedElement index)
		{
			return *(const TreeElement*)(index);
		}

		static inline const LeafElement& getLeaf(EncodedElement index)
		{
			return *(const LeafElement*)(index & (~LEAF_MASK));
		}

		static inline EncodedElement encodeLeaf(void* element)
		{
			return ((up)element) | LEAF_MASK;
		}

		static inline EncodedElement encodeNode(void* element)
		{
			return ((up)element);
		}

		static inline EncodedElement encodeEmpty()
		{
			return (EncodedElement)INVALID_ELEMENT;
		}

		/*
				Volume						_bound;
				size_t						_childItemBegin;
				size_t						_childItemEnd;
				size_t						_numChildNodes;
				std::array<size_t,NodeSize>	_childNodes;
		*/

		inline EncodedElement encodeConstructorNodeBreadthFirst(const typename Constructor::Node& root,const Constructor& constructor)
		{
			typedef std::pair<const typename Constructor::Node*,EncodedElement*> tempNode;
			std::deque<tempNode> nodeStack;
			nodeStack.push_back(tempNode(&root,nullptr));
			EncodedElement result;

			while(!nodeStack.empty())
			{
				const typename Constructor::Node& node = *nodeStack.front().first;
				EncodedElement* parent = nodeStack.front().second;
				nodeStack.pop_front();

				
				if(node._numChildNodes)
				{
					//node
					TreeElement* element = (TreeElement*)getMemory(sizeof(TreeElement));

					std::array<VolumeItem,NodeSize> subvolumes;

					for(size_t i = 0; i < node._numChildNodes; ++i)
						subvolumes[i] = constructor._nodes[ node._childNodes[i] ]._bound;
					for(size_t i = node._numChildNodes; i < NodeSize; ++i)
						subvolumes[i] = VolumeItem::Empty();

					new (element) TreeElement( ConstArrayWrapper<std::array<VolumeItem,NodeSize>>(subvolumes) );
						
					for(size_t i = 0; i < node._numChildNodes; ++i)
						nodeStack.push_back(tempNode(&constructor._nodes[ node._childNodes[i] ],&element->_children[i]));
					for(size_t i = node._numChildNodes; i < NodeSize; ++i)
						element->_children[i] = encodeEmpty();

					if(parent)
						*parent = encodeNode(element);
					else
						result = encodeNode(element);
				}
				else
				{
					//leaf
					LeafElement* element = (LeafElement*)getMemory(sizeof(LeafElement));

					std::array<LeafItem,LeafSize> leafs;

					for(size_t i = 0; i < node._childItemEnd-node._childItemBegin; ++i)
						leafs[i] = constructor._sortedItems[i+node._childItemBegin]->_item;
					for(size_t i = node._childItemEnd-node._childItemBegin; i < LeafSize; ++i)
						leafs[i] = LeafItem::Empty();

					new (element) LeafElement( ConstArrayWrapper<std::array<LeafItem,LeafSize>>(leafs) );

					if(parent)
						*parent = encodeLeaf(element);
					else
						result = encodeLeaf(element);
				}
			}
			return result;
		}

		inline EncodedElement encodeConstructorNodeDepthFirst(const typename Constructor::Node& root,const Constructor& constructor)
		{
			if(node._numChildNodes)
			{
				//node
				TreeElement* element = (TreeElement*)getMemory(sizeof(TreeElement));
				
				std::array<VolumeItem,NodeSize> subvolumes;

				for(size_t i = 0; i < node._numChildNodes; ++i)
					subvolumes[i] = constructor._nodes[ node._childNodes[i] ]._bound;
				for(size_t i = node._numChildNodes; i < NodeSize; ++i)
					subvolumes[i] = VolumeItem::Empty()

				new (element) TreeElement(subvolumes);
						
				for(size_t i = 0; i < node._numChildNodes; ++i)
					element->_children[i] = encodeConstructorNode( constructor._nodes[ node._childNodes[i] ], constructor);
				for(size_t i = node._numChildNodes; i < NodeSize; ++i)
					element->_children[i] = encodeEmpty();
				
				//generate traverse order
				/*
				for(int i = 0; i < NUM_ORDER_ELEMENTS; ++i)
				{
					std::array<int,NodeSize> order = {0,1,2,3};
					element->_childOrder[i] = 0;

					for(int j = 0; j < NodeSize; ++j)
						element->_childOrder[i] |= order[j] << (CHILD_NODE_INDEX_SIZE * j);
				}*/

				return encodeNode(element);
			}
			else
			{
				//leaf
				LeafElement* element = (LeafElement*)getMemory(sizeof(LeafElement));
				
				std::array<LeafItem,LeafSize> leafs;

				for(size_t i = 0; i < node._childItemEnd-node._childItemBegin; ++i)
					leafs[i] = constructor._sortedItems[i+node._childItemBegin]->_item;
				for(size_t i = node._childItemEnd-node._childItemBegin; i < LeafSize; ++i)
					leafs[i] = LeafItem::Empty();
				
				new (element) LeafElement(leafs);

				return encodeLeaf(element);
			}
		}

		inline void* getMemory(up size)
		{
			void* result = &((u8*)_memory)[_numUsed];
			_numUsed += size;
			assert(_numUsed <= _totalMem);
			return result;
		}

		// this is actually the only data we need at runtime
		EncodedElement					_root;
		up								_numUsed;
		up								_totalMem;
		void*							_memory;

		/*
		std::vector<LeafElement,AlignedAllocator<LeafElement>>		_leaf;
		std::vector<TreeElement,AlignedAllocator<TreeElement>>		_node;*/
	};

}

#endif