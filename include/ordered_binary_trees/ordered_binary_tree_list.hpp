#pragma once

#include <optional>

namespace ordered_binary_trees {

template<class TreeT>
class OrderedBinaryTreeList {
 private:
  /// This type.
  using This = OrderedBinaryTreeList<TreeT>;

 protected:
  /// `TreeT`.
  using Tree = TreeT;

  /// `Tree::Node`.
  using Node = typename Tree::Node;

  /// `Tree::Allocator`.
  using Allocator = typename Tree::Allocator;

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

  /// Internal tree representation.
  Tree tree_;

 public:

  using value_type = Data;

  OrderedBinaryTree() = default;
  OrderedBinaryTree(This const& other)
    : tree_{other.tree_.clone()} {}

  OrderedBinaryTree(This&& other)
    : tree_{std::move(other.tree_)} {}

  ~OrderedBinaryTree() {
    tree_.destroy_all_nodes();
  }



};

} // namespace ordered_binary_trees
