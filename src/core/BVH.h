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

#ifndef RAYTRACE_BVH_H_INCLUDED
#define RAYTRACE_BVH_H_INCLUDED

namespace Raytrace {

#ifdef _DEBUG
	inline void RAY_ASSERT(bool val) { RAY_ASSERT(val); }
#else
	inline void RAY_ASSERT(bool val) {}
#endif

	struct DefaultLeafContainer
	{
		template<class _Contents,int _Size> struct data
		{
			typedef typename std::array<_Contents,_Size> type;
		};
	};

	template<class _Leaf,int _NodeSize = 4,int _LeafSize = 4,class _LeafContainer = DefaultLeafContainer> struct BVH
	{
		static const int NodeSize = _NodeSize;
		static const int LeafSize = _LeafSize;
		
		typedef AABB		Volume;
		typedef typename _LeafContainer::data<_Leaf,_LeafSize>::type LeafElement;
		

		struct Constructor
		{
		public:
			typedef typename BVH::LeafItem LeafItem;
			typedef typename BVH::Volume Volume;

			inline Constructor(const size_t binSize) : _binSize(binSize) 
			{
				_bins.resize( _binSize );
			}

			~Constructor(){}

			inline void addElement(const LeafItem& element,const Vector3& centroid,const Volume& volume)
			{
				ConstructionItem item;
				item._item = element;
				item._centroid = centroid;
				item._volume = Volume(volume,centroid);
				_items.push_back(item);
			}
			
		private:

			friend struct BVH;

			void constructFinal()
			{
				if(_sortedItems.size() != 0)
					return;
				_sortedItems.resize(_items.size());

				Volume rootVolume = Volume::Empty();
				
				for(auto it = _items.begin(); it != _items.end(); ++it)
				{
					rootVolume = Volume(rootVolume,it->_volume);
					_sortedItems[it - _items.begin()] = &*it;
				}


				_rootNode._childItemBegin = 0;
				_rootNode._childItemEnd = _sortedItems.size();
				_rootNode._bound = rootVolume;
				makeMultiNode(_rootNode);
			}

			struct ConstructionItem
			{
				LeafItem			_item;
				Vector3				_centroid;
				Volume				_volume;
			};

			struct Bin
			{
				size_t	_itemCount;
				AABB	_bound;
			};

			struct Node
			{
				Node() : _bound(Volume::Empty()),_childItemBegin(0),_childItemEnd(0),_numChildNodes(0)
				{
					for(auto it = _childNodes.begin(); it != _childNodes.end(); ++it)
						*it = -1;
				}
				Volume						_bound;
				size_t						_childItemBegin;
				size_t						_childItemEnd;
				size_t						_numChildNodes;
				std::array<size_t,NodeSize>	_childNodes;
			};

			void makeMultiNode(Node& multi)
			{
				if(!nodeWantsToSplit(multi))
					return;

				Node child(multi);

				int childId = _nodes.size();
				_nodes.push_back(child);

				multi._childItemBegin = 0;
				multi._childItemEnd = 0;
				multi._numChildNodes = 1;
				multi._childNodes[0] = childId;

				while(multi._numChildNodes < NodeSize)
				{
					size_t splitChild = findSplitChild(multi);

					if(splitChild == -1)
						break;
					else
					{
						multi._childNodes[multi._numChildNodes] = splitNode( _nodes[multi._childNodes[splitChild]] );
						multi._numChildNodes++;
					}
				}

				for(size_t i = 0; i < multi._numChildNodes; ++i)
					makeMultiNode( _nodes[multi._childNodes[i]] );

			}

			inline size_t findSplitChild(const Node& parent) const
			{
				Real sah = -std::numeric_limits<Real>::infinity();
				size_t index = -1;

				for(size_t i = 0; i < parent._numChildNodes; ++i)
				{
					const Node& childNode = _nodes[ parent._childNodes[i] ];
					if( nodeWantsToSplit( childNode ) )
					{
						Real sah2 = childNode._bound.SAH();
						if(sah2 > sah)
						{
							sah = sah2;
							index = i;
						}
					}
				}
				return index;
			}

			size_t splitNode(Node& node1)
			{
				size_t nodeId2 = _nodes.size();
				_nodes.push_back(Node());

				Node& node2 = _nodes[nodeId2];

				size_t itemBegin = node1._childItemBegin;
				size_t itemEnd = node1._childItemEnd;

				Volume centroidVolume = Volume::Empty();

				for(size_t i = itemBegin; i < itemEnd; ++i)
					centroidVolume = Volume( centroidVolume, _sortedItems[i]->_centroid );
				
				node1._bound = Volume::Empty();
				node1._childItemBegin = 0;
				node1._childItemEnd = 0;

				projectItemsToBins(itemBegin,itemEnd,centroidVolume);

				Real sah = std::numeric_limits<Real>::infinity();
				Volume bestLeft = Volume::Empty(),bestRight = Volume::Empty();
				size_t split = -1;

				for(size_t i = 1; i < _bins.size(); ++ i)
				{
					Volume left = Volume::Empty(),right = Volume::Empty();

					for(size_t iLeft = 0; iLeft < i; ++ iLeft)
						left = Volume(left,_bins[iLeft]._bound);
					for(size_t iRight = i; iRight < _bins.size(); ++ iRight)
						right = Volume(right,_bins[iRight]._bound);

					if(left.isEmpty() || right.isEmpty())
						continue;

					if(left.SAH() + right.SAH() < sah)
					{
						sah = left.SAH() + right.SAH();
						bestLeft = left;
						bestRight = right;
						split = i;
					}
				}

				RAY_ASSERT(split != -1);

				sortItems(node1,node2,bestLeft,bestRight,itemBegin,itemEnd);

				return nodeId2;
			}

			inline void sortItems(Node& node1,Node& node2,const Volume& leftCentroid,const Volume& rightCentroid,size_t itemBegin, size_t itemEnd)
			{
				int iLeft = itemBegin, iRight = itemEnd - 1;

				while(iLeft < iRight)
				{
					while(leftCentroid.contains( _sortedItems[iLeft]->_centroid ) && iLeft < iRight)
					{
						node1._bound = Volume(node1._bound,_sortedItems[iLeft]->_volume);
						iLeft++;
					}
					
					while(rightCentroid.contains( _sortedItems[iRight]->_centroid ) && iLeft < iRight)
					{
						node2._bound = Volume(node2._bound,_sortedItems[iRight]->_volume);
						iRight--;
					}

					if(iLeft < iRight)
					{
						ConstructionItem* t = _sortedItems[iRight];
						_sortedItems[iRight] = _sortedItems[iLeft];
						_sortedItems[iLeft] = t;
					}
				}

				node1._childItemBegin = itemBegin;
				node1._childItemEnd = iLeft;
				node2._childItemBegin = iLeft;
				node2._childItemEnd = itemEnd;
				node2._bound = Volume(node2._bound,_sortedItems[iLeft]->_volume);
			}

			static inline bool nodeWantsToSplit(const Node& node)
			{
				if(node._childItemEnd - node._childItemBegin > NodeSize )
					return true;
				else
					return false;
			}

			inline size_t projectItemsToBins(size_t itemBegin,size_t itemEnd,const Volume& totalVolume)
			{
				static const Real EPSILON = .00001f;
				//reset bins
				for(auto it = _bins.begin(); it != _bins.end(); ++it)
				{
					it->_itemCount = 0;
					it->_bound = Volume::Empty();
				}

				//find dominant axis
				size_t dominant = 0;
				Real dominantSize = totalVolume.max()[dominant] - totalVolume.min()[dominant];
				for(size_t i = 1; i < 3; ++i)
				{
					Real size = totalVolume.max()[i] - totalVolume.min()[i];
					if(size > dominantSize)
					{
						dominant = i;
						dominantSize = size;
					}
				}

				Real k = (Real)_binSize*(1.0f-EPSILON)/(dominantSize);

				for(size_t i = itemBegin; i < itemEnd; ++i)
				{
					size_t binIndex = (size_t)(k*(_sortedItems[i]->_centroid[dominant] - totalVolume.min()[dominant]));
					_bins[binIndex]._bound = Volume(_bins[binIndex]._bound, _sortedItems[i]->_centroid);
					_bins[binIndex]._itemCount ++;
				}
				return dominant;
			}

			const size_t					_binSize;
			Node							_rootNode;
			std::deque<Node>				_nodes;
			std::vector<Bin>				_bins;
			std::vector<ConstructionItem*>	_sortedItems;
			std::vector<ConstructionItem>	_items;
		};

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
			_leaf.resize( numLeafs );
			_node.resize( numNodes );
			_volume.resize( numLeafs + numNodes );

			size_t nLeaf = 0;
			size_t nNode = 0;

			_root = encodeConstructorNode( init._rootNode, init, nLeaf, nNode );

			RAY_ASSERT(nLeaf == _leaf.size());
			RAY_ASSERT(nNode == _node.size());
		}

		inline ~BVH()
		{
		}

		struct nodeIterator
		{
			inline nodeIterator(const nodeIterator& other) : _contents(other._contents),_bvh(other._bvh)
			{
			}

			inline nodeIterator() : _data(nullptr,nullptr,0),_bvh(nullptr) {}

			inline nodeIterator& operator = (const nodeIterator& other)
			{
				_data = other._data;
				_bvh = other._bvh;
				return *this;
			}

			inline operator bool() const
			{
				return valid();
			}

			inline bool valid() const
			{
				if(_data._data)
					return true;
				else
					return false;
			}

			inline const Volume& volume() const
			{
				RAY_ASSERT(valid());
				return *_data._volume;
			}

			inline bool leaf() const
			{
				return _data._leafSize > 0;
			}

			inline const LeafElement* leaf(int& size) const
			{
				size = _data._leafSize;
				if(size)
					return ((const LeafElement*)_data._data);
				else
					return nullptr;
			}

			inline nodeIterator node(int index) const
			{
				if(_data._leafSize)
					return nodeIterator();
				else
					return nodeIterator(*_bvh,(*((TreeElement*)_data._data))[index]);
			}

		private:
			typedef typename BVH::encodedElement encodedElement;
			typedef typename BVH::decodedElement decodedElement;


			friend struct BVH;

			inline nodeIterator(const BVH& bvh,const encodedElement& element) : _data( bvh.decodeElement( element) ), _bvh(&bvh) {}

			decodedElement		_data;
			const BVH*			_bvh;
		};
		
		inline nodeIterator root() const
		{
			return nodeIterator(*this,_root);
		}

	private:
		
		typedef u32														encodedElement;
		typedef std::array<encodedElement,NodeSize>						TreeElement;
		typedef _Leaf													LeafItem;

		typedef struct _decodedElement
		{
			inline _decodedElement(const Volume* volume,const void* data,int leafSize) :_volume(volume),_data(data),_leafSize(leafSize) {}

			const Volume*	_volume;
			const void*		_data;
			int				_leafSize;
		} decodedElement;


		friend struct nodeIterator;
		static const int IndexOffset = 8;
		static const int LeafSizeMask = (1<<IndexOffset) -1;
		static const encodedElement EmptyElement = -1;

		inline decodedElement decodeElement(const encodedElement& index) const
		{
			if(index == EmptyElement)
				return decodedElement(0,0,0);
			int leafSize = index & LeafSizeMask;
			int realIndex = index >> IndexOffset;
			const Volume* volume = &_volume[leafSize?realIndex+_node.size():realIndex];
			const void* data = (leafSize>0)?((const void*)&(_leaf[realIndex])):((const void*)&(_node[realIndex]));
			return decodedElement(volume,data,leafSize);
		}

		inline encodedElement encodeLeaf(u32 size,u32 index) const
		{
			RAY_ASSERT(size != 0);
			RAY_ASSERT(index < _leaf.size());
			RAY_ASSERT(size <= LeafSize);
			return (encodedElement)(size | (index << IndexOffset));
		}

		inline encodedElement encodeNode(u32 index) const
		{
			RAY_ASSERT(index < _node.size());
			return (encodedElement)(index << IndexOffset);
		}

		inline encodedElement encodeEmpty() const
		{
			return (encodedElement)EmptyElement;
		}

		/*
				Volume						_bound;
				size_t						_childItemBegin;
				size_t						_childItemEnd;
				size_t						_numChildNodes;
				std::array<size_t,NodeSize>	_childNodes;
		*/

		inline encodedElement encodeConstructorNode(const typename Constructor::Node& node,const Constructor& constructor,size_t& iLeaf,size_t& iNode)
		{
			if(node._numChildNodes)
			{
				//node
				size_t write = iNode;
				++iNode;
				for(size_t i = 0; i < node._numChildNodes; ++i)
					_node[write][i] = encodeConstructorNode( constructor._nodes[ node._childNodes[i] ], constructor, iLeaf, iNode);
				for(size_t i = node._numChildNodes; i < NodeSize; ++i)
					_node[write][i] = encodeEmpty();
				_volume[write] = node._bound;
				return encodeNode(write);
			}
			else
			{
				//leaf
				size_t write = iLeaf;
				++iLeaf;
				for(size_t i = node._childItemBegin; i < node._childItemEnd; ++i)
					_leaf[write][i - node._childItemBegin] = constructor._sortedItems[i]->_item;
				_volume[write + _node.size()] = node._bound;
				return encodeLeaf( node._childItemEnd - node._childItemBegin, write);
			}
		}

		// this is actually the only data we need at runtime
		encodedElement					_root;
		std::vector<Volume>				_volume; // volumes list nodes first then leafs
		std::vector<LeafElement>		_leaf;
		std::vector<TreeElement>		_node;
	};

}

#endif