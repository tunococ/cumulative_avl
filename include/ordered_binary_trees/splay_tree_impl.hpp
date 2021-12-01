#pragma once

#include <iterator>
#include <memory>

#include <ordered_binary_trees/ordered_binary_tree.hpp>
#include <ordered_binary_trees/ordered_binary_tree_iterator.hpp>
#include <ordered_binary_trees/ordered_binary_tree_node.hpp>

namespace ordered_binary_trees {

template<class DataT, class AllocatorT = std::allocator<DataT>>
struct SplayTreeImpl {
  using This = SplayTreeImpl<DataT, AllocatorT>;
  using Data = DataT;
  using Node = OrderedBinaryTreeNode<
      Data,
      typename AddPointerFromAllocator<AllocatorT>::AddPointer>;
  using Allocator = typename std::allocator_traits<AllocatorT>::
      rebind_alloc<Node>;
  using DataAllocator = typename std::allocator_traits<AllocatorT>::
      rebind_alloc<Data>;
  using Tree = OrderedBinaryTree<Node, Allocator>;

  using ExtractValue = DefaultExtractValue<Node>;

};

} // namespace ordered_binary_trees
