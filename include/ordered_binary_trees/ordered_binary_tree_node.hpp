#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include <ordered_binary_trees/assert.hpp>

namespace ordered_binary_trees {

/**
 *  @brief
 *  Struct that presents `AddPointer` for any given `Allocator` type.
 * 
 *  For generality, `OrderedBinaryTreeNode` takes a functor of sort `* -> *`
 *    (similar to `std::add_pointer`) instead of taking an allocator type.
 *  The struct `AddPointerFromAllocator<Allocator>` provides a convenient way
 *    to construct the `AddPointer` functor for any valid `Allocator` type.
 *  (See `std::allocator` and `std::allocator_traits` for more details.)
 */
template<class AllocatorT>
struct AddPointerFromAllocator {
  using Allocator = AllocatorT;

  template<class T>
  struct AddPointer {
    using type = typename std::allocator_traits<Allocator>::
        template rebind_traits<T>::pointer;
  };
};

/**
 *  @brief
 *  Basic type of nodes in an ordered binary tree.
 */
template<
    class DataT,
    template<class> class AddPointerT = std::add_pointer,
    class SizeT = std::size_t>
struct OrderedBinaryTreeNode {
  /// This type.
  using This = OrderedBinaryTreeNode<DataT, AddPointerT, SizeT>;
  /// `DataT`.
  using Data = DataT;
  /// `SizeT`.
  using size_type = SizeT;
  
  template<class T>
  using AddPointer = typename AddPointerT<T>::type;

  using ThisPtr = typename AddPointerT<This>::type;
  using ConstThisPtr = typename AddPointerT<This const>::type;
  template<bool constant>
  using CondThisPtr = std::conditional_t<constant, ConstThisPtr, ThisPtr>;

  using DataPtr = typename AddPointerT<Data>::type;
  using ConstDataPtr = typename AddPointerT<Data const>::type;
  template<bool constant>
  using CondDataPtr = std::conditional_t<constant, ConstDataPtr, DataPtr>;

  /// Left child. May be null.
  ThisPtr left_child{nullptr};
  /// Right child. May be null.
  ThisPtr right_child{nullptr};
  /// Parent. May be null.
  ThisPtr parent{nullptr};
  /**
   *  @brief
   *  Size of the subtree rooted at this node.
   *  This is the piece of information that enables integer indexing.
   */
  size_type size{1};
  /**
   *  @brief
   *  Custom data.
   */
  Data data;

  /**
   *  @brief
   *  Whether a node is a root, a left child, or a right child.
   */
  enum ChildType : unsigned char {
    /// The node is a root.
    kNotChild = 0,
    /// The node is a left child of another node.
    kLeftChild = 1,
    /// The node is a right child of another node.
    kRightChild = 2
  };

  /**
   *  @brief
   *  Location of a prospective new node.
   */
  struct InsertPosition {
    /// Parent of the prospective new node. May be null if the tree is empty.
    ThisPtr node;
    /// Whether the prospective new node will be a left child of `node`.
    bool left_child;

    /**
     *  @brief
     *  Creates a position for a new root node.
     *  `left_child` is not relevant in this case.
     */
    constexpr InsertPosition()
      : node{nullptr} {}

    /**
     *  @brief
     *  Creates a position for a child of `node`.
     *
     *  If `node` is null, this is interpreted as creating a new root node,
     *    i.e., the result is the same as the default constructor.
     *  Otherwise, `left_child` determines whether the new node will be a left
     *    child of `n`.
     *  If `left_child` is `true`, `node->left_child` should be null.
     *  Otherwise, `node->right_child` should be null.
     */
    constexpr InsertPosition(ThisPtr node, bool left_child)
      : node{node}, left_child{left_child} {
    }

    /**
     *  @brief
     *  Returns `true` iff this `InsertPosition` is equal to `other`.
     */
    constexpr bool operator==(InsertPosition const& other) const {
      return (node == other.node) && (left_child == other.left_child);
    }

    /**
     *  @brief
     *  Returns `false` iff this `InsertPosition` is equal to `other`.
     */
    constexpr bool operator!=(InsertPosition const& other) const {
      return !operator==(other);
    }
  };

  /**
   *  @brief
   *  Creates a node and initializes `data` from `args`.
   */
  template<class... Args>
  constexpr OrderedBinaryTreeNode(Args&&... args)
    : data(std::forward<Args>(args)...) {}

  /**
   *  @brief
   *  Returns the `ChildType` of this node.
   */
  constexpr ChildType get_child_type() const {
    if (!parent) {
      return kNotChild;
    }
    return parent->left_child == this ? kLeftChild : kRightChild;
  }

  /**
   *  @brief
   *  Returns `true` iff `this` is a leaf node.
   */
  constexpr bool is_leaf() const {
    return !left_child && !right_child;
  }

  /**
   *  @brief
   *  Returns `true` iff `this` is a root node.
   */
  constexpr bool is_root() const {
    return !parent;
  }

  /**
   *  @brief
   *  Returns the `size` of node `n`.
   *  `n` may be null, in which case the return value will be `0`.
   */
  static constexpr size_type get_size(ThisPtr n) {
    return n ? n->size : 0;
  }

  /**
   *  @brief
   *  Returns `true` if `this` node is a descendant of `a`.
   */
  constexpr bool is_under(ConstThisPtr a) const {
    ConstThisPtr n{this};
    while (n != a) {
      if (!n) {
        return false;
      }
      n = n->parent;
    }
    return true;
  }

  /**
   *  @brief 
   *  Applies the unary function `f` to nodes along the path from `n` to the
   *    root, stopping if `f()` returns `false`.
   *  The return value of `traverse_upwards()` is the last node for which
   *    `f()` returns `true`.
   */
  template<bool constant, class FunctionType>
  static constexpr CondThisPtr<constant> traverse_upwards(
      CondThisPtr<constant> n,
      FunctionType f) {
    assert(n);
    if (!f(n)) {
      return nullptr;
    }
    while (true) {
      auto p{n->parent};
      if (!p || !f(p)) {
        return n;
      }
      n = p;
    }
  }

  /**
   *  @brief
   *  Calls `traverse_upwards(this, f)`.
   */
  template<class FunctionType>
  constexpr ThisPtr traverse_upwards(FunctionType f) {
    return traverse_upwards<false, FunctionType>(this, f);
  }

  /**
   *  @brief
   *  Calls `traverse_upwards(this, f)`.
   */
  template<class FunctionType>
  constexpr ConstThisPtr traverse_upwards(FunctionType f) const {
    return traverse_upwards<true, FunctionType>(this, f);
  }

  /**
   *  @brief
   *  Attempts to update the size at a single node and returns `true` iff the
   *    size actually changes.
   *
   *  This function assumes that `left_child` and `right_child` have the
   *    correct sizes.
   */
  constexpr bool update_size() {
    const size_type new_size{
        1 + get_size(left_child) + get_size(right_child)};
    if (new_size != size) {
      size = new_size;
      return true;
    }
    return false;
  }

  /**
   *  @brief
   *  Static version of `update_size()`.
   */
  static constexpr bool update_node_size(ThisPtr n) {
    return n->update_size();
  }

  /**
   *  @brief
   *  Calls `traverse_upwards()` with `update_node_size()` as the unary
   *    function.
   */
  constexpr ThisPtr update_sizes_upwards() {
    return traverse_upwards(update_node_size);
  }

  /**
   *  @brief
   *  Static version of `update_sizes_upwards()`.
   */
  static constexpr ThisPtr update_node_sizes_upwards(ThisPtr n) {
    return n->update_sizes_upwards();
  }

  /**
   *  @brief
   *  Applies the unary function `f` to each node in the subtree rooted at
   *    `n`.
   *  The nodes are ordered according to the depth-first in-order.
   * 
   *  `f` should be a unary operator that takes one argument of type `ThisPtr`.
   */
  template<bool constant, class FunctionType>
  static constexpr void traverse_inorder(
      CondThisPtr<constant> n,
      FunctionType f) {
    if (n) {
      traverse_inorder<constant, FunctionType>(n->left_child, f);
      f(n);
      traverse_inorder<constant, FunctionType>(n->right_child, f);
    }
  }

  /**
   *  @brief
   *  Calls `traverse_inorder(this, f);
   */
  template<class FunctionType>
  constexpr void traverse_inorder(FunctionType f) {
    traverse_inorder<false, FunctionType>(this, f);
  }

  /**
   *  @brief
   *  Calls `traverse_inorder(this, f);
   */
  template<class FunctionType>
  constexpr void traverse_inorder(FunctionType f) const {
    traverse_inorder<true, FunctionType>(this, f);
  }

  /**
   *  @brief
   *  Applies the unary function `f` to each node in the subtree rooted at
   *    `n`.
   *  The nodes are ordered according to the depth-first post-order.
   * 
   *  `f` should be a unary operator that takes one argument of type `ThisPtr`.
   */
  template<bool constant, class FunctionType>
  static constexpr void traverse_postorder(
      CondThisPtr<constant> n,
      FunctionType f) {
    if (n) {
      traverse_postorder<constant, FunctionType>(n->left_child, f);
      traverse_postorder<constant, FunctionType>(n->right_child, f);
      f(n);
    }
  }

  /**
   *  @brief
   *  Calls `traverse_postorder(this, f);
   */
  template<class FunctionType>
  constexpr void traverse_postorder(FunctionType f) {
    traverse_postorder<false, FunctionType>(this, f);
  }

  /**
   *  @brief
   *  Calls `traverse_postorder(this, f);
   */
  template<class FunctionType>
  constexpr void traverse_postorder(FunctionType f) const {
    traverse_postorder<true, FunctionType>(this, f);
  }

  /**
   *  @brief
   *  Applies the unary function `f` to each node in the subtree rooted at
   *    `n`.
   *  The nodes are ordered according to the depth-first pre-order.
   * 
   *  `f` should be a unary operator that takes one argument of type `ThisPtr`.
   */
  template<bool constant, class FunctionType>
  static constexpr void traverse_preorder(
      CondThisPtr<constant> n,
      FunctionType f) {
    if (n) {
      f(n);
      traverse_preorder<constant, FunctionType>(n->left_child, f);
      traverse_preorder<constant, FunctionType>(n->right_child, f);
    }
  }

  /**
   *  @brief
   *  Calls `traverse_preorder(this, f);
   */
  template<class FunctionType>
  constexpr void traverse_preorder(FunctionType f) {
    traverse_preorder<false, FunctionType>(this, f);
  }

  /**
   *  @brief
   *  Calls `traverse_preorder(this, f);
   */
  template<class FunctionType>
  constexpr void traverse_preorder(FunctionType f) const {
    traverse_preorder<true, FunctionType>(this, f);
  }

  /**
   *  @brief
   *  Returns the index of `this` node relative to the root.
   */
  constexpr size_type get_index() const {
    size_type index{get_size(left_child)};
    ConstThisPtr n{this};
    while (true) {
      ConstThisPtr p{n->parent};
      if (!p) {
        return index;
      }
      if (p->right_child == n) {
        index += get_size(p->left_child) + 1;
      }
      n = p;
    }
  }

  /**
   *  @brief
   *  Finds a node at a given integer `index` in a subtree rooted at `n`.
   */
  template<bool constant>
  static constexpr CondThisPtr<constant> find_node_at_index(
      CondThisPtr<constant> n,
      size_type index) {
    if (!n) {
      return nullptr;
    }
    if (n->size <= index) {
      return nullptr;
    }
    while (true) {
      auto l{n->left_child};
      if (l) {
        if (index < l->size) {
          n = l;
          continue;
        }
        index -= l->size;
      }
      if (index == 0) {
        return n;
      }
      --index;
      n = n->right_child;
      assert(n);
    }
  }

  /**
   *  @brief
   *  Calls `find_note_at_index(this, index)`.
   */
  constexpr ThisPtr find_node_at_index(size_type index) {
    return find_node_at_index<false>(this, index);
  }

  /**
   *  @brief
   *  Calls `find_note_at_index(this, index)`.
   */
  constexpr ConstThisPtr find_node_at_index(size_type index) const {
    return find_node_at_index<true>(this, index);
  }

  /**
   *  @brief
   *  Finds the leftmost node in the subtree rooted at `n`.
   */
  template<bool constant>
  static constexpr CondThisPtr<constant> find_first_node(
      CondThisPtr<constant> n) {
    assert(n);
    for (; n->left_child; n = n->left_child) {}
    return n;
  }

  /**
   *  @brief
   *  Calls `find_first_node(this)`.
   */
  constexpr ThisPtr find_first_node() {
    return find_first_node<false>(this);
  }

  /**
   *  @brief
   *  Calls `find_first_node(this)`.
   */
  constexpr ConstThisPtr find_first_node() const {
    return find_first_node<true>(this);
  }

  /**
   *  @brief
   *  Finds the node that would succeed `n` in an in-order traversal.
   */
  template<bool constant>
  static constexpr CondThisPtr<constant> find_next_node(
      CondThisPtr<constant> n) {
    assert(n);
    if (n->right_child) {
      return n->right_child->find_first_node();
    }
    while (true) {
      switch (n->get_child_type()) {
        case kNotChild:
          return nullptr;
        case kLeftChild:
          return n->parent;
        default:
          n = n->parent;
          break;
      }
    }
  }

  /**
   *  @brief
   *  Calls `find_next_node(this)`.
   */
  constexpr ThisPtr find_next_node() {
    return find_next_node<false>(this);
  }

  /**
   *  @brief
   *  Calls `find_next_node(this)`.
   */
  constexpr ConstThisPtr find_next_node() const {
    return find_next_node<true>(this);
  }

  /**
   *  @brief
   *  Finds the node that would be `steps` positions after `n` in an in-order
   *    traversal.
   */
  template<bool constant>
  static constexpr CondThisPtr<constant> find_next_node(
      CondThisPtr<constant> n,
      size_type steps) {
    if (steps < 0) {
      return find_prev_node<constant>(n, -steps);
    }
    assert(n);
    while (true) {
      if (steps == 0) {
        return n;
      }
      // If the right child is big enough, move down to it.
      if (steps <= get_size(n->right_child)) {
        // The target node is somewhere under `n->right_child`.
        n = n->right_child;
        size_type displacement{get_size(n->left_child) + 1};
        while (true) {
          if (steps > displacement) {
            n = n->right_child;
            displacement += get_size(n->left_child) + 1;
          } else if (steps < displacement) {
            n = n->left_child;
            displacement -= get_size(n->right_child) + 1;
          } else {
            return n;
          }
        }
      }
      // If the right child isn't big enough, move up to the parent.
      switch (n->get_child_type()) {
        case kNotChild:
          // If there's no parent, return null.
          return nullptr;
        case kLeftChild:
          // If `n` is a left child, moving up to the parent means stepping
          // forwards, so we decrease `steps` accordingly.
          steps -= get_size(n->right_child) + 1;
          n = n->parent;
          break;
        default:
          // If `n` is a right child, moving up to the parent means stepping
          // backwards, so we increase `steps` accordingly.
          steps += get_size(n->left_child) + 1;
          n = n->parent;
          break;
      }
    }
  }

  /**
   *  @brief
   *  Calls `find_next_node(this, steps)`.
   */
  constexpr ThisPtr find_next_node(size_type steps) {
    return find_next_node<false>(this, steps);
  }

  /**
   *  @brief
   *  Calls `find_next_node(this, steps)`.
   */
  constexpr ConstThisPtr find_next_node(size_type steps) const {
    return find_next_node<true>(this, steps);
  }

  /**
   *  @brief
   *  Finds the rightmost node in the subtree rooted at `n`.
   */
  template<bool constant>
  static constexpr CondThisPtr<constant> find_last_node(
      CondThisPtr<constant> n) {
    assert(n);
    for (; n->right_child; n = n->right_child) {}
    return n;
  }

  /**
   *  @brief
   *  Calls `find_last_node(this)`.
   */
  constexpr ThisPtr find_last_node() {
    return find_last_node<false>(this);
  }

  /**
   *  @brief
   *  Calls `find_last_node(this)`.
   */
  constexpr ConstThisPtr find_last_node() const {
    return find_last_node<true>(this);
  }

  /**
   *  @brief
   *  Finds the node that would precede `n` in an in-order traversal.
   */
  template<bool constant>
  static constexpr CondThisPtr<constant> find_prev_node(
      CondThisPtr<constant> n) {
    assert(n);
    if (n->left_child) {
      return n->left_child->find_last_node();
    }
    while (true) {
      switch (n->get_child_type()) {
        case kNotChild:
          return nullptr;
        case kRightChild:
          return n->parent;
        default:
          n = n->parent;
          break;
      }
    }
  }

  /**
   *  @brief
   *  Calls `find_prev_node(this)`.
   */
  constexpr ThisPtr find_prev_node() {
    return find_prev_node<false>(this);
  }

  /**
   *  @brief
   *  Calls `find_prev_node(this)`.
   */
  constexpr ConstThisPtr find_prev_node() const {
    return find_prev_node<true>(this);
  }

  /**
   *  @brief
   *  Finds the node that would be `steps` positions before `n` in an
   *    in-order traversal.
   */
  template<bool constant>
  static constexpr CondThisPtr<constant> find_prev_node(
      CondThisPtr<constant> n,
      size_type steps) {
    if (steps < 0) {
      return find_next_node<constant>(n, -steps);
    }
    assert(n);
    while (true) {
      if (steps == 0) {
        return n;
      }
      // If the left child is big enough, move down to it.
      if (steps <= get_size(n->left_child)) {
        // The target node is somewhere under `n->left_child`.
        n = n->left_child;
        size_type displacement{get_size(n->right_child) + 1};
        while (true) {
          if (steps > displacement) {
            n = n->left_child;
            displacement += get_size(n->right_child) + 1;
          } else if (steps < displacement) {
            n = n->right_child;
            displacement -= get_size(n->left_child) + 1;
          } else {
            return n;
          }
        }
      }
      // If the left child isn't big enough, move up to the parent.
      switch (n->get_child_type()) {
        case kNotChild:
          // If there's no parent, return null.
          return nullptr;
        case kRightChild:
          // If `n` is a right child, moving up to the parent means stepping
          // forwards, so we decrease `steps` accordingly.
          steps -= get_size(n->left_child) + 1;
          n = n->parent;
          break;
        default:
          // If `n` is a left child, moving up to the parent means stepping
          // backwards, so we increase `steps` accordingly.
          steps += get_size(n->right_child) + 1;
          n = n->parent;
          break;
      }
    }
  }

  /**
   *  @brief
   *  Calls `find_prev_node(this, steps)`.
   */
  constexpr ThisPtr find_prev_node(size_type steps) {
    return find_prev_node<false>(this, steps);
  }

  /**
   *  @brief
   *  Calls `find_prev_node(this, steps)`.
   */
  constexpr ConstThisPtr find_prev_node(size_type steps) const {
    return find_prev_node<true>(this, steps);
  }

  /**
   *  @brief
   *  Unifies `find_next_node()` and `find_prev_node()` by allowing `steps` to
   *    be an arbitrary, possibly signed, integer type.
   */
  template<bool constant, class Integer>
  static constexpr CondThisPtr<constant> find_node_displaced_by(
      CondThisPtr<constant> n,
      Integer steps) {
    if (steps > 0) {
      return n->template find_next_node<constant>(
          static_cast<size_type>(steps));
    }
    if (steps < 0) {
      return n->template find_prev_node<constant>(
          static_cast<size_type>(-steps));
    }
    return n;
  }

  /**
   *  @brief
   *  Calls `find_node_displaced_by(this, steps)`.
   */
  template<class Integer>
  constexpr ThisPtr find_node_displaced_by(Integer steps) {
    return find_node_displaced_by<false>(this, steps);
  }

  /**
   *  @brief
   *  Calls `find_node_displaced_by(this, steps)`.
   */
  template<class Integer>
  constexpr ConstThisPtr find_node_displaced_by(Integer steps) const {
    return find_node_displaced_by<true>(this, steps);
  }

  /**
   *  @brief
   *  Creates an `InsertPosition` for a child of `this` node. `left`
   *    specifies whether the `InsertPosition` will be for `left_child` or
   *    `right_child`.
   *
   *  This function simply calls the constructor of `InsertPosition`.
   */
  constexpr InsertPosition make_insert_position(bool left) {
    return InsertPosition{this, left};
  }

  /**
   *  @brief
   *  Finds `InsertPosition` for a given integer `index` in a subtree rooted
   *    at `this`.
   *
   *  This function assumes that `size` at every node is up to date.
   *
   *  After inserting a node at the returned `InsertPosition`, `index` will
   *    be the index of that node.
   */
  constexpr InsertPosition get_insert_position_for_index(size_type index) {
    ThisPtr n{this};
    while (true) {
      ThisPtr l{n->left_child};
      if (l) {
        if (index <= l->size) {
          n = l;
          continue;
        }
        index -= l->size + 1;
      } else if (index == 0) {
        return {n, true};
      } else {
        --index;
      }
      ThisPtr r{n->right_child};
      if (r) {
        n = r;
      } else {
        return {n, false};
      }
    }
  }

  /**
   *  @brief
   *  Finds `InsertPosition` for a node that would be a new immediate
   *    predecessor of this node after insertion.
   */
  constexpr InsertPosition get_prev_insert_position() {
    if (!left_child) {
      return {this, true};
    }
    return {find_prev_node(), false};
  }

  /**
   *  @brief
   *  Finds `InsertPosition` for a node that would be a new immediate successor
   *    of this node after insertion.
   */
  constexpr InsertPosition get_next_insert_position() {
    if (!right_child) {
      return {this, false};
    }
    return {find_next_node(), true};
  }

  /**
   *  @brief
   *  Links `this` node as a child of another node given in `pos`.
   *
   *  `pos.node` must be non-null.
   *  `parent` will be overwritten by `pos.node`.
   *
   *  If `update_sizes` is `true`, `parent->update_sizes_upwards()` will be
   *    called afterwards.
   */
  template<bool update_sizes = true>
  constexpr void link(InsertPosition const& pos) {
    assert(pos.node);
    ThisPtr p{pos.node};
    if (pos.left_child) {
      assert(!p->left_child);
      p->left_child = this;
      parent = p;
      if constexpr (update_sizes) {
        p->update_sizes_upwards();
      }
      return;
    }
    assert(!p->right_child);
    p->right_child = this;
    parent = p;
    if constexpr (update_sizes) {
      p->update_sizes_upwards();
    }
  }

  /**
   *  @brief
   *  Inserts `n` as the `index`-th node in the subtree rooted at `this`.
   *
   *  This function assumes that `n` has the correct size.
   */
  template<bool update_sizes = true>
  constexpr void link_at_index(size_type index, ThisPtr n) {
    assert(n);
    n->template link<update_sizes>(get_insert_position_for_index(index));
  }

  /**
   *  @brief
   *  Unlinks `this` node from its parent and returns `InsertPosition` for
   *    the position where the node used to be.
   *
   *  If `this` node doesn't have a parent, this function does nothing, and the
   *    return value will be `InsertPosition` with null `node`.
   */
  template<bool update_sizes = true>
  constexpr InsertPosition unlink() {
    switch (get_child_type()) {
      case kLeftChild: {
        parent->left_child = nullptr;
        if constexpr (update_sizes) {
          parent->update_sizes_upwards();
        }
        InsertPosition pos{parent, true};
        parent = nullptr;
        return pos;
      }
      case kRightChild: {
        parent->right_child = nullptr;
        if constexpr (update_sizes) {
          parent->update_sizes_upwards();
        }
        InsertPosition pos{parent, false};
        parent = nullptr;
        return pos;
      }
      default:
        return {};
    }
  }

  /**
   *  @brief
   *  Unlinks a node at a given index in the subtree rooted at `this` from its
   *    parent, then returns the node and `InsertPosition` for the position
   *    where the node used to be.
   */
  template<bool update_sizes = true>
  constexpr std::pair<ThisPtr, InsertPosition> unlink_at_index(
      size_type index) {
    ThisPtr n{find_node_at_index(index)};
    assert(n);
    InsertPosition pos{n->template unlink<update_sizes>()};
    return {n, pos};
  }

  /**
   *  @brief
   *  Rotates the subtree rooted at `this` to the left.
   *
   *  `right_child` must not be null before the call.
   *  After the call, `parent` (formerly `right_child`) will be the new root
   *    of the subtree.
   *
   *  This function does not update sizes of rotated nodes.
   *  The caller can manually call `update_size()` and
   *    `parent->update_size()` afterwards.
   *  (`parent` is guaranteed to be non-null.)
   */
  constexpr void rotate_left() {
    assert(right_child);
    ThisPtr p{parent};
    ChildType child_type{get_child_type()};

    ThisPtr r{right_child};
    ThisPtr rl{r->left_child};

    right_child = rl;
    if (rl) {
      rl->parent = this;
    }
    r->left_child = this;
    parent = r;

    if (child_type == kLeftChild) {
      p->left_child = r;
    } else if (child_type == kRightChild) {
      p->right_child = r;
    }
    r->parent = p;
  }

  /**
   *  @brief
   *  Rotates the subtree rooted at `this` to the right.
   *
   *  `left_child` must not be null before the call.
   *  After the call, `parent` (formerly `left_child`) will be the new root
   *    of the subtree.
   *
   *  This function does not update sizes of rotated nodes.
   *  The caller can manually call `update_size()` and
   *    `parent->update_size()` afterwards.
   *  (`parent` is guaranteed to be non-null.)
   */
  constexpr void rotate_right() {
    assert(left_child);
    ThisPtr p{parent};
    ChildType child_type{get_child_type()};

    ThisPtr r{left_child};
    ThisPtr rl{r->right_child};

    left_child = rl;
    if (rl) {
      rl->parent = this;
    }
    r->right_child = this;
    parent = r;

    if (child_type == kRightChild) {
      p->right_child = r;
    } else if (child_type == kLeftChild) {
      p->left_child = r;
    }
    r->parent = p;
  }

  /**
   *  @brief
   *  Performs a depth-one splay step, and returns the former value of
   *    `parent`.
   *
   *  This function moves up `this` node in the tree by one position while
   *    maintaining the order of all the nodes.
   *  `parent` must not be null before calling this function.
   * 
   *  The return value is the value of `parent` prior to the call.
   *  This is the node whose data might need to be updated before updating
   *    `this`.
   */
  constexpr ThisPtr splay_1() {
    assert(parent);
    ThisPtr p{parent};
    ChildType child_type{get_child_type()};
    if (child_type == kLeftChild) {
      parent->rotate_right();
    } else {
      parent->rotate_left();
    }
    return p;
  }

  /**
   *  @brief
   *  Performs a depth-two splay step, and returns the former value of
   *    `parent->parent`.
   *
   *  This function moves up `this` node in the tree by two positions while
   *    maintaining the order of all the nodes.
   *  `parent` and `parent->parent` must not be null before calling this
   *    function.
   *
   *  The return value is `{parent->parent, parent}` prior to the call.
   *  These are nodes whose data might need to be updated before updating
   *    `this`.
   *  The former `parent->parent` may become a child of the former `parent`
   *    but the converse is not possible, so it is safer to always
   *    update the `first` component of the return value before `second`.
   */
  constexpr std::pair<ThisPtr, ThisPtr> splay_2() {
    assert(parent);
    assert(parent->parent);
    ThisPtr p{parent};
    ThisPtr pp{p->parent};
    ThisPtr ppp{pp->parent};
    ChildType child_type{get_child_type()};
    ChildType p_child_type{p->get_child_type()};
    ChildType pp_child_type{pp->get_child_type()};
    if (p_child_type == kLeftChild) {
      if (child_type == kLeftChild) {
        // left-left zig-zig
        ThisPtr s{p->right_child}; // s = sibling
        pp->left_child = s;
        if (s) {
          s->parent = pp;
        }
        p->right_child = pp;
        pp->parent = p;
        p->left_child = right_child;
        if (right_child) {
          right_child->parent = p;
        }
        right_child = p;
        p->parent = this;
      } else {
        // left-right zig-zag
        ThisPtr left{left_child};
        ThisPtr right{right_child};
        p->right_child = left;
        if (left) {
          left->parent = p;
        }
        left_child = p;
        p->parent = this;
        pp->left_child = right;
        if (right) {
          right->parent = pp;
        }
        right_child = pp;
        pp->parent = this;
      }
    } else if (child_type == kLeftChild) {
      // right-left zig-zag
      ThisPtr left{left_child};
      ThisPtr right{right_child};
      p->left_child = right;
      if (right) {
        right->parent = p;
      }
      right_child = p;
      p->parent = this;
      pp->right_child = left;
      if (left) {
        left->parent = pp;
      }
      left_child = pp;
      pp->parent = this;
    } else {
      // right-right zig-zig
      ThisPtr s{p->left_child};
      pp->right_child = s;
      if (s) {
        s->parent = pp;
      }
      p->left_child = pp;
      pp->parent = p;
      p->right_child = left_child;
      if (left_child) {
        left_child->parent = p;
      }
      left_child = p;
      p->parent = this;
    }

    parent = ppp;
    if (pp_child_type == kLeftChild) {
      ppp->left_child = this;
    } else if (pp_child_type == kRightChild) {
      ppp->right_child = this;
    }

    return {pp, p};
  }

  /**
   *  @brief
   *  Splays a node upwards, stopping before touching `top`.
   *  If `top` is null, `this` will become a new root.
   *  Otherwise, `this` will become an immediate child of `top`.
   *
   *  The function `f` for *updating* node data along the way can be
   *    customized.
   *  `f` should take one argument of type `ThisPtr`.
   *  The return value of `f` is not used.
   */
  template<class FunctionType>
  constexpr void splay(FunctionType f, ThisPtr top = nullptr) {
    assert(!top || is_under(top));
    while (parent != top) {
      if (parent->parent != top) {
        std::pair<ThisPtr, ThisPtr> pp_p{splay_2()};
        f(pp_p.first);
        f(pp_p.second);
      } else {
        ThisPtr p{splay_1()};
        f(p);
      }
    }
    f(this);
  }

  /**
   *  @brief
   *  Overload of `splay()` that supplies either the no-op function or
   *    `update_node_size` as the update function, based on the template
   *    parameter `update_sizes`.
   */
  template<bool update_sizes = true>
  constexpr void splay(ThisPtr top = nullptr) {
    if constexpr (update_sizes) {
      splay(This::update_node_size, top);
    } else {
      splay([](ThisPtr) {}, top);
    }
  }

  /**
   *  @brief
   *  Swaps two nodes' connections (`parent`, `left_child`, and
   *    `right_child`) as well as sizes.
   *
   *  This function can be used to virtually swap node's `data` without
   *    actually moving `data`.
   */
  constexpr void swap(ThisPtr n) {
    assert(n);
    std::swap(size, n->size);
    ChildType child_type{get_child_type()};
    ChildType n_child_type{n->get_child_type()};
    std::swap(parent, n->parent);
    switch (n_child_type) {
      case kLeftChild:
        parent->left_child = this;
        break;
      case kRightChild:
        parent->right_child = this;
        break;
      default:
        break;
    }
    switch (child_type) {
      case kLeftChild:
        n->parent->left_child = n;
        break;
      case kRightChild:
        n->parent->right_child = n;
        break;
      default:
        break;
    }
    std::swap(left_child, n->left_child);
    if (left_child) {
      left_child->parent = this;
    }
    if (n->left_child) {
      n->left_child->parent = n;
    }
    std::swap(right_child, n->right_child);
    if (right_child) {
      right_child->parent = this;
    }
    if (n->right_child) {
      n->right_child->parent = n;
    }
  }

  /**
   *  @brief
   *  Erases `this` node from the tree and returns two nodes: a node that
   *    *replaces* `this`, and a node whose path towards the root contains
   *    nodes whose data might need to be updated.
   *
   *  This erase operation will disconnect `this` node from the tree it is
   *    currently in while maintaining the order of the rest of the tree.
   *  `this` node's members will not be modified, i.e., `parent`, `left_child`,
   *    `right_child` and `size` will remain unchanged.
   *
   *  If `this` node is a leaf, there will be no node that replaces `this`.
   *  In this case, the `first` component of the return value will be null.
   *  Otherwise, another node in the tree will take the place of `this` and
   *    will be returned as the `first` component of the return value.
   *  This information can be used to track when the root node is changed.
   * 
   *  Because an erase operation may invalidate some nodes' data, this function
   *    will return a node whose path towards the root contains nodes whose
   *    data may need to be updated as the `second` component of the return
   *    value.
   *  If `this` node is the only node in the tree, the `second` component of
   *    the return value will be null.
   */
  template<bool update_sizes = true>
  constexpr std::pair<ThisPtr, ThisPtr> erase() {
    if (!left_child) {
      ChildType child_type{get_child_type()};
      if (child_type == kLeftChild) {
        parent->left_child = right_child;
      } else if (child_type == kRightChild) {
        parent->right_child = right_child;
      }
      if (right_child) {
        right_child->parent = parent;
      }
      if constexpr (update_sizes) {
        if (parent) {
          parent->update_sizes_upwards();
        }
      }
      return {right_child, parent};
    } else if (!right_child) {
      ChildType child_type{get_child_type()};
      if (child_type == kLeftChild) {
        parent->left_child = left_child;
      } else if (child_type == kRightChild) {
        parent->right_child = left_child;
      }
      left_child->parent = parent;
      if constexpr (update_sizes) {
        if (parent) {
          parent->update_sizes_upwards();
        }
      }
      return {left_child, parent};
    }

    ThisPtr next{find_next_node()};
    assert(next);
    swap(next);
    assert(parent);
    ChildType child_type{get_child_type()};
    if (child_type == kLeftChild) {
      parent->left_child = right_child;
    } else {
      assert(child_type == kRightChild);
      parent->right_child = right_child;
    }
    if (right_child) {
      right_child->parent = parent;
    }
    if constexpr (update_sizes) {
      parent->update_sizes_upwards();
    }
    return {next, parent};
  }

  /**
   *  @brief
   *  Erases a node at a given index in a subtree rooted at `this` node.
   *
   *  This function essentially calls `find_node_at_index(index)->erase()` and
   *    returns all the values during the operation.
   *
   *  The return value has three components:
   *  - [0]: the node at the given index prior to calling this function.
   *  - [1]: the first component of the return value of `erase()`.
   *  - [2]: the second component of the return value of `erase()`.
   */
  template<bool update_sizes = true>
  constexpr std::tuple<ThisPtr, ThisPtr, ThisPtr> erase_at_index(size_type index) {
    ThisPtr n{find_node_at_index(index)};
    std::pair<ThisPtr, ThisPtr> erase_result{n->erase()};
    return {n, erase_result.first, erase_result.second};
  }

};

} // namespace ordered_binary_trees
