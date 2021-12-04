#pragma once

#include <cassert>
#include <memory>
#include <stdexcept>
#include <type_traits>

namespace ordered_binary_trees {

/**
 *  @brief
 *  Template class for binary tree-based list data structures.
 *
 *  This class provides a `std::deque`-like interface for tree-based data
 *    structures.
 */
template<class TreeImplT>
class ManagedTree {
 private:
  /// This class.
  using This = ManagedTree<TreeImplT>;

 protected:
  /**
   *  @brief
   *  Class that contains implementations of the tree data structure.
   */
  using TreeImpl = TreeImplT;

  /// Type of the actual representation of the tree.
  using Tree = typename TreeImpl::Tree;

  /// Type of nodes in a tree.
  using Node = typename TreeImpl::Node;

  /// Type of the `data` field inside `Node`.
  using Data = typename TreeImpl::Data;

  /// Type of *values*.
  using Value = typename TreeImpl::Value;

  /// Type of the node allocator.
  using Allocator = typename TreeImpl::Allocator;

  /// Type of the value allocator.
  using ValueAllocator = typename TreeImpl::ValueAllocator;

  /// Type of `ExtractValue`. See `basic_tree_impl.hpp` for more information.
  using ExtractValue = typename TreeImpl::ExtractValue;

  /// Type of pointers to nodes.
  using NodePtr = typename Tree::NodePtr;

  /// Type of `InsertPosition` for `Node`.
  using InsertPosition = typename Node::InsertPosition;

  /// Actual representation of the tree.
  mutable Tree tree_;

  /// Parametrized iterator type.
  template<bool constant = false, bool reverse = false>
  using p_iterator = OrderedBinaryTreeIterator<
      Tree,
      constant,
      reverse,
      ExtractValue>;

  /// Creates an iterator from `NodePtr`.
  template<bool constant = false, bool reverse = false>
  p_iterator<constant, reverse> make_iterator(NodePtr node) const {
    return {&tree_, node};
  }

 public:
  /// Type of *values*.
  using value_type = Value;

  /// Type of the allocator for values.
  using allocator_type = ValueAllocator;

  /// `size_type` derived from `allocator_type`.
  using size_type = typename std::allocator_traits<allocator_type>::size_type;

  /// `difference_type` derived from `allocator_type`.
  using difference_Type = typename std::allocator_traits<allocator_type>::
      difference_type;
  
  /// `value_type&`.
  using reference = value_type&;

  /// `value_type const&`.
  using const_reference = value_type const&;

  /// `pointer` type dervied from `allocator_type`.
  using pointer = typename std::allocator_traits<allocator_type>::pointer;

  /// `const_pointer` type dervied from `allocator_type`.
  using const_pointer = typename std::allocator_traits<allocator_type>::
      const_pointer;

  /// Type of iterators.
  using iterator = p_iterator<false, false>;
  /// Type of const-iterators.
  using const_iterator = p_iterator<true, false>;
  /// Type of reverse-iterators.
  using reverse_iterator = p_iterator<false, true>;
  /// Type of const-reverse-iterators.
  using const_reverse_iterator = p_iterator<true, true>;

  /// Creates a tree with a given `allocator`.
  constexpr ManagedTree(allocator_type allocator = allocator_type())
    : tree_{allocator} {}

  /// Clones the tree from `other`.
  constexpr ManagedTree(This const& other) : tree_{other.tree_.clone()} {}

  /// Takes the tree from `other`.
  constexpr ManagedTree(This&& other) : tree_{std::move(other.tree_)} {}

#if __cplusplus >= 202000L
  constexpr
#endif
  /// Destroys the tree.
  ~ManagedTree() {
    tree_.destroy_all_nodes();
  }

  /// Clones the tree from `other`.
  constexpr This& operator=(This const& other) {
    tree_ = other.tree_.clone();
    return *this;
  }

  /// Takes the tree from `other`.
  constexpr This& operator=(This&& other) {
    tree_ = std::move(other.tree_);
    return *this;
  }

  /// Empties the tree.
  constexpr void clear() {
    tree_.destroy_all_nodes();
  }

  /// Returns the number of elements in the tree.
  constexpr size_type size() const {
    return tree_.size();
  }

  /// Returns `true` iff the tree is empty.
  constexpr bool empty() const {
    return tree_.empty();
  }

  /// Returns the allocator.
  constexpr allocator_type get_allocator() const noexcept {
    return tree_.allocator;
  }

  /**
   *  @brief
   *  Accesses the `index`-th element.
   */
  constexpr reference operator[](size_type index) {
    return ExtractValue::value_in_data(tree_.find_node_at_index(index)->data);
  }

  /**
   *  @brief
   *  Accesses the `index`-th element.
   */
  constexpr const_reference operator[](size_type index) const {
    return ExtractValue::value_in_data(tree_.find_node_at_index(index)->data);
  }

  /**
   *  @brief
   *  Accesses the `index`-th element.
   */
  constexpr reference at(size_type pos) {
    if (pos < 0 || pos >= size()) {
      throw std::out_of_range("ManagedTree::at -- index out of range");
    }
    return operator[](pos);
  }

  /**
   *  @brief
   *  Accesses the `index`-th element.
   */
  constexpr const_reference at(size_type pos) const {
    if (pos < 0 || pos >= size()) {
      throw std::out_of_range("ManagedTree::at -- index out of range");
    }
    return operator[](pos);
  }

  /**
   *  @brief
   *  Accesses the first element.
   */
  constexpr reference front() {
    return ExtractValue::value_in_data(tree_.first->data);
  }

  /**
   *  @brief
   *  Accesses the first element.
   */
  constexpr const_reference front() const {
    return ExtractValue::value_in_data(tree_.first->data);
  }

  /**
   *  @brief
   *  Accesses the last element.
   */
  constexpr reference back() {
    return ExtractValue::value_in_data(tree_.last->data);
  }

  /**
   *  @brief
   *  Accesses the last element.
   */
  constexpr const_reference back() const {
    return ExtractValue::value_in_data(tree_.last->data);
  }

  /**
   *  @brief
   *  Returns the iterator to the first element.
   */
  constexpr iterator begin() {
    return make_iterator(tree_.first);
  }

  /**
   *  @brief
   *  Returns the const-iterator to the first element.
   */
  constexpr const_iterator begin() const {
    return make_iterator<true>(tree_.first);;
  }

  /**
   *  @brief
   *  Returns the const-iterator to the first element.
   */
  constexpr const_iterator cbegin() const {
    return begin();
  }

  /**
   *  @brief
   *  Returns the reverse-iterator to the last element.
   */
  constexpr reverse_iterator rbegin() {
    return make_iterator<false, true>(tree_.last);
  }

  /**
   *  @brief
   *  Returns the const-reverse-iterator to the last element.
   */
  constexpr const_reverse_iterator rbegin() const {
    return make_iterator<true, true>(tree_.last);
  }

  /**
   *  @brief
   *  Returns the const-reverse-iterator to the last element.
   */
  constexpr const_reverse_iterator crbegin() const {
    return rbegin();
  }

  /**
   *  @brief
   *  Returns the past-the-end iterator.
   */
  constexpr iterator end() {
    return make_iterator(nullptr);
  }

  /**
   *  @brief
   *  Returns the past-the-end const-iterator.
   */
  constexpr const_iterator end() const {
    return make_iterator<true>(nullptr);
  }

  /**
   *  @brief
   *  Returns the past-the-end const-iterator.
   */
  constexpr const_iterator cend() const {
    return end();
  }

  /**
   *  @brief
   *  Returns the past-the-beginning reverse-iterator.
   */
  constexpr reverse_iterator rend() {
    return make_iterator<false, true>(nullptr);
  }

  /**
   *  @brief
   *  Returns the past-the-beginning const-reverse-iterator.
   */
  constexpr const_reverse_iterator rend() const {
    return make_iterator<true, true>(nullptr);
  }

  /**
   *  @brief
   *  Returns the past-the-beginning const-reverse-iterator.
   */
  constexpr const_reverse_iterator crend() const {
    return rend();
  }

  /**
   *  @brief
   *  Returns an iterator for the `index`-th element.
   *
   *  This is equivalent to `begin() + index`, but may be more efficient.
   */
  constexpr iterator get_iterator_at_index(size_type index) {
    return make_iterator(tree_.find_node_at_index(index));
  }

  /**
   *  @brief
   *  Returns a const-iterator for the `index`-th element.
   *
   *  This is equivalent to `cbegin() + index`, but may be more efficient.
   */
  constexpr const_iterator get_iterator_at_index(size_type index) const {
    return make_iterator<true>(tree_.find_node_at_index(index));
  }

  /**
   *  @brief
   *  Returns an iterator for the first element.
   *
   *  This is equivalent to `get_iterator_at_index(0)`, but may be more
   *    efficient.
   */
  constexpr iterator get_front_iterator() {
    return make_iterator(tree_.first);
  }

  /**
   *  @brief
   *  Returns a const-iterator for the first element.
   *
   *  This is equivalent to `get_iterator_at_index(0)`, but may be more
   *    efficient.
   */
  constexpr const_iterator get_front_iterator() const {
    return make_iterator<true>(tree_.first);
  }

  /**
   *  @brief
   *  Returns an iterator for the last element.
   *
   *  This is equivalent to `get_iterator_at_index(size() - 1)`, but may be
   *    more efficient.
   */
  constexpr iterator get_back_iterator() {
    assert(!empty());
    return make_iterator(tree_.last);
  }

  /**
   *  @brief
   *  Returns a const-iterator for the last element.
   *
   *  This is equivalent to `get_iterator_at_index(size() - 1)`, but may be
   *    more efficient.
   */
  constexpr const_iterator get_back_iterator() const {
    assert(!empty());
    return make_iterator<true>(tree_.last);
  }

  /**
   *  @brief
   *  Inserts `value` right before `pos` and returns the iterator to the newly
   *    inserted value.
   */
  template<bool constant>
  constexpr iterator insert(p_iterator<constant> pos, Value const& value) {
    assert(pos.tree_ == &tree_);
    return make_iterator<constant>(
        TreeImpl::emplace_node_before(tree_, pos.node_, value));
  }

  /**
   *  @brief
   *  Inserts `value` right before `pos` and returns the iterator to the newly
   *    inserted value.
   */
  template<bool constant>
  constexpr iterator insert(p_iterator<constant> pos, Value&& value) {
    assert(pos.tree_ == &tree_);
    return make_iterator<constant>(
        TreeImpl::emplace_node_before(tree_, pos.node_, std::move(value)));
  }

  /**
   *  @brief
   *  Constructs a new `value` and inserts it right before `pos`, then returns
   *    the iterator to the newly inserted value.
   */
  template<bool constant, class... Args>
  constexpr iterator emplace(p_iterator<constant> pos, Args&&... args) {
    assert(pos.tree_ == &tree_);
    return make_iterator<constant>(
        TreeImpl::emplace_node_before(
          tree_, pos.node_, std::forward<Args>(args)...));
  }

  /**
   *  @brief
   *  Inserts a list of values from `[first, last)` right before `pos`, then
   *    returns the iterator to the first value that was inserted.
   *
   *  If `first == last`, this function simply returns the non-const version of
   *    `last`.
   */
  template<bool constant, class InputIterator>
  constexpr iterator insert(
      p_iterator<constant> pos,
      InputIterator first,
      InputIterator last) {
    assert(pos.tree_ == &tree_);
    return make_iterator<constant>(
        TreeImpl::insert_nodes_before(tree_, pos.node_, first, last));
  }

  /**
   *  @brief
   *  Inserts a value as the first element.
   */
  constexpr void push_front(Value const& value) {
    TreeImpl::emplace_front(tree_, value);
  }
  
  /**
   *  @brief
   *  Inserts a value as the first element.
   */
  constexpr void push_front(Value&& value) {
    TreeImpl::emplace_front(tree_, std::move(value));
  }

  /**
   *  @brief
   *  Constructs a value and inserts it as the first element.
   */
  template<class... Args>
  constexpr void emplace_front(Args&&... args) {
    TreeImpl::emplace_front(tree_, std::forward<Args>(args)...);
  }
  
  /**
   *  @brief
   *  Inserts a value as the last element.
   */
  constexpr void push_back(Value const& value) {
    TreeImpl::emplace_back(tree_, value);
  }

  /**
   *  @brief
   *  Inserts a value as the last element.
   */
  constexpr void push_back(Value&& value) {
    TreeImpl::emplace_back(tree_, std::move(value));
  }

  /**
   *  @brief
   *  Constructs a value and inserts it as the last element.
   */
  template<class... Args>
  constexpr void emplace_back(Args&&... args) {
    TreeImpl::emplace_back(tree_, std::forward<Args>(args)...);
  }

  /**
   *  @brief
   *  Erases an element pointed to by `pos`, then returns the iterator to the
   *    position right after `pos`.
   */
  template<bool constant>
  constexpr iterator erase(p_iterator<constant> pos) {
    assert(pos.tree_ == &tree_);
    assert(pos.node_);
    return make_iterator(TreeImpl::erase_node(tree_, pos.node_));
  }
  
  /**
   *  @brief
   *  Erases elements in the interval `[first, last)`, then returns the
   *    non-const version of `last`.
   *
   *  If `first == last`, this function simply returns the non-const version of
   *    last without erasing any elements.
   */
  template<bool constant_1, bool constant_2>
  constexpr iterator erase(
      p_iterator<constant_1> first,
      p_iterator<constant_2> last) {
    assert(first.tree_ == &tree_);
    assert(last.tree_ == &tree_);
    assert(first <= last);
    return make_iterator(TreeImpl::erase_nodes(
        tree_, first.node_, last.node_));
  }

  /**
   *  @brief
   *  Erases the first element.
   */
  constexpr void pop_front() {
    assert(!tree_.empty());
    TreeImpl::erase_front(tree_);
  }

  /**
   *  @brief
   *  Erases the last element.
   */
  constexpr void pop_back() {
    assert(!tree_.empty());
    TreeImpl::erase_back(tree_);
  }

};

} // namespace ordered_binary_trees
