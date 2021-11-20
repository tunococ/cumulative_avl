#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <ordered_binary_trees/ordered_binary_tree.hpp>

#include <catch2/catch_test_macros.hpp>

namespace obt = ordered_binary_trees;

/**
 *  @brief
 *  Dumps the tree rooted at `n` to output stream `os`.
 *  `width` is the maximum size of node output. `width` must be an odd number.
 */
template<class Node>
void dump_tree(std::ostream& os,
    Node* n,
    size_t width = 5) {
  if (!n) {
    return;
  }
  std::vector<Node*> curr_level_nodes;
  std::vector<Node*> next_level_nodes;

  next_level_nodes.emplace_back(n);
  bool has_next_level{true};
  while (has_next_level) {
    has_next_level = false;
    curr_level_nodes = std::move(next_level_nodes);
    next_level_nodes.clear();
    for (size_t i{0}; i < curr_level_nodes.size(); ++i) {
      Node* n{curr_level_nodes[i]};
      if (!n) {
        os << std::string(width, ' ');
        next_level_nodes.push_back(nullptr);
        continue;
      }
      Node* l{n->left_child};
      if (l) {
        os << std::string(width * Node::get_size(l->left_child), ' ');
        os << std::string(width / 2, ' ') << '+' << std::string(width / 2, '-');
        os << std::string(width * Node::get_size(l->right_child), '-');
        next_level_nodes.push_back(l);
        has_next_level = true;
      }
      os << '[' << std::setw(width - 2) << n->size << ']';
      next_level_nodes.push_back(nullptr);
      Node* r{n->right_child};
      if (r) {
        os << std::string(width * Node::get_size(r->left_child), '-');
        os << std::string(width / 2, '-') << '+' << std::string(width / 2, ' ');
        os << std::string(width * Node::get_size(r->right_child), ' ');
        next_level_nodes.push_back(r);
        has_next_level = true;
      }
    }
    os << '\n';
    for (size_t i{0}; i < curr_level_nodes.size(); ++i) {
      Node* n{curr_level_nodes[i]};
      if (!n) {
        os << std::string(width, ' ');
        continue;
      }
      Node* l{n->left_child};
      if (l) {
        os << std::string(width * Node::get_size(l->left_child), ' ');
        os << std::string(width / 2, ' ') << '|' << std::string(width / 2, ' ');
        os << std::string(width * Node::get_size(l->right_child), ' ');
      }
      os << std::setw(width) << n->data;
      Node* r{n->right_child};
      if (r) {
        os << std::string(width * Node::get_size(r->left_child), ' ');
        os << std::string(width / 2, ' ') << '|' << std::string(width / 2, ' ');
        os << std::string(width * Node::get_size(r->right_child), ' ');
      }
    }
    os << '\n';
  }
}

/**
 *  @brief
 *  Calls `dump_tree(os, tree.root, width)`.
 */
template<class DataT, class SizeT>
void dump_tree(
    std::ostream& os,
    obt::OrderedBinaryTree<DataT, SizeT> const& tree,
    size_t width = 5) {
  dump_tree(os, tree.root, width);
}

/**
 *  @brief
 *  Type of an index-based *insert* operation.
 * 
 *  An insertion requires an index and a value.
 */
using Insertion = std::pair<size_t, char const*>;

/**
 *  @brief
 *  Type of an index-based *erase* operation.
 */
using Erasure = size_t;

/**
 *  @brief
 *  Performs a list of insertions on a list.
 */
template<class List, class InsertionList>
void insert_to_list(List& list, InsertionList const& insertions) {
  for (auto& insertion : insertions) {
    list.emplace(std::next(list.begin(), insertion.first), insertion.second);
  }
}

/**
 *  @brief
 *  Performs a list of insertions on an ordered tree.
 */
template<class Tree, class InsertionList>
void insert_to_tree(Tree& tree, InsertionList const& insertions) {
  for (auto& insertion : insertions) {
    tree.emplace_at_index(insertion.first, insertion.second);
  }
}

static constexpr Insertion test_insertions_1[]{
    {0, "a"},
    {0, "b"},
    {0, "c"},
    {0, "d"},
    {1, "e"},
    {1, "f"},
    {3, "g"},
    {3, "h"},
    {8, "i"},
    {9, "j"},
    {10, "k"},
    {11, "l"},
    {9, "m"},
    {10, "n"},
    {9, "o"},
    {8, "p"},
    {8, "q"},
    {8, "r"},
    {8, "s"},
    {11, "t"},
    {12, "u"},
    {20, "v"},
    {21, "w"},
    {23, "x"},
    {24, "y"},
    {23, "z"}};

using namespace std;

TEST_CASE("OrderedBinaryTree -- insert and iterate") {
  using Tree = obt::OrderedBinaryTree<string>;
  using Node = typename Tree::Node;

  Tree tree;
  insert_to_tree(tree, test_insertions_1);

  vector<string> inorder_golden;
  insert_to_list(inorder_golden, test_insertions_1);

  SECTION("dump_tree") {
    dump_tree(cout, tree);
    cout << "Expected node order:\n  ";
    for (auto& data : inorder_golden) {
      cout << data << " ";
    }
    cout << endl;
  }

  SECTION("traverse_inorder") {
    cout << "Iterating through nodes with traverse_inorder():\n  ";
    vector<string> inorder;
    tree.traverse_inorder(
        [&inorder](Node* n) {
            inorder.push_back(n->data);
            cout << n->data << " ";
        });
    cout << endl;

    CHECK(equal(
        inorder_golden.begin(), inorder_golden.end(),
        inorder.begin(), inorder.end()));
  }

  SECTION("find_node_at_index") {
    cout << "Iterating through nodes with find_node_at_index():\n  ";
    vector<string> inorder;
    for (size_t i{0}; i < tree.size(); ++i) {
      Node* n{tree.find_node_at_index(i)};
      REQUIRE(n);
      inorder.push_back(n->data);
      cout << n->data << " ";
    }
    cout << endl;

    CHECK(equal(
        inorder_golden.begin(), inorder_golden.end(),
        inorder.begin(), inorder.end()));
  }

  SECTION("find_next_node") {
    cout << "Iterating through nodes with find_next_node():\n  ";
    vector<string> inorder;
    for (Node* n{tree.find_first_node()}; n; n = n->find_next_node()) {
      inorder.push_back(n->data);
      cout << n->data << " ";
    }
    cout << endl;

    CHECK(equal(
        inorder_golden.begin(), inorder_golden.end(),
        inorder.begin(), inorder.end()));
  }

  SECTION("find_next_node(steps)") {
    for (size_t i{0}; i < tree.size(); ++i) {
      Node* n_i{tree.find_node_at_index(i)};
      for (size_t steps{0}; steps + i <= tree.size(); ++steps) {
        size_t const j{i + steps};
        Node* n_j{tree.find_node_at_index(j)};
        Node* n_i_plus_steps{n_i->find_next_node(steps)};
        CHECK(n_j == n_i_plus_steps);
      }
    }
  }

  tree.delete_nodes();
}
