#pragma once

#include <type_traits>

#include <ordered_binary_trees/assert.hpp>

namespace ordered_binary_trees {

template<class TreeT, bool constant>
class OrderedBinaryTreeIterator {
 private:
  /// This type.
  using This = OrderedBinaryTreeIterator<TreeT, constant>;

 protected:
  /// `TreeT`.
  using Tree = TreeT;

  /// `Tree::Node`.
  using Node = typename Tree::Node;

  /// `Node::Data`.
  using Data = typename Node::Data;

  Tree* tree{nullptr};
  Node* node{nullptr};

 public:

  using size_type = typename Tree::size_type;
  using difference_type = std::make_signed_t<size_type>;
  using value_type = std::conditional_t<constant, Data const, Data>;
  using pointer = std::add_pointer_t<value_type>;
  using reference = std::add_lvalue_reference_t<value_type>;
  using iterator_category = std::random_access_iterator_tag;

  constexpr OrderedBinaryTreeIterator(Tree* tree, Node* node = nullptr)
    : tree{tree}, node{node} {}
  
  constexpr void reset(Tree* new_tree = nullptr, Node* new_node = nullptr) {
    tree = new_tree;
    node = new_node;
  }

  constexpr Tree* get_tree() const {
    return tree;
  }

  constexpr Node* get_node() const {
    return node;
  }

  constexpr size_type get_index() const {
    ASSERT(tree);
    return node ? node->get_index() : tree->size();
  }

  template<bool other_const>
  constexpr OrderedBinaryTreeIterator(
      OrderedBinaryTreeIterator<Tree, other_const> const& other)
    : tree{other.get_tree()}, node{other.get_node()} {}
  
  template<bool other_const>
  This& operator=(OrderedBinaryTreeIterator<Tree, other_const> const& other) {
    tree = other.get_tree();
    node = other.get_node();
    return *this;
  }

  constexpr bool operator==(This const& other) const {
    ASSERT(tree == other.tree);
    return node == other.node;
  }

  constexpr bool operator!=(This const& other) const {
    ASSERT(tree == other.tree);
    return node != other.node;
  }

  constexpr std::conditional_t<constant, Data const&, Data&> operator*()
      const {
    ASSERT(node);
    return node->data;
  }

  constexpr std::conditional_t<constant, Data const*, Data*> operator->()
      const {
    ASSERT(node);
    return &(node->data);
  }

  constexpr This& operator++() {
    ASSERT(node);
    node = node->find_next_node();
    return *this;
  }

  constexpr This operator++(int) const {
    This result{*this};
    ++result;
    return result;
  }

  template<class Integer,
      std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
  constexpr This& operator+=(Integer steps) {
    ASSERT(tree);
    if (steps > 0) {
      ASSERT(node);
      node = node->find_next_node(static_cast<size_type>(steps));
    } else if (steps < 0) {
      if (!node) {
        node = tree->last->find_prev_node(static_cast<size_type>(-steps - 1));
      } else {
        node = node->find_prev_node(static_cast<size_type>(-steps));
      }
      ASSERT(node);
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
    if (node) {
      node = node->find_prev_node();
    } else {
      ASSERT(tree);
      node = tree->last;
    }
    ASSERT(node);
    return *this;
  }

  constexpr This operator--(int) const {
    This result{*this};
    --result;
    return result;
  }

  template<class Integer,
      std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
  constexpr This& operator-=(Integer steps) {
    if (steps > 0) {
      if (!node) {
        ASSERT(tree);
        node = tree->last->find_prev_node(static_cast<size_type>(steps - 1));
      } else {
        node = node->find_prev_node(static_cast<size_type>(steps));
      }
      ASSERT(node);
    } else if (steps < 0) {
      ASSERT(node);
      node = node->find_next_node(static_cast<size_type>(-steps));
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
    ASSERT(tree == other.tree);
    return static_cast<difference_type>(get_index()) -
        static_cast<difference_type>(other.get_index());
  }

  constexpr bool operator<(This const& other) const {
    ASSERT(tree == other.tree);
    return get_index() < other.get_index();
  }

  constexpr bool operator>(This const& other) const {
    ASSERT(tree == other.tree);
    return get_index() > other.get_index();
  }

  constexpr bool operator<=(This const& other) const {
    ASSERT(tree == other.tree);
    return get_index() <= other.get_index();
  }

  constexpr bool operator>=(This const& other) const {
    ASSERT(tree == other.tree);
    return get_index() >= other.get_index();
  }

};

} // namespace ordered_binary_trees

template<class TreeT, bool constant, class Integer,
    std::enable_if_t<std::is_integral_v<Integer>, int> = 0>
typename ordered_binary_trees::OrderedBinaryTreeIterator<TreeT, constant>
  operator+(
    Integer steps,
    typename ordered_binary_trees::OrderedBinaryTreeIterator<TreeT, constant>
      const& i) {
  return i + steps;
}
