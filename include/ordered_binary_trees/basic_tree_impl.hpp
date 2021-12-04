#pragma once

#include <cassert>
#include <iterator>
#include <memory>

#include <ordered_binary_trees/ordered_binary_tree.hpp>
#include <ordered_binary_trees/ordered_binary_tree_iterator.hpp>
#include <ordered_binary_trees/ordered_binary_tree_node.hpp>

namespace ordered_binary_trees {

/**
 *  @brief
 *  Example template class to demonstrate how to *implement* a binary search
 *    tree data structure that is compatible with `ManagedTree`.
 *
 *  This class only contains types and static functions.
 */
template<class ValueT, class AllocatorT = std::allocator<ValueT>>
struct BasicTreeImpl {
  /// This type.
  using This = BasicTreeImpl<ValueT, AllocatorT>;

  /// Type of values to present to the user.
  using Value = ValueT;

  /// Type of allocators for `Value`.
  using ValueAllocator = AllocatorT;

  /**
   *  @brief
   *  Type of `Data` in `Node`.
   *
   *  This typically contains `Value`.
   *  For example, in a red-black tree, `Data` can be `std::pair<Value, bool>`.
   *
   *  In this simplest case (`BasicTreeImpl`), we simply pick `Data = Value`.
   */
  using Data = Value;

  /// Extraction of `AddPointer` inside `AddPointerFromAllocator`.
  template<class T>
  using AddPointer = typename AddPointerFromAllocator<ValueAllocator>::
      template AddPointer<T>;

  /// Type of nodes in a tree.
  using Node = OrderedBinaryTreeNode<Data, AddPointer>;

  /// Type of node allocators.
  using Allocator = typename std::allocator_traits<ValueAllocator>::
      template rebind_alloc<Node>;

  /// Type of trees.
  using Tree = OrderedBinaryTree<Node, Allocator>;

  /**
   *  @brief
   *  Struct or class that contains a public static function to convert `Data&`
   *    to `Value&`.
   */
  using ExtractValue = DefaultExtractValue<Node>;

  /// Type of node pointers.
  using NodePtr = typename Tree::NodePtr;

  /**
   *  @brief
   *  Constructs a new node and places it as the first node in a tree.
   */
  template<class... Args>
  static constexpr NodePtr emplace_front(Tree& tree, Args&&... args) {
    return tree.emplace(
        tree.first->make_insert_position(true),
        std::forward<Args>(args)...);
  }

  /**
   *  @brief
   *  Constructs a new node and places it as the last node in a tree.
   */
  template<class... Args>
  static constexpr NodePtr emplace_back(Tree& tree, Args&&... args) {
    return tree.emplace(
        tree.last->make_insert_position(false),
        std::forward<Args>(args)...);
  }

  /**
   *  @brief
   *  Constructs a new node and places it as an immediate predecessor of
   *    `node`, then returns the new node.
   */
  template<class... Args>
  static constexpr NodePtr emplace_node_before(
      Tree& tree,
      NodePtr node,
      Args&&... args) {
    return tree.emplace(
        node ?
          node->get_prev_insert_position() :
          tree.get_last_insert_position(),
        std::forward<Args>(args)...);
  }

  /**
   *  @brief
   *  Constructs nodes for values in `[input_i, input_end)` and adds them to
   *    the tree right before `node`, then returns the first new node.
   *
   *  If `input_i == input_end`, this function will return `nullptr`.
   *
   *  Each value extracted from the iterator will be fed as the only argument
   *    to the constructor of `Value`.
   *  That means `InputIterator` can point to any type that is convertible to
   *    `Value` via the constructor of `Value`.
   */
  template<class InputIterator>
  static constexpr NodePtr insert_nodes_before(
      Tree& tree,
      NodePtr node,
      InputIterator input_i,
      InputIterator input_end) {
    if (input_i == input_end) {
      return nullptr;
    }
    NodePtr first_new_node{emplace_node_before(tree, node, *input_i)};
    node = first_new_node;
    for (++input_i; input_i != input_end; ++input_i) {
      auto pos{node->make_insert_position(false)};
      node = tree.create_node(*input_i);
      tree.link(pos, node);
    }
    return first_new_node;
  }

  static constexpr void erase_front(Tree& tree) {
    assert(!tree.empty());
    tree.template erase<true, true>(tree.first);
  }

  static constexpr void erase_back(Tree& tree) {
    assert(!tree.empty());
    tree.template erase<true, true>(tree.last);
  }

  /**
   *  @brief
   *  Erases `node` and returns its former immediate successor.
   */
  static constexpr NodePtr erase_node(Tree& tree, NodePtr node) {
    NodePtr next_node{node->find_next_node()};
    tree.template erase<true, true>(node);
    return next_node;
  }

  /**
   *  @brief
   *  Erases nodes in the interval `[begin, end)`.
   * 
   *  If `begin == end`, this function will not erase any nodes and return
   *    `begin`.
   */
  static constexpr NodePtr erase_nodes(
      Tree& tree,
      NodePtr begin,
      NodePtr end) {
    while (begin != end) {
      assert(begin);
      NodePtr next{begin->find_next_node()};
      tree.erase(begin);
      begin = next;
    }
    return begin;
  }

};

} // namespace ordered_binary_trees
