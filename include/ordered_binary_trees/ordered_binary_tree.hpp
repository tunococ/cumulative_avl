#pragma once

#include <algorithm>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include <ordered_binary_trees/assert.hpp>

namespace ordered_binary_trees {

template<class NodeT>
struct OrderedBinaryTree {
  using Node = NodeT;
  using This = OrderedBinaryTree<Node>;

  using Data = typename Node::Data;
  using size_type = typename Node::size_type;
  using ChildType = typename Node::ChildType;
  using InsertPosition = typename Node::InsertPosition;

  /**
   *  @brief
   *  Root of the tree. This is null when the tree is empty.
   */
  Node* root{nullptr};

  /**
   *  @brief
   *  Creates a binary tree with a given root.
   */
  constexpr OrderedBinaryTree(Node* root = nullptr) : root{root} {}

  /**
   *  @brief
   *  Returns the number of nodes in the tree.
   */
  constexpr size_type size() const {
    return root ? root->size : 0;
  }

  /**
   *  @brief
   *  Returns true if and only if the tree is empty.
   */
  constexpr bool empty() const {
    return !root;
  }

  /**
   *  @brief
   *  Convenience operator for accessing `root`.
   */
  Node* operator->() {
    return root;
  }

  /**
   *  @brief
   *  Convenience operator for accessing `root`.
   */
  Node const* operator->() const {
    return root;
  }

  /**
   *  @brief
   *  Convenience operator for accessing `root`.
   */
  Node& operator*() {
    return *root;
  }

  /**
   *  @brief
   *  Convenience operator for accessing `root`.
   */
  Node const& operator*() const {
    return *root;
  }

  /**
   *  @brief
   *  Convenience operator for accessing `root`.
   */
  operator Node*() {
    return root;
  }

  /**
   *  @brief
   *  Convenience operator for accessing `root`.
   */
  operator Node const*() const {
    return root;
  }

  /**
   *  @brief
   *  Calls `root->find_insert_position_for_index(index)` if `root` is not
   *    null, or returns an empty `InsertPosition` otherwise.
   */
  constexpr InsertPosition find_insert_position_for_index(size_type index) {
    if (root) {
      return root->find_insert_position_for_index(index);
    } else {
      return {};
    }
  }

  /**
   *  @brief
   *  Links `n` as a child of `pos.node` if `root` is not null, or assigns `n`
   *    to `root` otherwise.
   *
   *  This function does not check if `pos.node` is actually reachable from
   *    `root`.
   */
  constexpr void link_child(InsertPosition const& pos, Node* n) {
    if (root) {
      ASSERT(pos.node);
      Node::link_child(pos, n);
    } else {
      ASSERT(!pos.node);
      root = n;
    }
  }

  /**
   *  @brief
   *  Calls `root->insert_at_index(index, n)` if `root` is not null, or sets
   *    `root` to `n` if `root` is null.
   */
  template<bool update_sizes = true>
  constexpr void insert_at_index(size_type index, Node* n) {
    ASSERT(n);
    if (root) {
      root->template insert_at_index<update_sizes>(index, n);
    } else {
      ASSERT(index == 0);
      root = n;
    }
  }

  /**
   *  @brief
   *  Calls `root->emplace_at_index(index, args...)` if `root` is not null, or
   *    sets `root` to a new node constructed with the given `args`, then
   *    returns the constructed node.
   *
   *  Note that the returned node will not be automatically deallocated by
   *    `OrderedBinaryTree`'s destructor.
   *  `OrderedBinaryTree::delete_nodes()` can be called to clean up all nodes
   *    reachable from `root`.
   */
  template<bool update_sizes = true, class... Args>
  constexpr Node* emplace_at_index(size_type index, Args&&... args) {
    if (root) {
      return root->template emplace_at_index<update_sizes>(
          index,
          std::forward<Args>(args)...);
    } else {
      ASSERT(index == 0);
      root = new Node(nullptr, std::forward<Args>(args)...);
      return root;
    }
  }

  /**
   *  @brief
   *  Unlinks a node at a given index from the tree.
   *  The node to unlink may be non-leaf.
   */
  template<bool update_sizes = true>
  constexpr Node* unlink_at_index(size_type index) {
    ASSERT(root);
    return root->unlink_at_index(index);
  }

  /**
   *  @brief
   *  Calls `f(n)` for every node `n` reachable from `root`, sequentially
   *    ordered by the depth-first in-order.
   *
   *  `f` should be a unary operator that takes `Node*`.
   */
  template<class FunctionType>
  constexpr void traverse_inorder(FunctionType f) {
    Node::template traverse_inorder<false, FunctionType>(root, f);
  }

  /**
   *  @brief
   *  Calls `f(n)` for every node `n` reachable from `root`, sequentially
   *    ordered by the depth-first in-order.
   *
   *  `f` should be a unary operator that takes `Node const*`.
   */
  template<class FunctionType>
  constexpr void traverse_inorder(FunctionType f) const {
    Node::template traverse_inorder<true, FunctionType>(root, f);
  }

  /**
   *  @brief
   *  Calls `f(n)` for every node `n` reachable from `root`, sequentially
   *    ordered by the depth-first post-order.
   *
   *  `f` should be a unary operator that takes `Node*`.
   */
  template<class FunctionType>
  constexpr void traverse_postorder(FunctionType f) {
    Node::template traverse_postorder<false, FunctionType>(root, f);
  }

  /**
   *  @brief
   *  Calls `f(n)` for every node `n` reachable from `root`, sequentially
   *    ordered by the depth-first post-order.
   *
   *  `f` should be a unary operator that takes `Node const*`.
   */
  template<class FunctionType>
  constexpr void traverse_postorder(FunctionType f) const {
    Node::template traverse_postorder<true, FunctionType>(root, f);
  }

  /**
   *  @brief
   *  Calls `f(n)` for every node `n` reachable from `root`, sequentially
   *    ordered by the depth-first pre-order.
   *
   *  `f` should be a unary operator that takes `Node*`.
   */
  template<class FunctionType>
  constexpr void traverse_preorder(FunctionType f) {
    Node::template traverse_preorder<false, FunctionType>(root, f);
  }

  /**
   *  @brief
   *  Calls `f(n)` for every node `n` reachable from `root`, sequentially
   *    ordered by the depth-first pre-order.
   *
   *  `f` should be a unary operator that takes `Node const*`.
   */
  template<class FunctionType>
  constexpr void traverse_preorder(FunctionType f) const {
    Node::template traverse_preorder<true, FunctionType>(root, f);
  }

  /**
   *  @brief
   *  Clones the tree.
   */
  constexpr This clone() const {
    return This{root ? root->clone() : nullptr};
  }

  /**
   *  @brief
   *  Returns the `index`-th node, relative to `root`.
   *
   *  If `index` exceeds the index of the rightmost node, `nullptr` will be
   *    returned.
   */
  constexpr Node* find_node_at_index(size_type index) {
    return Node::template find_node_at_index<false>(root, index);
  }

  /**
   *  @brief
   *  Returns the `index`-th node, relative to `root`.
   *
   *  If `index` exceeds the index of the rightmost node, `nullptr` will be
   *    returned.
   */
  constexpr Node const* find_node_at_index(size_type index) const {
    return Node::template find_node_at_index<true>(root, index);
  }

  /**
   *  @brief
   *  Returns the leftmost node reachable from `root`, or `nullptr` if `root`
   *    is null.
   */
  constexpr Node* find_first_node() {
    return root ? Node::template find_first_node<false>(root) : nullptr;
  }

  /**
   *  @brief
   *  Returns the leftmost node reachable from `root`, or `nullptr` if `root`
   *    is null.
   */
  constexpr Node const* find_first_node() const {
    return root ? Node::template find_first_node<true>(root) : nullptr;
  }

  /**
   *  @brief
   *  Returns the rightmost node reachable from `root`, or `nullptr` if `root`
   *    is null.
   */
  constexpr Node* find_last_node() {
    return root ? Node::template find_last_node<false>(root) : nullptr;
  }

  /**
   *  @brief
   *  Returns the rightmost node reachable from `root`, or `nullptr` if `root`
   *    is null.
   */
  constexpr Node const* find_last_node() const {
    return root ? Node::template find_last_node<true>(root) : nullptr;
  }

  /**
   *  @brief
   *  Calls `delete n` for every node `n` reachable from `root` and sets `root`
   *    to `nullptr`.
   */
  constexpr void delete_nodes() {
    Node::delete_nodes(root);
    root = nullptr;
  }

  /**
   *  @brief
   *  Rotates a node to the left.
   *
   *  @note
   *  This may change `root`.
   */
  constexpr void rotate_left(Node* n) {
    n->rotate_left();
    if (n == root) {
      root = root->parent;
    }
  }

  /**
   *  @brief
   *  Rotates a node to the right.
   *
   *  @note
   *  This may change `root`.
   */
  constexpr void rotate_right(Node* n) {
    n->rotate_right();
    if (n == root) {
      root = root->parent;
    }
  }

  /**
   *  @brief
   *  Splays a node to the root and calls `f(n)` for each node `n` that is
   *    affected by splaying.
   *
   *  @note
   *  This usually changes `root`.
   */
  template<class FunctionType>
  constexpr void splay(Node* n, FunctionType f) {
    ASSERT(root);
    ASSERT(n);
    n->splay(f);
    root = n;
  }

  /**
   *  @brief
   *  Overload of `splay()` that supplies `update_node_size` as the *update*
   *    function.
   */
  constexpr void splay(Node* n) {
    splay(n, Node::update_node_size);
  }

  /**
   *  @brief
   *  Calls `n1->swap(n2)` and updates `root` if it is involved in the swap.
   */
  constexpr void swap(Node* n1, Node* n2) {
    if (n1 == root) {
      n1->swap(n2);
      root = n2;
    } else if (n2 == root) {
      n2->swap(n1);
      root = n1;
    } else {
      n1->swap(n2);
    }
  }

  /**
   *  @brief
   *  Calls `n->erase()` and updates `root` if necessary.
   *
   *  If the template parameter `delete_node` is set to `true`, the node being
   *    erased will also be deleted.
   */
  template<bool update_sizes = true, bool delete_node = false>
  constexpr std::pair<Node*, Node*> erase(Node* n) {
    ASSERT(root);
    std::pair<Node*, Node*> erase_result{n->template erase<update_sizes>()};
    if (root == n) {
      root = erase_result.first;
    }
    if constexpr (delete_node) {
      delete n;
    }
    return erase_result;
  }

  /**
   *  @brief
   *  Erases a node at a given index.
   *
   *  The node will be removed from the tree and returned in the last component
   *    of the return value.
   *  The node will not be automatically deleted.
   *
   *  The first two components of the return value come from calling `erase()`
   *    on the node at the given index.
   */
  template<bool update_sizes = true>
  constexpr std::tuple<Node*, Node*, Node*> erase_at_index(size_type index) {
    ASSERT(root);
    Node* n{find_node_at_index(index)};
    std::pair<Node*, Node*> erase_result{erase<update_sizes, false>(n)};
    return {erase_result.first, erase_result.second, n};
  }

  /**
   *  @brief
   *  Erases a node at a given index and deletes it.
   *
   *  The return value comes from calling `erase()` on the node at the given
   *    index.
   */
  template<bool update_sizes = true>
  constexpr std::pair<Node*, Node*> delete_at_index(size_type index) {
    ASSERT(root);
    Node* n{find_node_at_index(index)};
    std::pair<Node*, Node*> erase_result{erase<update_sizes, true>(n)};
    return erase_result;
  }

};

} // namespace ordered_binary_trees
