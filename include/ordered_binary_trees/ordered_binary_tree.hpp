#include <algorithm>
#include <cstdint>
#include <memory>
#include <type_traits>

#include <ordered_binary_trees/assert.hpp>

namespace ordered_binary_trees {

template<class DataT, class SizeT = std::size_t>
struct OrderedBinaryTree {
  /// This type.
  using This = OrderedBinaryTree<DataT, SizeT>;
  /// `DataT`.
  using Data = DataT;
  /// `SizeT`.
  using size_type = SizeT;

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

  // Forward declaration of `Node` as needed by `InsertPosition`.
  struct Node;

  /**
   *  @brief
   *  Location of a prospective new node.
   */
  struct InsertPosition {
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
    constexpr InsertPosition(Node* node, bool left_child)
      : node{node}, left_child{left_child} {
    }

    /// Parent of the prospective new node. May be null for the root node.
    Node* node;
    /// Whether the prospective new node will be a left child of `node`.
    bool left_child;
  };

  /**
   *  @brief
   *  Type of nodes that supports integer indexing.
   */
  struct Node {
    /// Type of the tree containing nodes of type `Node`.
    using Tree = OrderedBinaryTree<DataT, SizeT>;
    /// This type.
    using This = Node;
    /// Same as `Tree::Data`.
    using Data = DataT;
    /// Same as `Tree::size_type`.
    using size_type = SizeT;

    /// Left child. May be null.
    Node* left_child{nullptr};
    /// Right child. May be null.
    Node* right_child{nullptr};
    /// Parent. May be null.
    Node* parent;
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
     *  Creates a `Node` with a given `parent` and initializes `data` from
     *    `args`.
     */
    template<class... Args>
    constexpr Node(Node* parent, Args&&... args)
      : parent{parent},
        data(std::forward<Args>(args)...) {}
    
    /**
     *  @brief
     *  Constructs a disconnected `Node` with `data` initialized from `args`.
     *
     *  This is a convenience function that simply calls the constructor with
     *    `parent` set to `nullptr`.
     */
    template<class... Args>
    static constexpr Node* with_data(Args&&... args) {
      return new Node(nullptr, std::forward<Args>(args)...);
    }

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
    static constexpr size_type get_size(Node* n) {
      return n ? n->size : 0;
    }

    /**
     *  @brief
     *  Swaps two nodes' connections (`parent`, `left_child`, and
     *    `right_child`) as well as sizes.
     *
     *  This function can be used to virtually swap node's `data` without
     *    actually moving `data`.
     */
    constexpr void swap(Node* n) {
      ASSERT(n);
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
     *  Applies the unary function `f` to nodes along the path from `n` to the
     *    root, stopping if `f()` returns `false`.
     *  The return value of `traverse_upwards()` is the last node for which
     *    `f()` returns `true`.
     */
    template<bool constant, class FunctionType>
    static constexpr std::conditional_t<constant, Node const, Node>*
        traverse_upwards(
          std::conditional_t<constant, Node const, Node>* n,
          FunctionType f) {
      ASSERT(n);
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
    constexpr Node* traverse_upwards(FunctionType f) {
      return traverse_upwards<false, FunctionType>(this, f);
    }

    /**
     *  @brief
     *  Calls `traverse_upwards(this, f)`.
     */
    template<class FunctionType>
    constexpr Node const* traverse_upwards(FunctionType f) const {
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
    static constexpr bool update_node_size(Node* n) {
      return n->update_size();
    }

    /**
     *  @brief
     *  Calls `traverse_upwards()` with `update_node_size()` as the unary
     *    function.
     */
    constexpr Node* update_sizes_upwards() {
      return traverse_upwards(update_node_size);
    }
  
    /**
     *  @brief
     *  Applies the unary function `f` to each node in the subtree rooted at
     *    `n`.
     *  The nodes are ordered according to the depth-first in-order.
     * 
     *  `f` should be a unary operator that takes one argument of type `Node*`.
     */
    template<bool constant, class FunctionType>
    static constexpr void traverse_inorder(
        std::conditional_t<constant, Node const, Node>* n,
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
     *  `f` should be a unary operator that takes one argument of type `Node*`.
     */
    template<bool constant, class FunctionType>
    static constexpr void traverse_postorder(
        std::conditional_t<constant, Node const, Node>* n,
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
     *  `f` should be a unary operator that takes one argument of type `Node*`.
     */
    template<bool constant, class FunctionType>
    static constexpr void traverse_preorder(
        std::conditional_t<constant, Node const, Node>* n,
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
     *  Deletes all nodes under the subtree rooted at `n`.
     */
    static constexpr void delete_nodes(Node* n) {
      traverse_postorder<false>(n, [](Node* n) { delete n; });
    }

    /**
     *  @brief
     *  Clones all nodes under the subtree rooted at `this` node.
     */
    constexpr Node* clone() const {
      Node* n{Node::with_data(data)};
      n->size = size;
      if (left_child) {
        Node* n_left_child{left_child->clone()};
        n->left_child = n_left_child;
        n_left_child->parent = n;
      }
      if (right_child) {
        Node* n_right_child{right_child->clone()};
        n->right_child = n_right_child;
        n_right_child->parent = n;
      }
      return n;
    }

    /**
     *  @brief
     *  Creates an `InsertPosition` for a child of `this` node. `left`
     *    specifies whether the `InsertPosition` will be for `left_child` or
     *    `right_child`.
     *
     *  This function simply calls the constructor of `InsertPosition`.
     */
    constexpr InsertPosition get_insert_position(bool left) {
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
    constexpr InsertPosition find_insert_position_for_index(size_type index) {
      Node* n{this};
      while (true) {
        Node* l{n->left_child};
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
        Node* r{n->right_child};
        if (r) {
          n = r;
        } else {
          return {n, false};
        }
      }
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
      ASSERT(pos.node);
      Node* p{pos.node};
      if (pos.left_child) {
        ASSERT(!p->left_child);
        p->left_child = this;
        parent = p;
        if constexpr (update_sizes) {
          p->update_sizes_upwards();
        }
        return;
      }
      ASSERT(!p->right_child);
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
    constexpr void insert_at_index(size_type index, Node* n) {
      ASSERT(n);
      n->template link<update_sizes>(find_insert_position_for_index(index));
    }

    /**
     *  @brief
     *  Constructs a `Node` with given arguments, adds it as the `index`-th
     *    node in the subtree rooted at `this`, and returns the pointer to it.
     *
     *  Note that a `Node` created this way will not be automatically deleted
     *    when the tree is destroyed.
     */
    template<bool update_sizes = true, class... Args>
    constexpr Node* emplace_at_index(size_type index, Args&&... args) {
      Node* n{new Node(nullptr, std::forward<Args>(args)...)};
      insert_at_index<update_sizes>(index, n);
      return n;
    }

    /**
     *  @brief
     *  Unlinks `this` node from its parent and returns `InsertPosition` for
     *    the position where the node was.
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

    template<bool update_sizes = true>
    constexpr Node* unlink_at_index(size_type index) {
      Node* n{find_node_at_index(index)};
      ASSERT(n);
      n->template unlink<update_sizes>();
      return n;
    }

    /**
     *  @brief
     *  Returns the index of `this` node relative to the root.
     */
    constexpr size_type get_index() const {
      size_type index{get_size(left_child)};
      Node const* n{this};
      while (true) {
        Node const* p{n->parent};
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
    static constexpr std::conditional_t<constant, Node const, Node>*
        find_node_at_index(
          std::conditional_t<constant, Node const, Node>* n,
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
        ASSERT(n);
      }
    }

    /**
     *  @brief
     *  Calls `find_note_at_index(this, index)`.
     */
    constexpr Node* find_node_at_index(size_type index) {
      return find_node_at_index<false>(this, index);
    }

    /**
     *  @brief
     *  Calls `find_note_at_index(this, index)`.
     */
    constexpr Node const* find_node_at_index(size_type index) const {
      return find_node_at_index<true>(this, index);
    }

    /**
     *  @brief
     *  Finds the leftmost node in the subtree rooted at `n`.
     */
    template<bool constant>
    static constexpr std::conditional_t<constant, Node const, Node>*
        find_first_node(std::conditional_t<constant, Node const, Node>* n) {
      ASSERT(n);
      for (; n->left_child; n = n->left_child) {}
      return n;
    }

    /**
     *  @brief
     *  Calls `find_first_node(this)`.
     */
    constexpr Node* find_first_node() {
      return find_first_node<false>(this);
    }

    /**
     *  @brief
     *  Calls `find_first_node(this)`.
     */
    constexpr Node const* find_first_node() const {
      return find_first_node<true>(this);
    }

    /**
     *  @brief
     *  Finds the node that would succeed `n` in an in-order traversal.
     */
    template<bool constant>
    static constexpr std::conditional_t<constant, Node const, Node>*
        find_next_node(std::conditional_t<constant, Node const, Node>* n) {
      ASSERT(n);
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
    constexpr Node* find_next_node() {
      return find_next_node<false>(this);
    }

    /**
     *  @brief
     *  Calls `find_next_node(this)`.
     */
    constexpr Node const* find_next_node() const {
      return find_next_node<true>(this);
    }

    /**
     *  @brief
     *  Finds the node that would be `steps` positions after `n` in an in-order
     *    traversal.
     */
    template<bool constant>
    static constexpr std::conditional_t<constant, Node const, Node>*
        find_next_node(
          std::conditional_t<constant, Node const, Node>* n,
          size_type steps) {
      ASSERT(n);
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
    constexpr Node* find_next_node(size_type steps) {
      return find_next_node<false>(this, steps);
    }

    /**
     *  @brief
     *  Calls `find_next_node(this, steps)`.
     */
    constexpr Node const* find_next_node(size_type steps) const {
      return find_next_node<true>(this, steps);
    }

    /**
     *  @brief
     *  Finds the rightmost node in the subtree rooted at `n`.
     */
    template<bool constant>
    static constexpr std::conditional_t<constant, Node const, Node>*
        find_last_node(std::conditional_t<constant, Node const, Node>* n) {
      ASSERT(n);
      for (; n->right_child; n = n->right_child) {}
      return n;
    }

    /**
     *  @brief
     *  Calls `find_last_node(this)`.
     */
    constexpr Node* find_last_node() {
      return find_last_node<false>(this);
    }

    /**
     *  @brief
     *  Calls `find_last_node(this)`.
     */
    constexpr Node const* find_last_node() const {
      return find_last_node<true>(this);
    }

    /**
     *  @brief
     *  Finds the node that would precede `n` in an in-order traversal.
     */
    template<bool constant>
    static constexpr std::conditional_t<constant, Node const, Node>*
        find_prev_node(std::conditional_t<constant, Node const, Node>* n) {
      ASSERT(n);
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
    constexpr Node* find_prev_node() {
      return find_prev_node<false>(this);
    }

    /**
     *  @brief
     *  Calls `find_prev_node(this)`.
     */
    constexpr Node const* find_prev_node() const {
      return find_prev_node<true>(this);
    }

    /**
     *  @brief
     *  Finds the node that would be `steps` positions before `n` in an
     *    in-order traversal.
     */
    template<bool constant>
    static constexpr std::conditional_t<constant, Node const, Node>*
        find_prev_node(
          std::conditional_t<constant, Node const, Node>* n,
          size_type steps) {
      ASSERT(n);
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
    constexpr Node* find_prev_node(size_type steps) {
      return find_prev_node<false>(this, steps);
    }

    /**
     *  @brief
     *  Calls `find_prev_node(this, steps)`.
     */
    constexpr Node const* find_prev_node(size_type steps) const {
      return find_prev_node<true>(this, steps);
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
      ASSERT(right_child);
      This* p{parent};
      ChildType child_type{get_child_type()};

      This* r{right_child};
      This* rl{r->left_child};

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
      ASSERT(left_child);
      This* p{parent};
      ChildType child_type{get_child_type()};

      This* r{left_child};
      This* rl{r->right_child};

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
    constexpr Node* splay_1() {
      ASSERT(parent);
      Node* p{parent};
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
     *  Node that the former `parent->parent` may become a child of the former
     *    `parent`, so it is safer to update the `first` component of the
     *    return value before `second`.
     */
    constexpr std::pair<Node*, Node*> splay_2() {
      ASSERT(parent);
      ASSERT(parent->parent);
      Node* p{parent};
      Node* pp{p->parent};
      Node* ppp{pp->parent};
      ChildType child_type{get_child_type()};
      ChildType p_child_type{p->get_child_type()};
      ChildType pp_child_type{pp->get_child_type()};
      if (p_child_type == kLeftChild) {
        if (child_type == kLeftChild) {
          // left-left zig-zig
          Node* s{p->right_child}; // s = sibling
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
          Node* left{left_child};
          Node* right{right_child};
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
        Node* left{left_child};
        Node* right{right_child};
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
        Node* s{p->left_child};
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
     *  Splays a node up to be the new root.
     * 
     *  The function `f` for *updating* node data along the way can be
     *    customized.
     *  `f` should take on argument of type `Node*`.
     *  The return value of `f` is not used.
     */
    template<class FunctionType>
    constexpr void splay(FunctionType f) {
      while (parent) {
        if (parent->parent) {
          std::pair<Node*, Node*> pp_p{splay_2()};
          f(pp_p.first);
          f(pp_p.second);
        } else {
          Node* p{splay_1()};
          f(p);
        }
      }
      f(this);
    }

    constexpr void splay() {
      splay(Node::update_node_size);
    }

  }; // struct Node

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
   *  Calls `delete n` for every node `n` reachable from `root`.
   */
  constexpr void delete_nodes() {
    Node::delete_nodes(root);
  }

  constexpr void rotate_left(Node* n) {
    n->rotate_left();
    if (n == root) {
      root = root->parent;
    }
  }

  constexpr void rotate_right(Node* n) {
    n->rotate_right();
    if (n == root) {
      root = root->parent;
    }
  }

  template<class FunctionType>
  constexpr void splay(Node* n, FunctionType f) {
    ASSERT(root);
    ASSERT(n);
    n->splay(f);
    root = n;
  }

  constexpr void splay(Node* n) {
    splay(n, Node::update_node_size);
  }

  /**
   *  @brief
   *  Root of the tree. This is null when the tree is empty.
   */
  Node* root{nullptr};
};

} // namespace ordered_binary_trees
