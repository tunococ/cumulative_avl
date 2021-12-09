#pragma once

#include <cassert>
#include <iterator>
#include <memory>

#include <ordered_binary_trees/ordered_binary_tree.hpp>
#include <ordered_binary_trees/ordered_binary_tree_iterator.hpp>
#include <ordered_binary_trees/ordered_binary_tree_node.hpp>
#include <ordered_binary_trees/basic_tree_impl.hpp>

namespace ordered_binary_trees {

/**
 *  @brief
 *  Example template class to demonstrate how to *implement* a binary search
 *    tree data structure that is compatible with `ManagedTree`.
 *
 *  This class only contains types and static functions.
 */
template<class ValueT, class AllocatorT = std::allocator<ValueT>>
struct SplayTreeImpl: BasicTreeImpl<ValueT, AllocatorT> {
  /// This type.
  using This = SplayTreeImpl<ValueT, AllocatorT>;

  /// Base class: `BasicTreeImpl<Value, Allocator>`.
  using Super = BasicTreeImpl<ValueT, AllocatorT>;

  /// Type of values to present to the user.
  using Value = ValueT;

  /// Type of allocators for `Value`.
  using ValueAllocator = AllocatorT;

  /// Splay trees do not need extra data.
  using Data = Value;

  /// Extraction of `AddPointer` inside `AddPointerFromAllocator`.
  template<class T>
  using AddPointer = typename Super::template AddPointer<T>;

  /// Type of nodes in a tree.
  using Node = typename Super::Node;

  /// Type of node allocators.
  using Allocator = typename Super::Node;

  /// Type of trees.
  using Tree = typename Super::Tree;

  /// Type of indices.
  using size_type = typename Super::size_type;

  /// Conversion from `Data` to `Value` by the identity function.
  using ExtractValue = typename Super::ExtractValue;

  /// Type of node pointers.
  using NodePtr = typename Super::NodePtr;

	/// `Tree::InsertPosition`.
	using InsertPosition = typename Super::InsertPosition;

  /**
   *  @brief
   *  Returns the node at a given index.
   */
  static constexpr NodePtr find_node_at_index(Tree& tree, size_type index) {
		NodePtr n{tree.find_node_at_index(index)};
		if (n) {
			tree.splay(n);
		}
    return n;
  }

  /**
   *  @brief
   *  Constructs a new node and places it as the first node in a tree.
   */
  template<class... Args>
  static constexpr NodePtr emplace_front(Tree& tree, Args&&... args) {
    if (tree.empty()) {
			return tree.emplace(InsertPosition{}, std::forward<Args>(args)...);
    }
    tree.splay(tree.first);
		return tree.emplace(
				tree.get_first_insert_position(),
				std::forward<Args>(args)...);
  }

  /**
   *  @brief
   *  Constructs a new node and places it as the last node in a tree.
   */
  template<class... Args>
  static constexpr NodePtr emplace_back(Tree& tree, Args&&... args) {
		if (tree.empty()) {
			return tree.emplace(InsertPosition{}, std::forward<Args>(args)...);
		}
		tree.splay(tree.last);
		return tree.emplace(
				tree.get_last_insert_position(),
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
		NodePtr n{tree.emplace(
        node ?
          node->get_prev_insert_position() :
          tree.get_last_insert_position(),
        std::forward<Args>(args)...)};
		tree.splay(n);
		return n;
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
      return node;
    }
    NodePtr first_new_node{Super::emplace_node_before(tree, node, *input_i)};
    node = first_new_node;
		NodePtr last_node{node};
    for (++input_i; input_i != input_end; ++input_i) {
      auto pos{node->make_insert_position(false)};
      node = tree.create_node(*input_i);
			last_node = node;
      tree.link(pos, node);
    }
		tree.splay(last_node);
    return first_new_node;
  }

  /**
   *  @brief
   *  Clears the tree and assigns values from `[input_begin, input_end)` to the
   *    tree.
   */
  template<class InputIterator>
  static constexpr void assign(
      Tree& tree,
      InputIterator input_begin,
      InputIterator input_end) {
    tree.destroy_all_nodes();
    insert_nodes_before(tree, tree.first, input_begin, input_end);
  }

  /**
   *  @brief
   *  Erases the first node.
   */
  static constexpr void erase_front(Tree& tree) {
    assert(!tree.empty());
		tree.splay(tree.first);
    tree.template erase<true, true>(tree.first);
  }

  /**
   *  @brief
   *  Erases the last node.
   */
  static constexpr void erase_back(Tree& tree) {
    assert(!tree.empty());
		tree.splay(tree.last);
    tree.template erase<true, true>(tree.last);
  }

  /**
   *  @brief
   *  Erases `node` and returns its former immediate successor.
   */
  static constexpr NodePtr erase_node(Tree& tree, NodePtr node) {
    NodePtr next_node{node->find_next_node()};
    NodePtr p{tree.template erase<true, true>(node).second};
		if (p) {
			tree.splay(p);
		}
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
		if (begin == end) {
			return end;
		}
		assert(begin);
		if (end) {
			tree.splay(end);
		}
		NodePtr prev{begin->find_prev_node()};
		NodePtr sub;
		if (prev) {
			tree.splay(prev, end);
			assert(!end || prev->parent == end);
			assert(begin->is_under(prev->right_child));
			sub = prev->right_child;
		} else if (end) {
			sub = end->left_child;
		} else {
			sub = tree.root;
		}
		tree.unlink(sub);

		Node::template traverse_postorder<false>(sub, [&tree](NodePtr n) {
					tree.destroy_node(n);
				});
    return end;
  }

};

} // namespace ordered_binary_trees
