#pragma once

#include <type_traits>

#include <ordered_binary_trees/assert.hpp>

namespace ordered_binary_trees {

/**
 *  @brief
 *  Default collection of functions for extracting `Value&` from `Node::data`.
 *
 *  For trees that require the `data` field to store more information,
 *    different implementations may be required.
 *  For example, in an AVL tree_, `data` may be a pair of height and *value*,
 *    in which case `value()` should return only the value component.
 */
template<class NodeT>
struct DefaultExtractValue {
  /// `NodeT`.
  using Node = NodeT;
  /// `Node::Data`.
  using Data = typename Node::Data;
  /// Same as `Data`.
  using Value = Data;
  
  /// Returns `data`.
  static constexpr Value& value_in_data(Data& data) {
    return data;
  }
};

/**
 *  @brief
 *  Common iterator class for ordered binary trees.
 *
 *  This iterator class contains two pointers: `tree_` and `node_`.
 *  A past-the-end iterator has a null `node_`.
 *
 *  @tparam TreeT
 *    Type of the tree_.
 *    Generally, this would be an instance of `OrderedBinaryTree`.
 *  @tparam ExtractValueT
 *    Type that contains static functions for extracting *value* from
 *      `node_->data`.
 *    `DefaultExtractValue` is one example.
 */
template<
    class TreeT,
    bool constant,
    bool reverse = false,
    class ExtractValueT = DefaultExtractValue<typename TreeT::Node>>
class OrderedBinaryTreeIterator {
 private:
  /// This type.
  using This =
      OrderedBinaryTreeIterator<TreeT, constant, reverse, ExtractValueT>;

 protected:
  /// `TreeT`.
  using Tree = TreeT;

  /// `Tree::Node`.
  using Node = typename Tree::Node;

  /// `Node::Data`.
  using Data = typename Node::Data;

  /// `ExtractValueT`.
  using ExtractValue = ExtractValueT;

  Tree* tree_{nullptr};
  Node* node_{nullptr};

  friend class
      OrderedBinaryTreeIterator<Tree, !constant, reverse, ExtractValue>;
  friend class
      OrderedBinaryTreeIterator<Tree, constant, !reverse, ExtractValue>;
  
  template<class TreeImplT>
  friend class ManagedTree;
  
  constexpr Node* begin_node() const {
    ASSERT(tree_);
    if constexpr (reverse) {
      return tree_->last;
    } else {
      return tree_->first;
    }
  }

  constexpr Node* before_end_node() const {
    ASSERT(tree_);
    if constexpr (reverse) {
      return tree_->first;
    } else {
      return tree_->last;
    }
  }

  static constexpr Node* next_node(Node* n) {
    if constexpr (reverse) {
      return n->find_prev_node();
    } else {
      return n->find_next_node();
    }
  }

  template<class Integer>
  static constexpr Node* next_node(Node* n, Integer steps) {
    if constexpr (reverse) {
      return n->find_prev_node(steps);
    } else {
      return n->find_next_node(steps);
    }
  }

  static constexpr Node* prev_node(Node* n) {
    if constexpr (reverse) {
      return n->find_next_node();
    } else {
      return n->find_prev_node();
    }
  }

  template<class Integer>
  static constexpr Node* prev_node(Node* n, Integer steps) {
    if constexpr (reverse) {
      return n->find_next_node(steps);
    } else {
      return n->find_prev_node(steps);
    }
  }

 public:

  using size_type = typename Tree::size_type;
  using difference_type = std::make_signed_t<size_type>;
  using value_type = std::conditional_t<constant, Data const, Data>;
  using pointer = std::add_pointer_t<value_type>;
  using reference = std::add_lvalue_reference_t<value_type>;
  using iterator_category = std::random_access_iterator_tag;

  constexpr OrderedBinaryTreeIterator(Tree* tree_ = nullptr, Node* node_ = nullptr)
    : tree_{tree_}, node_{node_} {}
  
  constexpr void reset(Tree* new_tree = nullptr, Node* new_node = nullptr) {
    tree_ = new_tree;
    node_ = new_node;
  }

  constexpr size_type get_index() const {
    ASSERT(tree_);
    if constexpr (reverse) {
      return node_ ? (tree_->size() - node_->get_index() - 1) : tree_->size();
    } else {
      return node_ ? node_->get_index() : tree_->size();
    }
  }

  constexpr OrderedBinaryTreeIterator(
      OrderedBinaryTreeIterator<Tree, false, reverse, ExtractValue> const&
        other)
    : tree_{other.tree_}, node_{other.node_} {}
  
  constexpr OrderedBinaryTreeIterator<Tree, constant, !reverse, ExtractValue>
      make_reverse_iterator() const {
    if (node_) {
      return {tree_, prev_node(node_)};
    }
    return {tree_, before_end_node()};
  }

  This& operator=(OrderedBinaryTreeIterator<Tree, false, reverse, ExtractValue>
      const& other)
  {
    tree_ = other.tree_;
    node_ = other.node_;
    return *this;
  }

  constexpr bool operator==(This const& other) const {
    ASSERT(tree_ == other.tree_);
    return node_ == other.node_;
  }

  constexpr bool operator!=(This const& other) const {
    ASSERT(tree_ == other.tree_);
    return node_ != other.node_;
  }

  constexpr std::conditional_t<constant, Data const&, Data&> operator*()
      const {
    ASSERT(node_);
    return ExtractValue::value_in_data(node_->data);
  }

  constexpr std::conditional_t<constant, Data const*, Data*> operator->()
      const {
    ASSERT(node_);
    return &(operator*());
  }

  constexpr This& operator++() {
    ASSERT(node_);
    node_ = next_node(node_);
    return *this;
  }

  constexpr This operator++(int) {
    This result{*this};
    operator++();
    return result;
  }

  template<class Integer,
      std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
  constexpr This& operator+=(Integer steps) {
    ASSERT(tree_);
    if (steps > 0) {
      ASSERT(node_);
      node_ = next_node(node_, static_cast<size_type>(steps));
    } else if (steps < 0) {
      if (!node_) {
        node_ = prev_node(before_end_node(), static_cast<size_type>(-steps - 1));
      } else {
        node_ = prev_node(node_, static_cast<size_type>(-steps));
      }
      ASSERT(node_);
    }
    return *this;
  }

  template<class Integer,
      std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
  constexpr This operator+(Integer steps) const {
    This result{*this};
    result += steps;
    return result;
  }

  constexpr This& operator--() {
    if (node_) {
      node_ = prev_node(node_);
    } else {
      ASSERT(tree_);
      node_ = before_end_node();
    }
    ASSERT(node_);
    return *this;
  }

  constexpr This operator--(int) {
    This result{*this};
    operator--();
    return result;
  }

  template<class Integer,
      std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
  constexpr This& operator-=(Integer steps) {
    if (steps > 0) {
      if (!node_) {
        ASSERT(tree_);
        node_ = prev_node(before_end_node(), static_cast<size_type>(steps - 1));
      } else {
        node_ = prev_node(node_, static_cast<size_type>(steps));
      }
      ASSERT(node_);
    } else if (steps < 0) {
      ASSERT(node_);
      node_ = next_node(node_, static_cast<size_type>(-steps));
    }
    return *this;
  }

  template<class Integer,
      std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
  constexpr This operator-(Integer steps) const {
    This result{*this};
    result -= steps;
    return result;
  }

  template<class Integer,
      std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
  constexpr std::conditional_t<constant, Data const&, Data&> operator[](
      Integer steps) const {
    return *(operator+(steps));
  }

  constexpr difference_type operator-(This const& other) const {
    ASSERT(tree_ == other.tree_);
    return static_cast<difference_type>(get_index()) -
        static_cast<difference_type>(other.get_index());
  }

  constexpr bool operator<(This const& other) const {
    ASSERT(tree_ == other.tree_);
    return get_index() < other.get_index();
  }

  constexpr bool operator>(This const& other) const {
    ASSERT(tree_ == other.tree_);
    return get_index() > other.get_index();
  }

  constexpr bool operator<=(This const& other) const {
    ASSERT(tree_ == other.tree_);
    return get_index() <= other.get_index();
  }

  constexpr bool operator>=(This const& other) const {
    ASSERT(tree_ == other.tree_);
    return get_index() >= other.get_index();
  }

};

} // namespace ordered_binary_trees

template<class TreeT, bool constant, bool reverse, class ExtractValueT,
    class Integer, std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
typename ordered_binary_trees::OrderedBinaryTreeIterator<
    TreeT, constant, reverse, ExtractValueT> operator+(
      Integer steps,
      typename ordered_binary_trees::OrderedBinaryTreeIterator<
        TreeT, constant, reverse, ExtractValueT> const& i) {
  return i + steps;
}
