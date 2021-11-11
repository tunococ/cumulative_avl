#include <algorithm>
#include <cassert>
#include <cstdint>
#include <memory>

namespace ordered_binary_trees {

template<class DataT, class SizeT = std::size_t>
struct OrderedBinaryTree {
  using This = OrderedBinaryTree<DataT, SizeT>;
  using Data = DataT;
  using size_type = SizeT;

  /**
   *  @brief
   *  Type of nodes that supports integer indexing.
   */
  struct Node {
    using Tree = This;
    using This = Node;
    using Data = DataT;
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
  };

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
   *  Whether a node is a root, a left child, or a right child.
   */
  enum ChildType {
    /// The node is a root.
    kNotChild = 0,
    /// The node is a left child of another node.
    kLeftChild = 1,
    /// The node is a right child of another node.
    kRightChild = 2
  };

  /**
   *  @brief
   *  Returns the `ChildType` of node `n`. `n` must not be null.
   */
  static constexpr ChildType get_child_type(Node* n) {
    assert((n));
    if (!n->parent) {
      return kNotChild;
    }
    return n->parent->left_child == n ? kLeftChild : kRightChild;
  }

  /**
   *  @brief
   *  Returns `true` iff `n` is a leaf node. `n` must not be null.
   */
  static constexpr bool is_leaf(Node* n) {
    assert((n));
    return !n->left_child && !n->right_child;
  }

  /**
   *  @brief
   *  Applies the unary function `f` to each node in the tree.
   *  The nodes are ordered according to the depth-first in-order.
   * 
   *  `f` should be a unary operator that takes one argument of type `Node*`.
   */
  template<class FunctionType>
  static constexpr void traverse_inorder(Node* n, FunctionType f) {
    if (n) {
      traverse_inorder(n->left_child, f);
      f(n);
      traverse_inorder(n->right_child, f);
    }
  }

  /**
   *  @brief
   *  Applies the unary function `f` to each node in the tree.
   *  The nodes are ordered according to the depth-first post-order.
   * 
   *  `f` should be a unary operator that takes one argument of type `Node*`.
   */
  template<class FunctionType>
  static constexpr void traverse_postorder(Node* n, FunctionType f) {
    if (n) {
      traverse_inorder(n->left_child, f);
      traverse_inorder(n->right_child, f);
      f(n);
    }
  }

  /**
   *  @brief
   *  Applies the unary function `f` to each node in the tree.
   *  The nodes are ordered according to the depth-first pre-order.
   * 
   *  `f` should be a unary operator that takes one argument of type `Node*`.
   */
  template<class FunctionType>
  static constexpr void traverse_preorder(Node* n, FunctionType f) {
    if (n) {
      f(n);
      traverse_inorder(n->left_child, f);
      traverse_inorder(n->right_child, f);
    }
  }

  /**
   *  @brief 
   *  Applies the unary function `update` to nodes along the path from `n` to
   *    the root, stopping if `update()` returns `false`.
   *  The return value is the last node for which `f()` returns `true`.
   */
  template<class Update>
  static constexpr Node* update_upwards(Node* n, Update update) {
    assert((n));
    if (!update(n)) {
      return nullptr;
    }
    while (true) {
      Node* p{n->parent};
      if (!p || !update(p)) {
        return n;
      }
      n = p;
    }
  }

  /**
   *  @brief
   *  Attempts to update the size at a single node and returns `true` iff the
   *    size actually changes. `n` must not be null.
   */
  static constexpr bool update_size(Node* n) {
    assert((n));
    const size_type new_size{
        1 + get_size(n->left_child) + get_size(n->right_child)};
    if (new_size != n->size) {
      n->size = new_size;
      return true;
    }
    return false;
  }

  /**
   *  @brief
   *  Calls `update_upwards()` with `update_size` as the update function.
   */
  static constexpr Node* update_sizes_upwards(Node* n) {
    assert((n));
    return update_upwards(n, update_size);
  }

  /**
   *  @brief
   *  Rotates the subtree rooted at `n` to the left.
   *
   *  `n->right_child` must not be null before the call.
   *  After the call, `n->parent` (formerly `n->right_child`) will be the new
   *    root of the subtree.
   *
   *  This function updates sizes of rotated nodes.
   */
  static constexpr void rotate_left(This* n) {
    assert((n && n->right_child));
    This* p{n->parent};
    ChildType child_type{get_child_type(n)};

    This* r{n->right_child};
    This* rl{r->left_child};

    n->right_child = rl;
    if (rl) {
      rl->parent = n;
    }
    r->left_child = n;
    n->parent = r;

    update_size(n);
    update_size(r);

    if (child_type == kLeftChild) {
      p->left_child = r;
    } else if (child_type == kRightChild) {
      p->right_child = r;
    }
    r->parent = p;
  }

  /**
   *  @brief
   *  Rotates the subtree rooted at `n` to the right.
   *
   *  `n->left_child` must not be null before the call.
   *  After the call, `n->parent` (formerly `n->left_child`) will be the new
   *    root of the subtree.
   *
   *  This function updates sizes of rotated nodes.
   */
  static constexpr void rotate_right(This* n) {
    assert((n && n->left_child));
    This* p{n->parent};
    ChildType child_type{get_child_type(n)};

    This* l{n->left_child};
    This* lr{l->right_child};

    n->left_child = lr;
    if (lr) {
      lr->parent = n;
    }
    l->right_child = n;
    n->parent = l;

    update_size(n);
    update_size(l);

    if (child_type == kRightChild) {
      p->right_child = l;
    } else if (child_type == kLeftChild) {
      p->left_child = l;
    }
    l->parent = p;
  }

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
      assert(((node && left_child) <= (!node->left_child)));
      assert(((node && !left_child) <= (!node->right_child)));
    }

    /// Parent of the prospective new node. May be null for the root node.
    Node* node;
    /// Whether the prospective new node will be a left child of `node`.
    bool left_child;
  };

  /**
   *  @brief
   *  Finds `InsertPosition` for a given integer `index` in a subtree rooted at
   *    `n`.
   *
   *  `n` must not be null.
   *  After inserting a node at the returned `InsertPosition`, `index` will be
   *    the index of that node.
   */
  static constexpr InsertPosition find_insert_position_for_index(
      Node* n,
      size_type index) {
    assert((n));
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
   *  Finds a node at a given integer `index` in a subtree rooted at `n`.
   */
  static constexpr Node* find_node_at_index(Node* n, size_type index) {
    if (!n) {
      return nullptr;
    }
    if (n->size <= index) {
      return nullptr;
    }
    while (true) {
      Node* l{n->left_child};
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
      assert((n));
    }
  }

  /**
   *  @brief
   *  Finds the leftmost node in the subtree rooted at `n`.
   */
  static constexpr Node* find_first_node(Node* n) {
    assert((n));
    for (; n->left_child; n = n->left_child) {}
    return n;
  }

  /**
   *  @brief
   *  Finds the node that would succeed `n` in an in-order traversal.
   */
  static constexpr Node* find_next_node(Node* n) {
    assert((n));
    if (n->right_child) {
      return find_first_node(n->right_child);
    }
    while (true) {
      switch (get_child_type(n)) {
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
   *  Finds the node that would be `steps` positions after `n` in an in-order
   *  traversal.
   */
  static constexpr Node* find_next_node(Node* n, size_type steps) {
    assert((n));
    while (true) {
      if (steps == 0) {
        return n;
      }
      if (steps <= get_size(n->right_child)) {
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
      switch (get_child_type(n)) {
        case kNotChild:
          return nullptr;
        case kLeftChild:
          steps -= get_size(n->right_child) + 1;
          n = n->parent;
          break;
        default:
          steps += get_size(n->left_child) + 1;
          n = n->parent;
          break;
      }
    }

    while (true) {
      if (steps == 0) {
        return n;
      }
      if (steps <= get_size(n->right)) {
        n = n->right;
        steps -= get_size(n->left) + 1;
      }
    }
  }

  /**
   *  @brief
   *  Finds the leftmost node in the subtree rooted at `n`.
   */
  static constexpr Node* find_last_node(Node* n) {
    assert((n));
    for (; n->right_child; n = n->right_child) {}
    return n;
  }

  /**
   *  @brief
   *  Finds the node that would precede `n` in an in-order traversal.
   */
  static constexpr Node* find_prev_node(Node* n) {
    assert((n));
    if (n->left_child) {
      return find_last_node(n->left_child);
    }
    while (true) {
      switch (get_child_type(n)) {
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
   *  Inserts node `n` at a given `InsertPosition` `pos`.
   *
   *  `pos` must not suggest creation of a root node.
   *  `n` must not be null. `n` does not have to be a leaf node.
   */
  static constexpr void insert_child(InsertPosition const& pos, Node* n) {
    assert((pos.node));
    Node* p{pos.node};
    if (pos.left_child) {
      assert((!p->left_child));
      p->left_child = n;
      n->parent = p;
      return;
    }
    assert((!p->right_child));
    p->right_child = n;
    n->parent = p;
  }

  /**
   *  @brief
   *  Creates a binary tree with a given root.
   */
  constexpr OrderedBinaryTree(Node* root = nullptr) : root{root} {}

  /**
   *  @brief
   *  Inserts `n` into the tree as the `index`-th node.
   */
  template<bool update_sizes = true>
  constexpr void insert_at_index(size_type index, Node* n) {
    assert((n));
    if (!root) {
      root = n;
    } else {
      insert_child(find_insert_position_for_index(root, index), n);
      if constexpr (update_sizes) {
        assert((n->parent));
        update_sizes_upwards(n->parent);
      }
    }
  }

  /**
   *  @brief
   *  Returns the number of nodes in the tree.
   */
  constexpr size_type size() const {
    return root ? 0 : root->size;
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
   *  Finds a node at a given integer `index` in the tree.
   */
  constexpr Node* find_node_at_index(size_type index) {
    return find_node_at_index(root, index);
  }

  Node* root{nullptr};
};

} // namespace ordered_binary_trees
