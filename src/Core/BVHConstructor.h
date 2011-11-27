/********************************************************/
/* FILE: BVH.h											*/
/* DESCRIPTION: Bounding Volume Hierarchy Constructor   */
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

#include "AABB.h"

#ifndef RAYTRACE_BVH_CONSTRUCTOR_H_INCLUDED
#define RAYTRACE_BVH_CONSTRUCTOR_H_INCLUDED

namespace Raytrace {

#ifdef _DEBUG
	inline void RAY_ASSERT(bool val) { assert(val); }
#else
	inline void RAY_ASSERT(bool val) {}
#endif


	template<class _Leaf,class _Volume,int _NodeSize,int _LeafSize> struct BVHConstructor
	{
	public:
		typedef _Leaf				LeafItem;
		typedef _Volume				Volume;
		static const int NodeSize = _NodeSize;
		static const int LeafSize = _LeafSize;

		inline BVHConstructor(size_t binSize) : _binSize(binSize)
		{
			_bins.resize( _binSize );
		}

		~BVHConstructor(){}

		inline void addElement(const LeafItem& element,const Vector3& centroid,const Volume& volume)
		{
			ConstructionItem item;
			item._item = element;
			item._centroid = centroid;
			item._volume = Volume(volume,centroid);
			_items.push_back(item);
		}
			
	private:

		template<class _Leaf,class _Volume,int _NodeSize,int _LeafSize,class _LeafContainer,class _VolumeContainer> friend struct BVH;

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
			_rootNode._splitDirection = -1;
			makeMultiNode(_rootNode);
			
			size_t numLeafs=0,numNodes=0;
			float leafUtilization = 0.0f,nodeUtilization = 0.0f;

			for(auto it = _nodes.begin(); it != _nodes.end(); ++it)
			{
				if(it->_numChildNodes)
				{
					nodeUtilization += (float)(it->_numChildNodes);
					++numNodes;
				}
				else
				{
					leafUtilization += (float)(it->_childItemEnd - it->_childItemBegin);
					++numLeafs;
				}
			}

			nodeUtilization /= (float)numNodes;
			leafUtilization /= (float)numLeafs;

			FILE* log = fopen("log.txt","w");

			fprintf(log,"%u Nodes, %.2f full out of %u\n",numNodes,nodeUtilization,NodeSize);
			fprintf(log,"%u Leaves, %.2f full out of %u\n",numLeafs,leafUtilization,LeafSize);

			fclose(log);
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
			Volume	_centroidBound;
			Volume	_centroidLeft;
			Volume	_centroidRight;
			Volume	_bound;
			Volume	_boundLeft;
			Volume	_boundRight;
		};

		struct Node
		{
			Node() : _bound(Volume::Empty()),_childItemBegin(0),_childItemEnd(0),_numChildNodes(0)
			{
				for(auto it = _childNodes.begin(); it != _childNodes.end(); ++it)
					*it = -1;
			}
			Volume						_bound;
			size_t						_splitDirection;
			float						_splitSAH;
			size_t						_childSplit;

			size_t						_childItemBegin;
			size_t						_childItemEnd;
			size_t						_numChildNodes;
			std::array<size_t,NodeSize>	_childNodes;
		};

		void makeMultiNode(Node& multi)
		{
			if(!nodeWantsToSplit(multi))
				return;

			Node child = multi;
			child._numChildNodes = 0;
			initializeLeaf(child,child._childItemBegin,child._childItemEnd);

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

			assert(multi._childItemEnd == 0 && multi._childItemBegin == 0);

		}

		inline size_t findSplitChild(const Node& parent) const
		{
			size_t index = -1;

			float sah = -std::numeric_limits<Real>::infinity();

			for(size_t i = 0; i < parent._numChildNodes; ++i)
			{
				const Node& childNode = _nodes[ parent._childNodes[i] ];
				if( nodeWantsToSplit( childNode ) )
				{
					Real sah2 = calculateSAH(childNode._bound, childNode._childItemEnd - childNode._childItemBegin) / childNode._splitSAH;
					if(sah2 > sah || index == -1)
					{
						sah = sah2;
						index = i;
					}
				}
			}
			return index;
		}
					
		static inline float getIntersectionCost(size_t items)
		{
			float adjustment = 1.0f;
			/*
			if(items < LeafSize)
				adjustment = 10.0f;
				*/
			static const float LeafCost = LeafSize;
			static const float NodeCost = NodeSize;

			size_t used_slots = 0;
			size_t wasted_slots = 0;
			size_t leaf_depth = 0;
			size_t leaf_count = 0;
			size_t node_depth = 0;
			size_t node_count = 0;
			size_t num_containers = 0;
			float cost,usage;
			

			//number of slots in leaves that are empty
			num_containers = (items - 1)/LeafSize+1;
			usage = (float)items / (float)(num_containers*LeafSize - items);
			cost = LeafCost;

			//items = number of leaves
			items = num_containers;
			leaf_depth ++;
			leaf_count  += num_containers;

			while(items > 1)
			{
				num_containers = (items - 1)/NodeSize+1;
				usage *= (float)items / (float)(num_containers*NodeSize - items);
				cost = NodeCost + cost;

				items = num_containers;
				node_depth ++;
				node_count+=num_containers;
			}

			return adjustment*(leaf_count*LeafCost + node_count*NodeCost);
		}

		inline static float calculateSAH(const Volume& volume,size_t numItems)
		{
			/*
			if(numItems < LeafSize)
				return std::numeric_limits<Real>::infinity();*/
			return volume.SAH()*getIntersectionCost(numItems);
		}


		bool findBestSplitPlane(size_t dimension,size_t itemBegin,size_t itemEnd,const Volume& centroidVolume,float& newSah,Volume& leftCentroid,Volume& rightCentroid)
		{
			leftCentroid= centroidVolume;
			rightCentroid = centroidVolume;
			newSah = std::numeric_limits<Real>::infinity();

			if(centroidVolume.max()[dimension] -centroidVolume.min()[dimension] <= 0.0f)
				return false;

			projectItemsToBins(itemBegin,itemEnd,centroidVolume,dimension);
				
			size_t left_count = _bins[0]._itemCount,right_count = itemEnd-itemBegin - _bins[0]._itemCount;

			size_t split = -1;

			for(size_t i = 1; i < _bins.size(); ++ i)
			{

				if(!_bins[i-1]._centroidLeft.isEmpty() && ! _bins[i]._centroidRight.isEmpty())
				{
					float thisSAH = calculateSAH(_bins[i-1]._boundLeft,left_count) + calculateSAH(_bins[i]._boundRight,right_count);

					if(thisSAH < newSah)
					{
						split = i;
						newSah = thisSAH;
						leftCentroid = _bins[i-1]._centroidLeft;
						rightCentroid = _bins[i]._centroidRight;
					}
				}

				left_count+= _bins[i]._itemCount;
				right_count-= _bins[i]._itemCount;
			}

			assert(right_count == 0);

			return true;
		}
		
		void initializeLeaf(Node& node,size_t itemBegin,size_t itemEnd)
		{
			assert(node._numChildNodes == 0);

			Volume centroidVolume = Volume::Empty();
			Volume totalVolume = Volume::Empty();
			for(size_t i = itemBegin; i < itemEnd; ++i)
			{
				centroidVolume = Volume( centroidVolume, _sortedItems[i]->_centroid );
				totalVolume = Volume( totalVolume, _sortedItems[i]->_volume );
			}

			float newSah;
			//float dominantSize = centroidVolume.size
			size_t dominant = 0;
			Volume leftCentroid,rightCentroid;

			bool valid = findBestSplitPlane(dominant,itemBegin,itemEnd,centroidVolume,newSah,leftCentroid,rightCentroid);

			for(int i = 1; i < 3; ++i)
			{
				float newSah2;
				Volume leftCentroid2,rightCentroid2;

				if(findBestSplitPlane(i,itemBegin,itemEnd,centroidVolume,newSah2,leftCentroid2,rightCentroid2) && newSah2 < newSah)
				{
					newSah = newSah2;
					leftCentroid = leftCentroid2;
					rightCentroid = rightCentroid2;
					dominant = i;
				}
			}

			if(!valid || (itemEnd-itemBegin) < LeafSize )
			{
				node._childItemBegin = itemBegin;
				node._childItemEnd = itemEnd;
				node._bound = totalVolume;
				node._splitDirection = dominant;
				node._childSplit = (itemEnd+itemBegin)/2;
				node._splitSAH =	std::numeric_limits<Real>::infinity();
				node._numChildNodes = 0;
				return;
			}
			
			int iLeft = itemBegin, iRight = itemEnd - 1;

			while(iLeft < iRight)
			{
				while(leftCentroid.contains( _sortedItems[iLeft]->_centroid ) && iLeft < iRight)
				{
					iLeft++;
				}
					
				while(rightCentroid.contains( _sortedItems[iRight]->_centroid ) && iLeft < iRight)
				{
					iRight--;
				}

				if(iLeft < iRight)
				{
					ConstructionItem* t = _sortedItems[iRight];
					_sortedItems[iRight] = _sortedItems[iLeft];
					_sortedItems[iLeft] = t;
				}
			}

			node._childItemBegin = itemBegin;
			node._childItemEnd = itemEnd;
			node._bound = totalVolume;
			node._splitDirection = dominant;
			node._splitSAH = newSah;
			node._childSplit = iLeft;
			node._numChildNodes = 0;
		}

		inline size_t splitNode(Node& oldNode)
		{
			Node newNode;
			
			size_t oldBegin = oldNode._childItemBegin;
			size_t center = oldNode._childSplit;
			size_t newEnd = oldNode._childItemEnd;
			
			initializeLeaf(oldNode,oldBegin,center);
			initializeLeaf(newNode,center,newEnd);

			_nodes.push_back(newNode);
			return _nodes.size()-1;
		}

		static inline bool nodeWantsToSplit(const Node& node)
		{
			if(node._childItemEnd - node._childItemBegin > LeafSize )
				return true;
			else
				return false;
		}

		inline size_t projectItemsToBins(size_t itemBegin,size_t itemEnd,const Volume& centroidVolume,size_t dominant)
		{
			static const Real EPSILON = .00001f;
			//reset bins
			for(auto it = _bins.begin(); it != _bins.end(); ++it)
			{
				it->_itemCount = 0;
				it->_bound = Volume::Empty();
				it->_centroidBound = Volume::Empty();
			}
			
			Real dominantSize = centroidVolume.max()[dominant] - centroidVolume.min()[dominant];
			Real k = (Real)_binSize*(1.0f-EPSILON)/(dominantSize);

			for(size_t i = itemBegin; i < itemEnd; ++i)
			{
				size_t binIndex = (size_t)(k*(_sortedItems[i]->_centroid[dominant] - centroidVolume.min()[dominant]));
				_bins[binIndex]._centroidBound = Volume(_bins[binIndex]._centroidBound, _sortedItems[i]->_centroid);
				_bins[binIndex]._bound = Volume(_bins[binIndex]._bound, _sortedItems[i]->_volume);
				_bins[binIndex]._itemCount ++;
			}

			Volume summedBound = Volume::Empty();
			Volume summedCentroid = Volume::Empty();

			for(size_t i = 0; i < _bins.size(); ++i)
			{
				summedBound = Volume(summedBound,_bins[i]._bound);
				_bins[i]._boundLeft = summedBound;

				summedCentroid = Volume(summedCentroid,_bins[i]._centroidBound);
				_bins[i]._centroidLeft = summedCentroid;
			}

			summedBound = Volume::Empty();
			summedCentroid = Volume::Empty();

			for(size_t i = _bins.size(); i > 0; --i)
			{
				summedBound = Volume(summedBound,_bins[i-1]._bound);
				_bins[i-1]._boundRight = summedBound;

				summedCentroid = Volume(summedCentroid,_bins[i-1]._centroidBound);
				_bins[i-1]._centroidRight = summedCentroid;
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
}

#endif