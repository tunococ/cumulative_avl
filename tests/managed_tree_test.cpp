#include <algorithm>
#include <cstdint>
#include <deque>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <random>
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
#include <ordered_binary_trees/splay_tree_impl.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

namespace obt = ordered_binary_trees;
using namespace std;

struct IndexRand {
  mt19937_64 generator;
  IndexRand(uint_fast64_t seed = 123456) : generator{seed} {}
  size_t operator()(size_t modulus) {
    return static_cast<size_t>(
        generator() % static_cast<uint_fast64_t>(modulus));
  }
};

using Value = size_t;
using TreeImpls = tuple<obt::BasicTreeImpl<Value>, obt::SplayTreeImpl<Value>>;

TEMPLATE_LIST_TEST_CASE("ManagedTree - insertion",
    "", TreeImpls) {
  
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
          REQUIRE(false);
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
          REQUIRE(false);
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

TEMPLATE_LIST_TEST_CASE("ManagedTree - element access",
    "", TreeImpls) {
  
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

TEMPLATE_LIST_TEST_CASE("ManagedTree - bulk insertion",
    "", TreeImpls) {
  
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

TEMPLATE_LIST_TEST_CASE("ManagedTree - erase",
    "", TreeImpls) {
  
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

TEMPLATE_LIST_TEST_CASE("ManagedTree - random single-tree operations",
    "", TreeImpls) {
  
  using Tree = obt::ManagedTree<TestType>;

  Tree tree;
  deque<Value> list;

  auto read_only_check = [](auto& tree, auto& list) {
    CHECK(equal(tree.begin(), tree.end(), list.begin(), list.end()));
    CHECK(equal(tree.rbegin(), tree.rend(), list.rbegin(), list.rend()));
    if (!tree.empty()) {
      CHECK(tree.front() == list.front());
      CHECK(tree.back() == list.back());

      CHECK(tree.front() == *tree.get_front_iterator());
      CHECK(tree.back() == *tree.get_back_iterator());

      CHECK(tree.end() - 1 == tree.get_back_iterator());

      CHECK(tree.back() == tree.rbegin()[0]);
      CHECK(tree.front() == tree.rend()[-1]);
    } else {
      CHECK(tree.begin() == tree.end());
    }
    CHECK(tree.begin() == tree.get_front_iterator());
    for (size_t index{0}; index < tree.size(); ++index) {
      CHECK(tree.at(index) == list.at(index));
      CHECK(tree[index] == list[index]);
      CHECK(tree.get_iterator_at_index(index) == tree.begin() + index);
      CHECK(*tree.get_iterator_at_index(index) == tree[index]);
    }
    CHECK(tree.get_iterator_at_index(tree.size()) == tree.end());

    {
      // Test copy constructor.
      Tree tree_a{tree};
      CHECK(equal(tree_a.begin(), tree_a.end(), list.begin(), list.end()));

      // Test move constructor.
      Tree tree_b{std::move(tree_a)};
      CHECK(equal(tree_b.begin(), tree_b.end(), list.begin(), list.end()));
      CHECK(tree_a.empty());

      // Test copy assignment.
      tree_a = tree;
      CHECK(equal(tree_a.begin(), tree_a.end(), list.begin(), list.end()));
      
      // Test move assignment.
      tree_b = std::move(tree_a);
      CHECK(equal(tree_b.begin(), tree_b.end(), list.begin(), list.end()));
      CHECK(tree_a.empty());

      // Test clear.
      CHECK(!tree_b.empty());
      tree_b.clear();
      CHECK(tree_b.empty());
    }
  };

  static constexpr size_t kNumOperations{1000};
  static constexpr size_t kMaxBulkSize{8};

  IndexRand rand{};

  for (size_t counter{0}; counter < kNumOperations; ++counter) {
    size_t op_1{rand(tree.empty() ? 4 : 6)};

    switch (op_1) {
      case 0: { // Add an element to one of the two ends.
        size_t op_2{rand(8)};
        switch (op_2) {
          case 0: // push_front
            cout << "push_front(" << counter << ")\n";
            list.push_front(counter);
            tree.push_front(counter);
            CHECK(tree.front() == counter);
            break;
          case 1: // emplace_front
            cout << "emplace_front(" << counter << ")\n";
            list.emplace_front(counter);
            tree.emplace_front(counter);
            CHECK(tree.front() == counter);
            break;
          case 2: // insert to front
            cout << "insert(begin(), " << counter << ")\n";
            list.insert(list.begin(), counter);
            CHECK(*tree.insert(tree.begin(), counter) == counter);
            CHECK(tree.front() == counter);
            break;
          case 3: // emplace to front
            cout << "emplace(begin(), " << counter << ")\n";
            list.emplace(list.begin(), counter);
            CHECK(*tree.emplace(tree.begin(), counter) == counter);
            CHECK(tree.front() == counter);
            break;
          case 4: // push_back
            cout << "push_back(" << counter << ")\n";
            list.push_back(counter);
            tree.push_back(counter);
            CHECK(tree.back() == counter);
            break;
          case 5: // emplace_back
            cout << "emplace_back(" << counter << ")\n";
            list.emplace_back(counter);
            tree.emplace_back(counter);
            CHECK(tree.back() == counter);
            break;
          case 6: // insert to back
            cout << "insert(end(), " << counter << ")\n";
            list.insert(list.end(), counter);
            CHECK(*tree.insert(tree.end(), counter) == counter);
            CHECK(tree.back() == counter);
            break;
          case 7: // emplace to back
            cout << "emplace(end(), " << counter << ")\n";
            list.emplace(list.end(), counter);
            CHECK(*tree.emplace(tree.end(), counter) == counter);
            CHECK(tree.back() == counter);
            break;
          default:
            REQUIRE(false);
            break;
        }
        break;
      }
      case 1: { // Insert an element at a random index.
        size_t op_2{rand(2)};
        size_t index{rand(tree.size() + 1)};
        switch (op_2) {
          case 0: { // insert at index
            cout << "insert(get_iterator_at_index("
                << index << "), "
                << counter << ")\n";
            list.insert(list.begin() + index, counter);
            auto it{tree.insert(tree.get_iterator_at_index(index), counter)};
            CHECK(it == tree.get_iterator_at_index(index));
            break;
          }
          case 1: { // emplace at index
            cout << "emplace(get_iterator_at_index("
                << index << "), "
                << counter << ")\n";
            list.emplace(list.begin() + index, counter);
            auto it{tree.emplace(tree.get_iterator_at_index(index), counter)};
            CHECK(it == tree.get_iterator_at_index(index));
            break;
          }
          default:
            REQUIRE(false);
            break;
        }
        break;
      }
      case 2: { // Insert multiple elements at a random index.
        size_t index{rand(tree.size() + 1)};
        size_t size{rand(kMaxBulkSize + 1)};
        vector<Value> list_to_insert;
        for (size_t j{0}; j < size; ++j) {
          list_to_insert.push_back(j + counter * kNumOperations);
        }

        if (size == 0) {
          cout << "insert empty range at index " << index << "\n";
        } else {
          cout << "insert ["
              << list_to_insert.front() << ".." << list_to_insert.back()
              << "] at index " << index << "\n";
        }
        list.insert(list.begin() + index,
            list_to_insert.begin(), list_to_insert.end());
        auto it{
            tree.insert(
              tree.get_iterator_at_index(index),
              list_to_insert.begin(),
              list_to_insert.end())};
        CHECK(it == tree.get_iterator_at_index(index));
        break;
      }
      case 3: { // Erase a random interval of elements.
        size_t begin{rand(tree.size())};
        size_t length{rand(kMaxBulkSize)};
        size_t end{min(begin + length, tree.size())};

        cout << "erase an interval: [" << begin << ", " << end << ")\n";
        auto end_it{tree.get_iterator_at_index(end)};
        list.erase(list.begin() + begin, list.begin() + end);
        CHECK(tree.erase(tree.get_iterator_at_index(begin), end_it) == end_it);
        break;
      }
      case 4: { // Erase one element from one of the two ends.
        REQUIRE(!tree.empty());
        size_t op_2{rand(6)};
        auto it_1{tree.get_iterator_at_index(1)};
        auto it_end{tree.end()};
        switch (op_2) {
          case 0: // pop_front
            cout << "pop_front()\n";
            list.pop_front();
            tree.pop_front();
            break;
          case 1: // erase at get_front_iterator()
            cout << "erase(get_front_iterator())\n";
            list.erase(list.begin());
            CHECK(tree.erase(tree.get_front_iterator()) == it_1);
            break;
          case 2: // erase at begin()
            cout << "erase(begin())\n";
            list.erase(list.cbegin());
            CHECK(tree.erase(tree.cbegin()) == it_1);
            break;
          case 3: // pop_back
            cout << "pop_back()\n";
            list.pop_back();
            tree.pop_back();
            break;
          case 4: // erase at get_back_iterator()
            cout << "erase(get_back_iterator())\n";
            list.erase(std::prev(list.end()));
            CHECK(tree.erase(tree.get_back_iterator()) == it_end);
            break;
          case 5: // erase at end() - 1
            cout << "erase(end() - 1)\n";
            list.erase(std::prev(list.cend()));
            CHECK(tree.erase(std::prev(tree.cend())) == it_end);
            break;
          default:
            REQUIRE(false);
            break;
        }
        break;
      }
      case 5: { // Erase one element at a random index.
        REQUIRE(!tree.empty());
        size_t index{rand(tree.size())};

        cout << "erase(get_iterator_at_index(" << index << "))\n";
        auto it_next{tree.get_iterator_at_index(index + 1)};
        list.erase(list.begin() + index);
        CHECK(tree.erase(tree.get_iterator_at_index(index)) == it_next);
        break;
      }
    }

    read_only_check(tree, list);
    read_only_check(const_cast<Tree const&>(tree), list);
  }
}

TEMPLATE_LIST_TEST_CASE("ManagedTree - join",
    "", TreeImpls) {
  
  using Tree = obt::ManagedTree<TestType>;

  static constexpr size_t kLength{64};

  Tree tree_1;
  Tree tree_2;
  deque<Value> list_1;
  deque<Value> list_2;

  for (size_t i{0}; i < kLength; ++i) {
    tree_1.push_back(i);
    list_1.push_back(i);
    tree_2.push_back(i + kLength);
    list_2.push_back(i + kLength);
  }

  SECTION("front") {
    tree_1.join_front(tree_2);
    CHECK(tree_2.empty());

    list_1.insert(list_1.begin(), list_2.begin(), list_2.end());
    CHECK(equal(
        tree_1.begin(), tree_1.end(),
        list_1.begin(), list_1.end()));
  }

  SECTION("back") {
    tree_1.join_back(tree_2);
    CHECK(tree_2.empty());

    list_1.insert(list_1.end(), list_2.begin(), list_2.end());
    CHECK(equal(
        tree_1.begin(), tree_1.end(),
        list_1.begin(), list_1.end()));
  }

  SECTION("middle") {
    for (size_t i{0}; i <= tree_1.size(); ++i) {
      Tree tree_a{tree_1};
      Tree tree_b{tree_2};
      
      tree_a.join(tree_a.get_iterator_at_index(i), tree_b);
      CHECK(tree_b.empty());

      deque<Value> list_a{list_1};
      list_a.insert(list_a.begin() + i, list_2.begin(), list_2.end());
      CHECK(equal(
          tree_a.begin(), tree_a.end(),
          list_a.begin(), list_a.end()));
    }
  }
}
