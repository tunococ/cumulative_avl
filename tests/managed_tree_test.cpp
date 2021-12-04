#include <algorithm>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <ordered_binary_trees/basic_tree_impl.hpp>
#include <ordered_binary_trees/managed_tree.hpp>
#include <ordered_binary_trees/ordered_binary_tree.hpp>
#include <ordered_binary_trees/ordered_binary_tree_node.hpp>
#include <ordered_binary_trees/ordered_binary_tree_iterator.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

namespace obt = ordered_binary_trees;
using namespace std;

using Value = size_t;
using TreeImpls = tuple<
    obt::BasicTreeImpl<Value>>;

TEMPLATE_LIST_TEST_CASE("ManagedTree - insertion", "", TreeImpls) {
  using Tree = obt::ManagedTree<TestType>;

  Tree tree;
  deque<Value> list;

  static constexpr size_t kLength{128};
  static constexpr size_t kIncrement{257};

  SECTION("front and back positions") {
    for (size_t i{0}; i < kLength; ++i) {
      switch (i % 8) {
        case 0:
          list.push_back(i);
          tree.push_back(i);
          break;
        case 1:
          list.push_front(i);
          tree.push_front(i);
          break;
        case 2:
          list.emplace_back(i);
          tree.emplace_back(i);
          break;
        case 3:
          list.emplace_front(i);
          tree.emplace_front(i);
          break;
        case 4:
          CHECK(i == *list.insert(list.end(), i));
          CHECK(i == *tree.insert(tree.end(), i));
          break;
        case 5:
          CHECK(i == *list.insert(list.begin(), i));
          CHECK(i == *tree.insert(tree.begin(), i));
          break;
        case 6:
          CHECK(i == *list.emplace(list.end(), i));
          CHECK(i == *tree.emplace(tree.end(), i));
          break;
        case 7:
          CHECK(i == *list.emplace(list.begin(), i));
          CHECK(i == *tree.emplace(tree.begin(), i));
          break;
        default:
          CHECK(false);
          break;
      }
      CHECK(list.front() == tree.front());
      CHECK(list.back() == tree.back());
      CHECK(equal(list.begin(), list.end(), tree.begin(), tree.end()));
      CHECK(equal(list.cbegin(), list.cend(), tree.cbegin(), tree.cend()));
      CHECK(equal(list.rbegin(), list.rend(), tree.rbegin(), tree.rend()));
      CHECK(equal(list.crbegin(), list.crend(), tree.crbegin(), tree.crend()));
    }
  }

  SECTION("random positions") {
    size_t j{0};
    for (size_t i{0}; i < kLength; ++i) {
      j = (j + kIncrement) % (tree.size() + 1);
      auto list_it{i % 2 == 0 ? (list.begin() + j) : (list.end() - j)};
      auto tree_it{i % 2 == 0 ? (tree.begin() + j) : (tree.end() - j)};
      switch (i % 4) {
        case 0:
          [[fallthrough]];
        case 1:
          CHECK(i == *list.insert(list_it, i));
          CHECK(i == *tree.insert(tree_it, i));
          break;
        case 2:
          [[fallthrough]];
        case 3:
          CHECK(i == *list.emplace(list_it, i));
          CHECK(i == *tree.emplace(tree_it, i));
          break;
        default:
          CHECK(false);
          break;
      }
      CHECK(list.front() == tree.front());
      CHECK(list.back() == tree.back());
      CHECK(equal(list.begin(), list.end(), tree.begin(), tree.end()));
      CHECK(equal(list.cbegin(), list.cend(), tree.cbegin(), tree.cend()));
      CHECK(equal(list.rbegin(), list.rend(), tree.rbegin(), tree.rend()));
      CHECK(equal(list.crbegin(), list.crend(), tree.crbegin(), tree.crend()));

    }
  }
}

TEMPLATE_LIST_TEST_CASE("ManagedTree - element access", "", TreeImpls) {
  using Tree = obt::ManagedTree<TestType>;

  Tree tree;
  Tree const& const_tree{tree};
  deque<size_t> list;

  static constexpr size_t kLength{128};

  for (size_t i{0}; i < kLength; ++i) {
    tree.push_back(i);
    list.push_back(i);

    for (size_t j{0}; j <= tree.size(); ++j) {
      // `get_iterator_at_index()`
      CHECK(tree.get_iterator_at_index(j) == tree.begin() + j);
      CHECK(const_tree.get_iterator_at_index(j) == const_tree.begin() + j);

      if (j < tree.size()) {
        // `operator[]` and `at()`
        CHECK(tree[j] == tree.at(j));
        CHECK(tree[j] == *tree.get_iterator_at_index(j));
        CHECK(tree[j] == const_tree[j]);
        CHECK(const_tree[j] == const_tree.at(j));
        CHECK(const_tree[j] == *const_tree.get_iterator_at_index(j));
      }
    }

    // `get_front_iterator` and `get_back_iterator`.
    CHECK(tree.get_front_iterator()
        == tree.get_iterator_at_index(0));
    CHECK(tree.front() == *tree.get_front_iterator());
    CHECK(tree.get_back_iterator() ==
        tree.get_iterator_at_index(tree.size() - 1));
    CHECK(tree.back() == *tree.get_back_iterator());
    CHECK(const_tree.get_front_iterator() ==
        const_tree.get_iterator_at_index(0));
    CHECK(const_tree.front() == *const_tree.get_front_iterator());
    CHECK(const_tree.get_back_iterator() ==
        const_tree.get_iterator_at_index(const_tree.size() - 1));
    CHECK(const_tree.back() == *const_tree.get_back_iterator());
  }

}

TEMPLATE_LIST_TEST_CASE("ManagedTree - bulk insertion", "", TreeImpls) {
  using Tree = obt::ManagedTree<TestType>;

  Tree tree;
  deque<Value> list;
  static constexpr size_t kLength{64};

  for (size_t i{0}; i < kLength; ++i) {
    tree.push_back(i);
    list.push_back(i);
  }

  static constexpr size_t kLength_1{16};
  deque<Value> list_1;
  
  for (size_t i{0}; i < kLength_1; ++i) {
    list_1.push_back(i + kLength);
  }

  for (size_t i{0}; i <= kLength; ++i) {
    for (size_t j{0}; j <= kLength_1; ++j) {
      Tree tree_a{tree};
      deque<Value> list_a{list};

      tree_a.insert(tree_a.get_iterator_at_index(i),
          list_1.begin(), list_1.begin() + j);
      list_a.insert(list_a.begin() + i,
          list_1.begin(), list_1.begin() + j);

      CHECK(equal(tree_a.begin(), tree_a.end(), list_a.begin(), list_a.end()));
    }
  }

}

TEMPLATE_LIST_TEST_CASE("ManagedTree - erase", "", TreeImpls) {
  using Tree = obt::ManagedTree<TestType>;

  Tree tree;
  deque<Value> list;
  static constexpr size_t kLength{64};

  for (size_t i{0}; i < kLength; ++i) {
    tree.push_back(i);
    list.push_back(i);
  }

  SECTION("front") {
    while (!tree.empty()) {
      tree.pop_front();
      list.pop_front();
      CHECK(equal(tree.begin(), tree.end(), list.begin(), list.end()));
      CHECK(equal(tree.cbegin(), tree.cend(), list.cbegin(), list.cend()));
      CHECK(equal(tree.rbegin(), tree.rend(), list.rbegin(), list.rend()));
      CHECK(equal(tree.crbegin(), tree.crend(), list.crbegin(), list.crend()));
    }
    CHECK(list.empty());
  }

  SECTION("back") {
    while (!tree.empty()) {
      tree.pop_back();
      list.pop_back();
      CHECK(equal(tree.begin(), tree.end(), list.begin(), list.end()));
      CHECK(equal(tree.cbegin(), tree.cend(), list.cbegin(), list.cend()));
      CHECK(equal(tree.rbegin(), tree.rend(), list.rbegin(), list.rend()));
      CHECK(equal(tree.crbegin(), tree.crend(), list.crbegin(), list.crend()));
    }
    CHECK(list.empty());
  }

  SECTION("one at a time") {
    for (size_t inc : {251, 503, 751, 1009, 1511, 2003}) {
      size_t j{0};
      Tree tree_a{tree};
      deque<Value> list_a{list};
      while (!tree_a.empty()) {
        j = (j + inc) % tree_a.size();

        auto tree_it = tree_a.erase(tree_a.get_iterator_at_index(j));
        auto list_it = list_a.erase(list_a.begin() + j);

        if (tree_it != tree_a.end()) {
          CHECK(*tree_it == *list_it);
        } else {
          CHECK(list_it == list_a.end());
        }

        CHECK(equal(
            tree_a.begin(), tree_a.end(),
            list_a.begin(), list_a.end()));
      }
      CHECK(list_a.empty());
    }
  }

  SECTION("bulk") {
    for (size_t i{0}; i <= kLength; ++i) {
      for (size_t j{i}; j <= kLength; ++j) {
        Tree tree_a{tree};
        deque<Value> list_a{list};

        auto tree_it = tree_a.erase(
            tree_a.get_iterator_at_index(i),
            tree_a.get_iterator_at_index(j));
        auto list_it = list_a.erase(
            list_a.begin() + i,
            list_a.begin() + j);
            
        if (tree_it != tree_a.end()) {
          CHECK(*tree_it == *list_it);
        } else {
          CHECK(list_it == list_a.end());
        }

        CHECK(equal(
            tree_a.begin(), tree_a.end(),
            list_a.begin(), list_a.end()));

        cout << "Erasing all elements\n" << endl;
        tree_it = tree_a.erase(tree_a.cbegin(), tree_a.cend());
        cout << "Checking iterator\n" << endl;
        CHECK(tree_it == tree_a.end());
        CHECK(tree_a.empty());
      }
    }
  }

}
