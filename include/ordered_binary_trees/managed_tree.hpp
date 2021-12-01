#pragma once

#include <memory>
#include <stdexcept>

namespace ordered_binary_trees {

template<class TreeImplT>
class ManagedTree {
 private:
  using This = ManagedTree<TreeImplT>;

 protected:
  using TreeImpl = TreeImplT;
  using Tree = typename TreeImpl::Tree;
  using Node = typename TreeImpl::Node;
  using Data = typename TreeImpl::Data;
  using Value = typename TreeImpl::Value;
  using Allocator = typename TreeImpl::Allocator;
  using ValueAllocator = typename std::allocator_traits<Allocator>::
      template rebind_alloc<Value>;
  using ExtractValue = typename TreeImpl::ExtractValue;

  using NodePtr = typename Tree::NodePtr;
  using InsertPosition = typename Node::InsertPosition;

  mutable Tree tree_;

 public:
  using value_type = Value;
  using allocator_type = ValueAllocator;
  using size_type = typename std::allocator_traits<allocator_type>::size_type;
  using difference_Type = typename std::allocator_traits<allocator_type>::
      difference_type;
  using reference = value_type&;
  using const_reference = value_type const&;
  using pointer = typename std::allocator_traits<allocator_type>::pointer;
  using const_pointer = typename std::allocator_traits<allocator_type>::
      const_pointer;

  using iterator = OrderedBinaryTreeIterator<Tree,
      false, false, ExtractValue>;
  using const_iterator = OrderedBinaryTreeIterator<Tree,
      true, false, ExtractValue>;
  using reverse_iterator = OrderedBinaryTreeIterator<Tree,
      false, true, ExtractValue>;
  using const_reverse_iterator = OrderedBinaryTreeIterator<Tree,
      true, true, ExtractValue>;
  
  constexpr ManagedTree(This const& other) : tree_{other.tree_.clone()} {}
  constexpr ManagedTree(This&& other) : tree_{std::move(other.tree_)} {}

  constexpr ~ManagedTree() {
    tree_.destroy_all_nodes();
  }
  
  constexpr This& operator=(This const& other) {
    tree_ = other.tree_.clone();
    return *this;
  }

  constexpr This& operator=(This&& other) {
    tree_ = std::move(other.tree_);
    return *this;
  }

  constexpr void clear() {
    tree_.destroy_all_nodes();
  }

  constexpr size_type size() const {
    return tree_.size();
  }

  constexpr bool empty() const {
    return tree_.empty();
  }

  constexpr allocator_type get_allocator() const noexcept {
    return tree_.allocator;
  }

  constexpr reference operator[](size_type pos) {
    return ExtractValue::value_in_data(tree_.find_node_at_index(pos)->data);
  }

  constexpr const_reference operator[](size_type pos) const {
    return ExtractValue::value_in_data(tree_.find_node_at_index(pos)->data);
  }

  constexpr reference at(size_type pos) {
    if (pos < 0 || pos >= size()) {
      throw std::out_of_range();
    }
    return operator[](pos);
  }

  constexpr const_reference at(size_type pos) const {
    if (pos < 0 || pos >= size()) {
      throw std::out_of_range();
    }
    return operator[](pos);
  }

  constexpr reference front() {
    return ExtractValue::value_in_data(tree_.first->data);
  }

  constexpr const_reference front() const {
    return ExtractValue::value_in_data(tree_.first->data);
  }

  constexpr reference back() {
    return ExtractValue::value_in_data(tree_.last->data);
  }

  constexpr const_reference back() const {
    return ExtractValue::value_in_data(tree_.last->data);
  }

  constexpr iterator begin() {
    return iterator{&tree_, tree_.first};
  }

  constexpr const_iterator begin() const {
    return const_iterator(&tree_, tree_.first);;
  }

  constexpr const_iterator cbegin() const {
    return begin();
  }

  constexpr iterator end() {
    return iterator{&tree_, nullptr};
  }

  constexpr const_iterator end() const {
    return const_iterator{&tree_, nullptr};
  }

  constexpr const_iterator cend() const {
    return end();
  }

  constexpr reverse_iterator rbegin() {
    return reverse_iterator{&tree_, tree_.last};
  }

  constexpr const_reverse_iterator rbegin() const {
    return const_reverse_iterator(&tree_, tree_.last);;
  }

  constexpr const_reverse_iterator crbegin() const {
    return rbegin();
  }

  constexpr reverse_iterator rend() {
    return reverse_iterator{&tree_, nullptr};
  }

  constexpr const_reverse_iterator rend() const {
    return const_reverse_iterator{&tree_, nullptr};
  }

  constexpr const_reverse_iterator crend() const {
    return rend();
  }

  constexpr iterator insert(iterator pos, const Value& value) {
    // TODO: Implement by passing to TreeImpl.
    throw;
  }

};

} // namespace ordered_binary_trees
