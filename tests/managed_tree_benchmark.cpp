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

#include <catch2/benchmark/catch_benchmark.hpp>
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

TEMPLATE_LIST_TEST_CASE("ManagedTree benchmark - insertion",
    "", TreeImpls) {

  using Tree = obt::ManagedTree<TestType>;

  static constexpr size_t kLength{1 << 10};

  BENCHMARK("insert to the front") {
    Tree tree;
    for (size_t i{0}; i < kLength; ++i) {
      tree.push_front(i);
    }
    return tree.size();
  };
  
  BENCHMARK("insert to the back") {
    Tree tree;
    for (size_t i{0}; i < kLength; ++i) {
      tree.push_back(i);
    }
    return tree.size();
  };
  
  BENCHMARK("insert at random index") {
    IndexRand rand{};
    Tree tree;
    for (size_t i{0}; i < kLength; ++i) {
      tree.insert(tree.get_iterator_at_index(rand(i + 1)), i);
    }
    return tree.size();
  };
  
}

TEMPLATE_LIST_TEST_CASE("ManagedTree benchmark - element access",
    "", TreeImpls) {

  using Tree = obt::ManagedTree<TestType>;

  static constexpr size_t kLength{1 << 10};

  SECTION("precondition: back insertions") {
    Tree tree;
    for (size_t i{0}; i < kLength; ++i) {
      tree.push_back(i);
    }

    {
      IndexRand rand;
      BENCHMARK("access at a random index") {
        return tree[rand(tree.size())];
      };
    }

    BENCHMARK("backward iteration") {
      size_t acc{0};
      for (auto i = tree.crbegin(); i != tree.crend(); ++i) {
        acc = (acc + *i) % 1234;
      }
      return acc;
    };
  }

  SECTION("precondition: random insertions") {
    Tree tree;
    IndexRand rand;
    for (size_t i{0}; i < kLength; ++i) {
      tree.insert(tree.get_iterator_at_index(rand(i + 1)), i);
    }

    {
      IndexRand rand;
      BENCHMARK("access at a random index") {
        return tree[rand(tree.size())];
      };
    }

    BENCHMARK("backward iteration") {
      size_t acc{0};
      for (auto i = tree.crbegin(); i != tree.crend(); ++i) {
        acc = (acc + *i) % 1234;
      }
      return acc;
    };
  }
}

TEMPLATE_LIST_TEST_CASE("ManagedTree benchmark - insert and erase",
    "", TreeImpls) {

  using Tree = obt::ManagedTree<TestType>;

  Tree tree;

  BENCHMARK_ADVANCED("insert one and erase one")(
      Catch::Benchmark::Chronometer meter) {
    Tree tree;
    IndexRand rand{};
    static constexpr size_t kInitialSize{1 << 12};
    for (size_t i{0}; i < kInitialSize; ++i) {
      tree.insert(tree.get_iterator_at_index(rand(i + 1)), i);
    }
    meter.measure([&tree, &rand]() {
          tree.insert(
              tree.get_iterator_at_index(rand(kInitialSize + 1)),
              rand(kInitialSize));
          return tree.erase(
              tree.get_iterator_at_index(
                rand(kInitialSize + 1)));
        });
  };

  BENCHMARK_ADVANCED("insert chunk and erase chunk")(
      Catch::Benchmark::Chronometer meter) {
    Tree tree;
    IndexRand rand{};
    static constexpr size_t kInitialSize{1 << 12};
    static constexpr size_t kChunkSize{1 << 8};
    for (size_t i{0}; i < kInitialSize; ++i) {
      tree.insert(tree.get_iterator_at_index(rand(i + 1)), i);
    }
    deque<Value> list;
    for (size_t i{0}; i < kChunkSize; ++i) {
      list.push_back(i);
    })
    meter.measure([&tree, &rand, &list]() {
          tree.insert(
              tree.get_iterator_at_index(rand(kInitialSize + 1)),
              list.begin(), list.end());
          auto begin{tree.get_iterator_at_index(rand(kInitialSize + 1))};
          return tree.erase(begin, begin + kChunkSize);
        });
  };
}

TEMPLATE_LIST_TEST_CASE(
    "ManagedTree benchmark - single tree operations",
    "",
    TreeImpls) {

  using Tree = obt::ManagedTree<TestType>;

  static constexpr size_t kNumOperations{1000};
  static constexpr size_t kMaxBulkSize{8};

  BENCHMARK("random operations") {
    Tree tree;
    IndexRand rand{};
    for (size_t counter{0}; counter < kNumOperations; ++counter) {
      size_t op_1{rand(tree.empty() ? 4 : 6)};

      switch (op_1) {
        case 0: { // Add an element to one of the two ends.
          size_t op_2{rand(8)};
          switch (op_2) {
            case 0: // push_front
              tree.push_front(counter);
              break;
            case 1: // emplace_front
              tree.emplace_front(counter);
              break;
            case 2: // insert to front
              tree.insert(tree.begin(), counter);
              break;
            case 3: // emplace to front
              tree.emplace(tree.begin(), counter);
              break;
            case 4: // push_back
              tree.push_back(counter);
              break;
            case 5: // emplace_back
              tree.emplace_back(counter);
              break;
            case 6: // insert to back
              tree.insert(tree.end(), counter);
              break;
            case 7: // emplace to back
              tree.emplace(tree.end(), counter);
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
              tree.insert(tree.get_iterator_at_index(index), counter);
              break;
            }
            case 1: { // emplace at index
              tree.emplace(tree.get_iterator_at_index(index), counter);
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

          tree.insert(
              tree.get_iterator_at_index(index),
              list_to_insert.begin(),
              list_to_insert.end());
          break;
        }
        case 3: { // Erase a random interval of elements.
          size_t begin{rand(tree.size())};
          size_t length{rand(kMaxBulkSize)};
          size_t end{min(begin + length, tree.size())};

          auto end_it{tree.get_iterator_at_index(end)};
          tree.erase(tree.get_iterator_at_index(begin), end_it);
          break;
        }
        case 4: { // Erase one element from one of the two ends.
          REQUIRE(!tree.empty());
          size_t op_2{rand(6)};
          switch (op_2) {
            case 0: // pop_front
              tree.pop_front();
              break;
            case 1: // erase at get_front_iterator()
              tree.erase(tree.get_front_iterator());
              break;
            case 2: // erase at begin()
              tree.erase(tree.cbegin());
              break;
            case 3: // pop_back
              tree.pop_back();
              break;
            case 4: // erase at get_back_iterator()
              tree.erase(tree.get_back_iterator());
              break;
            case 5: // erase at end() - 1
              tree.erase(std::prev(tree.cend()));
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
          tree.erase(tree.get_iterator_at_index(index));
          break;
        }
      }
    }
    return tree.size();
  };
}
