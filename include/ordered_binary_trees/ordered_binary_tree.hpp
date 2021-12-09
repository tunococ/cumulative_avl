#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ordered_binary_trees {

/**
 *  @brief
 *  Convenience struct for allocating nodes, deallocating nodes, and managing
 *    important nodes: `root`, `first` and `last`.
 */
template<class NodeT, class AllocatorT = std::allocator<NodeT>>
struct OrderedBinaryTree {
  /// This type.
  using This = OrderedBinaryTree<NodeT, AllocatorT>;

  /// `NodeT`.
  using Node = NodeT;

  /// `AllocatorT`.
  using Allocator = typename std::allocator_traits<AllocatorT>::
      template rebind_alloc<Node>;

  /// `Node::Data`.
  using Data = typename Node::Data;

  /// `Node::size_type`.
  using size_type = typename Node::size_type;

  /// `Node::ThisPtr`.
  using NodePtr = typename Node::ThisPtr;

  /// `Node::ConstThisPtr`.
  using ConstNodePtr = typename Node::ConstThisPtr;

  /// `Node::CondThisPtr`.
  template<bool constant>
  using CondNodePtr = typename Node::template CondThisPtr<constant>;

  /// `Node::ChildType`.
  using ChildType = typename Node::ChildType;

  /// `Node::InsertPosition`.
  using InsertPosition = typename Node::InsertPosition;

  static_assert(std::is_same_v<
      NodePtr,
      typename std::allocator_traits<Allocator>::pointer>);
  static_assert(std::is_same_v<
      ConstNodePtr,
      typename std::allocator_traits<Allocator>::const_pointer>);

  /**
   *  @brief
   *  Allocator for nodes.
   */
  mutable Allocator allocator;

  /**
   *  @brief
   *  Root of the tree. This is null when the tree is empty.
   */
  NodePtr root{nullptr};

  /**
   *  @brief
   *  Leftmost node of the tree. This is null when the tree is empty.
   */
  NodePtr first{nullptr};

  /**
   *  @brief
   *  Rightmost node of the tree. This is null when the tree is empty.
   */
  NodePtr last{nullptr};
  
  /**
   *  @brief
   *  Creates a binary tree with a given allocator and a given root.
   */
  constexpr OrderedBinaryTree(
      Allocator const& allocator = Allocator(),
      NodePtr root = nullptr)
    : allocator{allocator},
      root{root},
      first{root ? root->find_first_node() : nullptr},
      last{root ? root->find_last_node() : nullptr} {}

  /**
   *  @brief
   *  Copies everything from another tree.
   *
   *  This copy constructor is useful only in a very special situation such as
   *    when the source tree will be abandoned at a later time.
   */
  template<class OtherNode, class OtherAllocator>
  constexpr OrderedBinaryTree(
      OrderedBinaryTree<OtherNode, OtherAllocator> const& other)
    : allocator{std::allocator_traits<Allocator>::
        select_on_container_copy_construction(other.allocator)},
      root{other.root},
      first{other.first},
      last{other.last} {
  }

  /**
   *  @brief
   *  Takes everything from another tree.
   */
  template<class OtherNode, class OtherAllocator>
  constexpr OrderedBinaryTree(
      OrderedBinaryTree<OtherNode, OtherAllocator>&& other)
    : allocator{std::move(other.allocator)},
      root{other.root},
      first{other.first},
      last{other.last} {
    other.clear();
  }

  /**
   *  @brief
   *  Copies everything (excluding `allocator`) from another tree.
   *
   *  This copy assignment abandons the current tree and copies all members
   *    from another tree.
   *  It is useful only in a very special situation where ownerships of nodes
   *    will be handled manually.
   */
  constexpr This& operator=(This const& other) {
    root = other.root;
    first = other.first;
    last = other.last;
    return *this;
  }

  /**
   *  @brief
   *  Takes everything (excluding `allocator`) from another tree.
   *
   *  This move assignment abandons the current tree and takes all members from
   *    another tree.
   */
  constexpr This& operator=(This&& other) {
    root = other.root;
    first = other.first;
    last = other.last;
    other.clear();
    return *this;
  }

  /**
   *  @brief
   *  Swaps contents of the tree (excluding `allocator`) with another tree.
   */
  constexpr void swap(This& other) {
    using std::swap;
    swap(root, other.root);
    swap(first, other.first);
    swap(last, other.last);
  }

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
   *  Empties the tree.
   */
  constexpr void clear() {
    root = nullptr;
    first = nullptr;
    last = nullptr;
  }

  /**
   *  @brief
   *  Gives up ownership of the root node.
   */
  constexpr NodePtr release() {
    NodePtr r{root};
    clear();
    return r;
  }

  /**
   *  @brief
   *  Calls `f(n)` for every node `n` reachable from `root`, sequentially
   *    ordered by the depth-first in-order.
   *
   *  `f` should be a unary operator that takes `NodePtr`.
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
   *  `f` should be a unary operator that takes `ConstNodePtr`.
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
   *  `f` should be a unary operator that takes `NodePtr`.
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
   *  `f` should be a unary operator that takes `ConstNodePtr`.
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
   *  `f` should be a unary operator that takes `NodePtr`.
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
   *  `f` should be a unary operator that takes `ConstNodePtr`.
   */
  template<class FunctionType>
  constexpr void traverse_preorder(FunctionType f) const {
    Node::template traverse_preorder<true, FunctionType>(root, f);
  }

  /**
   *  @brief
   *  Clones all nodes under the subtree rooted at `this` node.
   */
  constexpr NodePtr clone_nodes(ConstNodePtr n) const {
    assert(n);
    NodePtr cloned{create_node(n->data)};
    cloned->size = n->size;
    if (n->left_child) {
      NodePtr cloned_left_child{clone_nodes(n->left_child)};
      cloned->left_child = cloned_left_child;
      cloned_left_child->parent = cloned;
    }
    if (n->right_child) {
      NodePtr cloned_right_child{clone_nodes(n->right_child)};
      cloned->right_child = cloned_right_child;
      cloned_right_child->parent = cloned;
    }
    return cloned;
  }

  /**
   *  @brief
   *  Clones the tree.
   */
  constexpr This clone() const {
    return This{
        std::allocator_traits<Allocator>::
          select_on_container_copy_construction(allocator),
        root ? clone_nodes(root) : nullptr};
  }

  /**
   *  @brief
   *  Clones from another tree rooted at `n`.
   *
   *  If `destroy_nodes` is `true`, `destroy_all_nodes()` will be called prior
   *    to the clone operation.
   *  Otherwise, the current tree will be abandoned without a cleanup.
   */
  template<bool destroy_nodes = true>
  constexpr void clone_from(ConstNodePtr n) {
    if constexpr (destroy_nodes) {
      destroy_all_nodes();
    }
    root = clone_nodes(n);
    first = root->find_first_node();
    last = root->find_last_node();
  }

  /**
   *  @brief
   *  Returns the `index`-th node, relative to `root`.
   *
   *  If `index` exceeds the index of the rightmost node, `nullptr` will be
   *    returned.
   */
  constexpr NodePtr find_node_at_index(size_type index) {
    return Node::template find_node_at_index<false>(root, index);
  }

  /**
   *  @brief
   *  Returns the `index`-th node, relative to `root`.
   *
   *  If `index` exceeds the index of the rightmost node, `nullptr` will be
   *    returned.
   */
  constexpr ConstNodePtr find_node_at_index(size_type index) const {
    return Node::template find_node_at_index<true>(root, index);
  }

  /**
   *  @brief
   *  Returns the leftmost node reachable from `root`, or `nullptr` if `root`
   *    is null.
   */
  constexpr NodePtr find_first_node() {
    return root ? Node::template find_first_node<false>(root) : nullptr;
  }

  /**
   *  @brief
   *  Returns the leftmost node reachable from `root`, or `nullptr` if `root`
   *    is null.
   */
  constexpr ConstNodePtr find_first_node() const {
    return root ? Node::template find_first_node<true>(root) : nullptr;
  }

  /**
   *  @brief
   *  Returns the rightmost node reachable from `root`, or `nullptr` if `root`
   *    is null.
   */
  constexpr NodePtr find_last_node() {
    return root ? Node::template find_last_node<false>(root) : nullptr;
  }

  /**
   *  @brief
   *  Returns the rightmost node reachable from `root`, or `nullptr` if `root`
   *    is null.
   */
  constexpr ConstNodePtr find_last_node() const {
    return root ? Node::template find_last_node<true>(root) : nullptr;
  }

  /**
   *  @brief
   *  Creates a node using `allocator`.
   */
  template<class... Args>
  constexpr NodePtr create_node(Args&&... args) const {
    NodePtr n{std::allocator_traits<Allocator>::allocate(allocator, 1)};
    std::allocator_traits<Allocator>::construct(allocator,
        n, std::forward<Args>(args)...);
    return n;
  }

  /**
   *  @brief
   *  Destroys a node using `allocator`.
   */
  constexpr void destroy_node(NodePtr n) {
    assert(n);
    std::allocator_traits<Allocator>::destroy(allocator, n);
    std::allocator_traits<Allocator>::deallocate(allocator, n, 1);
  }

  /**
   *  @brief
   *  Calls `destroy_node(n)` for every node `n` reachable from `root` and sets
   *    `root` to `nullptr`.
   */
  constexpr void destroy_all_nodes() {
    traverse_postorder(
        [this](NodePtr n) {
          destroy_node(n);
        });
    clear();
  }

  /**
   *  @brief
   *  Calls `root->get_insert_position_for_index(index)` if `root` is not
   *    null, or returns an empty `InsertPosition` otherwise.
   */
  constexpr InsertPosition get_insert_position_for_index(size_type index) {
    if (root) {
      return root->get_insert_position_for_index(index);
    } else {
      return {};
    }
  }

  /**
   *  @brief
   *  Returns an `InsertPosition` for the prospective first node.
   */
  constexpr InsertPosition get_first_insert_position() {
    if (first) {
      return InsertPosition{first, true};
    } else {
      return {};
    }
  }

  /**
   *  @brief
   *  Returns an `InsertPosition` for the prospective last node.
   */
  constexpr InsertPosition get_last_insert_position() {
    if (last) {
      return InsertPosition{last, false};
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
   *
   *  If `n` is null, this function does nothing.
   */
  template<bool update_sizes = true>
  constexpr void link(InsertPosition const& pos, NodePtr n) {
    if (!n) {
      return;
    }
    if (root) {
      assert(pos.node);
      assert(pos.node->is_under(root));
      n->template link<update_sizes>(pos);
      if (pos.left_child && pos.node == first) {
        first = n->find_first_node();
      } else if (!pos.left_child && pos.node == last) {
        last = n->find_last_node();
      }
    } else {
      assert(!pos.node);
      root = n;
      first = n->find_first_node();
      last = n->find_last_node();
    }
  }

  /**
   *  @brief
   *  Calls `root->link_at_index(index, n)` if `root` is not null, or sets
   *    `root` to `n` if `root` is null.
   * 
   *  If `n` is null, this function does nothing.
   */
  template<bool update_sizes = true>
  constexpr void link_at_index(size_type index, NodePtr n) {
    if (!n) {
      return;
    }
    if (root) {
      if (index == 0) {
        first = n->find_first_node();
      } else if (index == size()) {
        last = n->find_last_node();
      }
      root->template link_at_index<update_sizes>(index, n);
    } else {
      assert(index == 0);
      root = n;
      first = n->find_first_node();
      last = n->find_last_node();
    }
  }

  /**
   *  @brief
   *  Calls `link(pos, other.root)`.
   *
   *  This function does not take ownership from `other`.
   *  To take ownership from `other`, use the overload that takes `This&&`.
   */
  template<bool update_sizes = true>
  constexpr void link_subtree(InsertPosition const& pos, This const& other) {
    if (other.empty()) {
      return;
    }
    if (root) {
      assert(pos.node);
      assert(pos.node->is_under(root));
      other.root->template link<update_sizes>(pos);
      if (pos.left_child && pos.node == first) {
        first = other.first;
      } else if (!pos.left_child && pos.node == last) {
        last = other.last;
      }
    } else {
      assert(!pos.node);
      operator=(other);
    }
  }

  /**
   *  @brief
   *  Calls the other overload of `link_subtree()`, then calls `other.clear()`.
   *
   *  In most situations, this overload should be preferred over the other one.
   */
  template<bool update_sizes = true>
  constexpr void link_subtree(InsertPosition const& pos, This&& other) {
    link_subtree(pos, const_cast<This const&>(other));
    other.clear();
  }

  /**
   *  @brief
   *  Calls `link_at_index(index, other.root)`.
   *
   *  This function does not take ownership from `other`.
   *  To take ownership from `other`, use the overload that takes `This&&`.
   */
  template<bool update_sizes = true>
  constexpr void link_subtree_at_index(size_type index, This const& other) {
    if (other.empty()) {
      return;
    }
    if (root) {
      if (index == 0) {
        first = other.first;
      } else if (index == size()) {
        last = other.last;
      }
      root->template link_at_index<update_sizes>(index, other.root);
    } else {
      assert(index == 0);
      operator=(other);
    }
  }

  /**
   *  @brief
   *  Calls the other overload of `link_subtree_at_index()`, then calls
   *    `other.clear()`.
   *
   *  In most situations, this overload should be preferred over the other one.
   */
  template<bool update_sizes = true>
  constexpr void link_subtree_at_index(size_type index, This&& other) {
    link_subtree_at_index(index, const_cast<This const&>(other));
    other.clear();
  }

  /**
   *  @brief
   *  Allocates a new node and adds it to the tree at the given
   *    `InsertPosition`, then returns the new node.
   *
   *  Note that the new node will not be automatically deallocated when
   *    `OrderedBinaryTree` is destroyed.
   * 
   *  @sa destroy_all_nodes, emplace
   */
  template<bool update_sizes = true, class... Args>
  constexpr NodePtr emplace(InsertPosition const& pos, Args&&... args) {
    NodePtr n{create_node(std::forward<Args>(args)...)};
    link<update_sizes>(pos, n);
    return n;
  }

  /**
   *  @brief
   *  Allocates a new node and adds it to the tree at the given `index`, then
   *    returns the new node.
   *
   *  Note that the new node will not be automatically deallocated when
   *    `OrderedBinaryTree` is destroyed.
   * 
   *  @sa destroy_all_nodes, emplace_at_index
   */
  template<bool update_sizes = true, class... Args>
  constexpr NodePtr emplace_at_index(size_type index, Args&&... args) {
    NodePtr n{create_node(std::forward<Args>(args)...)};
    link_at_index<update_sizes>(index, n);
    return n;
  }

  /**
   *  @brief
   *  Unlinks `n` from its parent and returns `InsertPosition` for the position
   *    where `n` used to be.
   */
  template<bool update_sizes = true>
  constexpr InsertPosition unlink(NodePtr n) {
    assert(root);
    assert(n->is_under(root));
    if (first->is_under(n)) {
      first = n->parent;
    }
    if (last->is_under(n)) {
      last = n->parent;
    }
    if (root == n) {
      clear();
    }
    return n->template unlink<update_sizes>();
  }

  /**
   *  @brief
   *  Unlinks a node at a given index from its parent, then returns the
   *    unlinked node and `InsertPosition` for the position where the node used
   *    to be.
   */
  template<bool update_sizes = true>
  constexpr std::pair<NodePtr, InsertPosition> unlink_at_index(
      size_type index) {
    NodePtr n{find_node_at_index(index)};
    assert(n);
    InsertPosition pos{unlink<update_sizes>(n)};
    return {n, pos};
  }

  /**
   *  @brief
   *  Unlinks `n` from its parent, then returns the subtree rooted at the `n`
   *    and `InsertPosition` for the position where `n` used to be in the
   *    original tree.
   */
  template<bool update_sizes = true>
  constexpr std::pair<This, InsertPosition> unlink_subtree(NodePtr n) {
    InsertPosition pos{unlink<update_sizes>(n)};
    return {This{allocator, n}, pos};
  }

  /**
   *  @brief
   *  Calls `unlink_subtree(find_node_at_index(index))`.
   */
  template<bool update_sizes = true>
  constexpr std::pair<This, InsertPosition> unlink_subtree_at_index(
      size_type index) {
    return unlink_subtree(find_node_at_index(index));
  }

  /**
   *  @brief
   *  Rotates a node to the left.
   *
   *  @note
   *  This may change `root`.
   */
  constexpr void rotate_left(NodePtr n) {
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
  constexpr void rotate_right(NodePtr n) {
    n->rotate_right();
    if (n == root) {
      root = root->parent;
    }
  }

  /**
   *  @brief
   *  Splays a node `n` to be an immediate child of `top` or to be the root if
   *    `top` is null.
   *  `f()` is called for all nodes affected by splaying.
   *
   *  @note
   *  If `top` is null and `n` is not `root`, `root` will change.
   */
  template<class FunctionType>
  constexpr void splay(NodePtr n, FunctionType f, NodePtr top = nullptr) {
    assert(root);
    assert(n);
    assert(n->is_under(root));
    assert(!top || top->is_under(root));
    n->splay(f, top);
    if (!top) {
      root = n;
    }
  }

  /**
   *  @brief
   *  Overload of `splay()` that supplies `update_node_size` as the *update*
   *    function.
   */
  template<bool update_sizes = true>
  constexpr void splay(NodePtr n, NodePtr top = nullptr) {
    if constexpr (update_sizes) {
      splay(n, Node::update_node_size, top);
    } else {
      splay(n, [](NodePtr) {}, top);
    }
  }

  /**
   *  @brief
   *  Calls `n1->swap(n2)` and updates `root` if it is involved in the swap.
   */
  constexpr void swap_nodes(NodePtr n1, NodePtr n2) {
    if (n1 == n2) {
      return;
    }
    unsigned root_matches{root == n1 ? 1u : (root == n2 ? 2u : 0)};
    unsigned first_matches{first == n1 ? 1u : (first == n2 ? 2u : 0)};
    unsigned last_matches{last == n1 ? 1u : (last == n2 ? 2u : 0)};
    n1->swap(n2);
    if (root_matches == 1) {
      root = n2;
    } else if (root_matches == 2) {
      root = n1;
    }
    if (first_matches == 1) {
      first = n2;
    } else if (first_matches == 2) {
      first = n1;
    }
    if (last_matches == 1) {
      last = n2;
    } else if (last_matches == 2) {
      last = n1;
    }
  }

  /**
   *  @brief
   *  Calls `n->erase()` and updates `root`, `first`, and `last` if necessary.
   *
   *  If the template parameter `delete_node` is set to `true`, the node being
   *    erased will also be deleted.
   */
  template<bool update_sizes = true, bool delete_node = true>
  constexpr std::pair<NodePtr, NodePtr> erase(NodePtr n) {
    assert(root);
    assert(n->is_under(root));
    if (first == n) {
      first = first->find_next_node();
    }
    if (last == n) {
      last = last->find_prev_node();
    }
    std::pair<NodePtr, NodePtr> erase_result{n->template erase<update_sizes>()};
    if (root == n) {
      root = erase_result.first;
    }
    if constexpr (delete_node) {
      destroy_node(n);
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
  constexpr std::tuple<NodePtr, NodePtr, NodePtr> erase_at_index(size_type index) {
    assert(root);
    NodePtr n{find_node_at_index(index)};
    std::pair<NodePtr, NodePtr> erase_result{erase<update_sizes, false>(n)};
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
  constexpr std::pair<NodePtr, NodePtr> delete_at_index(size_type index) {
    assert(root);
    NodePtr n{find_node_at_index(index)};
    std::pair<NodePtr, NodePtr> erase_result{erase<update_sizes, true>(n)};
    return erase_result;
  }

};

} // namespace ordered_binary_trees
