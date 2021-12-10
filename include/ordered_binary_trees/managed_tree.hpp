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

  /**
   *  @brief
   *  Const-iterator type to facilitate initialization with repeated values.
   * 
   *  Conceptually, this can be viewed as an iterator into an infinite list
   *    whose elements are all `value`.
   */
  template<class IndexT>
  struct ValueRepeater {
    /**
     *  @brief
     *  `Value const` is the `value_type` of this iterator because this
     *    iterator does not allow the `value` to be modified.
     */
    using value_type = Value const;
    /// `value_type&`.
    using reference = value_type&;
    /// `IndexT`.
    using size_type = IndexT;
    /// Signed version of `size_type`.
    using difference_type = std::make_signed_t<size_type>;
    /// The value to repeat.
    value_type& value;
    /// Index (counter).
    size_type index;
    /// Constructs an iterator that produces `value` repeatedly.
    constexpr ValueRepeater(value_type const& value, size_type index)
      : index{index}, value{value} {}
    /// Returns `value`.
    constexpr value_type& operator*() {
      return value;
    }
    /// Returns `value`'s pointer.
    constexpr value_type* operator->() {
      return &value;
    }
    /// Pre-increments the iterator.
    constexpr ValueRepeater& operator++() {
      ++index;
      return *this;
    }
    /// Post-increments the iterator.
    constexpr ValueRepeater operator++(int) {
      ValueRepeater output{value, index++};
      return output;
    }
    /// Pre-decrements the iterator.
    constexpr ValueRepeater& operator--() {
      --index;
      return *this;
    }
    /// Post-decrements the iterator.
    constexpr ValueRepeater operator--(int) {
      ValueRepeater output{value, index--};
      return output;
    }
    /// Returns `true` if this iterator is equal to `other`.
    constexpr bool operator==(ValueRepeater const& other) const {
      return index == other.index;
    }
    /// Returns `true` if this iterator is not equal to `other`.
    constexpr bool operator!=(ValueRepeater const& other) const {
      return index != other.index;
    }
    /// Returns `true` if this iterator is greater than `other`.
    constexpr bool operator>(ValueRepeater const& other) const {
      return index > other.index;
    }
    /// Returns `true` if this iterator is greater than or equal to `other`.
    constexpr bool operator>=(ValueRepeater const& other) const {
      return index >= other.index;
    }
    /// Returns `true` if this iterator is less than `other`.
    constexpr bool operator<(ValueRepeater const& other) const {
      return index < other.index;
    }
    /// Returns `true` if this iterator is less than or equal to `other`.
    constexpr bool operator<=(ValueRepeater const& other) const {
      return index <= other.index;
    }
  };

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

  /**
   *  @brief
   *  Creates an empty tree with a given `allocator`.
   */
  constexpr ManagedTree(allocator_type const& allocator = allocator_type())
    : tree_{allocator} {}

  /**
   *  @brief
   *  Copies data from another tree using the given `allocator`.
   */
  constexpr ManagedTree(This const& other, allocator_type const& allocator)
    : tree_{allocator} {
    if (other.tree_.root) {
      tree_.template clone_from<false>(other.tree_.root);
    }
  }

  /**
   *  @brief
   *  Copies data from another tree. The allocator is copied via
   *    `select_on_container_copy_construction()`.
   */
  constexpr ManagedTree(This const& other)
    : tree_{std::allocator_traits<allocator_type>::
        select_on_container_copy_construction(other.tree_.allocator)} {
    if (other.tree_.root) {
      tree_.template clone_from<false>(other.tree_.root);
    }
  }

  /**
   *  @brief
   *  Copies data from another tree if `allocator != other.allocator`,
   *    or takes ownership of the data from another tree otherwise.
   */
  constexpr ManagedTree(This&& other, allocator_type const& allocator)
    : tree_{allocator} {
    if (other.tree_.root) {
      if (allocator != other.tree_.allocator) {
        tree_.template clone_from<false>(other.tree_.root);
      } else {
        tree_ = std::move(other.tree_);
      }
    }
  }

  /**
   *  @brief
   *  Moves data from another tree.
   */
  constexpr ManagedTree(This&& other)
    : tree_{std::move(other.tree_.allocator)} {
    tree_ = std::move(other.tree_);
  }

#if __cplusplus >= 202000L
  constexpr
#endif
  /**
   *  @brief
   *  Destroys the tree.
   */
  ~ManagedTree() {
    clear();
  }

  /**
   *  @brief
   *  Empties the tree.
   */
  constexpr void clear() {
    tree_.destroy_all_nodes();
  }

  /**
   *  @brief
   *  Clones the tree from `other`.
   */
  constexpr This& operator=(This const& other) {
    clear();
    if constexpr (std::allocator_traits<allocator_type>::
        propagate_on_container_copy_assignment::value) {
      tree_.allocator = other.tree_.allocator;
    }
    if (other.tree_.root) {
      tree_.clone_from(other.tree_.root);
    }
    return *this;
  }

  /**
   *  @brief
   *  Takes the tree from `other`.
   */
  constexpr This& operator=(This&& other) {
    clear();
    if constexpr (std::allocator_traits<allocator_type>::
        propagate_on_container_move_assignment::value) {
      tree_.allocator = std::move(other.tree_.allocator);
    }
    tree_ = std::move(other.tree_);
    return *this;
  }

  /**
   *  @brief
   *  Swaps this tree with `other`.
   */
  constexpr void swap(This& other) {
    if constexpr (std::allocator_traits<allocator_type>::
        propagate_on_container_swap::value) {
      using std::swap;
      swap(tree_.allocator, other.tree_.allocator);
    }
    tree_.swap(other.tree_);
  }

  /**
   *  @brief
   *  Returns the number of elements in the tree.
   */
  constexpr size_type size() const {
    return tree_.size();
  }

  /**
   *  @brief
   *  Returns `true` iff the tree is empty.
   */
  constexpr bool empty() const {
    return tree_.empty();
  }

  /**
   *  @brief
   *  Returns the allocator.
   */
  constexpr allocator_type get_allocator() const noexcept {
    return tree_.allocator;
  }

  /**
   *  @brief
   *  Clears the tree and assigns values from `[first, last)` to the tree.
   */
  template<class InputIterator>
  constexpr void assign(InputIterator first, InputIterator last) {
    TreeImpl::assign(tree_, first, last);
  }

  /**
   *  @brief
   *  Clears the tree and assigns values from `ilist` to the tree.
   */
  template<class V>
  constexpr void assign(std::initializer_list<V> ilist) {
    assign(ilist.begin(), ilist.end());
  }

  /**
   *  @brief
   *  Clears the tree and assigns `n` copies of `value` to the tree.
   */
  constexpr void assign(size_type n, value_type const& value) {
    assign(
        ValueRepeater<size_type>{value, 0},
        ValueRepeater<size_type>{value, n});
  }

  /**
   *  @brief
   *  Accesses the `index`-th element.
   */
  constexpr reference operator[](size_type index) {
    return ExtractValue::value_in_data(
        TreeImpl::find_node_at_index(tree_, index)->data);
  }

  /**
   *  @brief
   *  Accesses the `index`-th element.
   */
  constexpr const_reference operator[](size_type index) const {
    return ExtractValue::value_in_data(
        TreeImpl::find_node_at_index(tree_, index)->data);
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
    return make_iterator(TreeImpl::find_node_at_index(tree_, index));
  }

  /**
   *  @brief
   *  Returns a const-iterator for the `index`-th element.
   *
   *  This is equivalent to `cbegin() + index`, but may be more efficient.
   */
  constexpr const_iterator get_iterator_at_index(size_type index) const {
    return make_iterator<true>(TreeImpl::find_node_at_index(tree_, index));
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
   *  Converts a const-iterator to a regular iterator.
   *
   *  This function works on reverse iterators also.
   */
  template<bool constant = false, bool reverse = false>
  p_iterator<false, reverse> make_mutable_iterator(
      p_iterator<constant, reverse> it) const {
    return {it.tree_, it.node_};
  }

  /**
   *  @brief
   *  Inserts `value` right before `pos` and returns the iterator to the newly
   *    inserted value.
   */
  template<bool constant>
  constexpr iterator insert(p_iterator<constant> pos, Value const& value) {
    assert(pos.tree_ == &tree_);
    return make_iterator(
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
    return make_iterator(
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
    return make_iterator(
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
    return make_iterator(
        TreeImpl::insert_nodes_before(tree_, pos.node_, first, last));
  }

  /**
   *  @brief
   *  Takes data from `other` and inserts them right before `pos`;
   *    then returns an iterator to the node that the first element of `other`
   *    ends up at, or simply returns the non-const version of `pos` if `other`
   *    is empty.
   *
   *  `join()` will move nodes from the `other` tree into this tree at the
   *    position immediately before `pos`.
   *  This is similar to `insert(pos, other.begin(), other.end())` followed by
   *    `other.clear()`, but may be more efficient.
   *
   *  The allocators in `this` and `other` must be compatible in the sense that
   *    all allocations done by `other` must be deallocatable by `this`.
   *  Otherwise, the behavior will be undefined.
   */
  template<bool constant>
  constexpr iterator join(p_iterator<constant> pos, This& other) {
    assert(pos.tree_ == &tree_);
    assert(tree_.allocator == other.tree_.allocator);
    if (other.empty()) {
      return make_iterator(pos.node_);
    }
    NodePtr n{other.tree_.first};
    TreeImpl::join(
        tree_,
        pos.node_ ?
          pos.node_->get_prev_insert_position() :
          tree_.get_last_insert_position(),
        other.tree_);
    return make_iterator(n);
  }

  /**
   *  @brief
   *  Similar to `join(begin(), other)`.
   */
  constexpr iterator join_front(This& other) {
    assert(tree_.allocator == other.tree_.allocator);
    if (other.empty()) {
      return make_iterator(tree_.first);
    }
    NodePtr n{other.tree_.first};
    TreeImpl::join_front(tree_, other.tree_);
    return make_iterator(n);
  }

  /**
   *  @brief
   *  Similar to `join(end(), other)`.
   */
  constexpr iterator join_back(This& other) {
    assert(tree_.allocator == other.tree_.allocator);
    if (other.empty()) {
      return make_iterator(nullptr);
    }
    NodePtr n{other.tree_.first};
    TreeImpl::join_back(tree_, other.tree_);
    return make_iterator(n);
  }

  /**
   *  @brief
   *  Inserts `value` as the first element.
   */
  constexpr void push_front(Value const& value) {
    TreeImpl::emplace_front(tree_, value);
  }
  
  /**
   *  @brief
   *  Inserts `value` as the first element.
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
   *  Inserts `value` as the last element.
   */
  constexpr void push_back(Value const& value) {
    TreeImpl::emplace_back(tree_, value);
  }

  /**
   *  @brief
   *  Inserts `value` as the last element.
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
