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
      os << '(' << std::setw(width - 2) << n->size << ')';
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
      os << '[' << std::setw(width - 2) << n->get_index() << ']';
      Node* r{n->right_child};
      if (r) {
        os << std::string(width * Node::get_size(r->left_child), ' ');
        os << std::string(width / 2, ' ') << '|' << std::string(width / 2, ' ');
        os << std::string(width * Node::get_size(r->right_child), ' ');
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

/**
 *  @brief
 *  Performs a list of deletions on a list.
 */
template<class List, class ErasureList>
void erase_from_list(List& list, ErasureList const& erasures) {
  for (auto& erasure : erasures) {
    list.erase(std::next(list.begin(), erasure));
  }
}

/**
 *  @brief
 *  Performs a list of deletions on a tree.
 */
template<class Tree, class ErasureList>
void erase_from_tree(Tree& tree, ErasureList const& erasures) {
  for (auto& erasure : erasures) {
    tree.erase_at_index(erasure);
  }
}

/**
 *  @brief
 *  Checks if elements in a tree when traversed in-order are the same as
 *    elements in a list.
 */
template<class Tree, class List>
bool tree_equals_list(Tree const& tree, List const& list) {
  if (tree.size() != list.size()) {
    return false;
  }
  using Node = typename Tree::Node;
  size_t i{0};
  Node const* n{tree.find_first_node()};
  while (n && i < list.size()) {
    if (list[i] != n->data) {
      return false;
    }
    n = n->find_next_node();
    ++i;
  }
  return i == list.size();
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

static constexpr Insertion test_insertions_2[]{
    {0, "A"},
    {0, "B"},
    {0, "C"},
    {0, "D"},
    {2, "E"},
    {2, "F"},
    {3, "G"},
    {7, "H"},
    {8, "I"},
    {8, "J"}};

static constexpr Insertion test_insertions_3[]{
    {0, "a"},
    {0, "b"},
    {1, "c"},
    {3, "d"},
    {3, "e"},
    {3, "f"},
    {5, "g"},
    {1, "h"},
    {8, "i"},
    {8, "j"}};

using namespace std;

TEST_CASE("OrderedBinaryTree -- insert nodes and iterate") {
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

  SECTION("get_index") {
    cout << "Checking that get_index() is the inverse of find_node_at_index():"
        "\n  ";
    for (size_t i{0}; i < tree.size(); ++i) {
      Node* n{tree.find_node_at_index(i)};
      size_t j{n->get_index()};
      cout << "| " << i << (i == j ? " == " : "!=") << j << " ";
      CHECK(i == j);
    }
    cout << endl;
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
        // j = i + steps
        Node* n_j{tree.find_node_at_index(i + steps)};
        Node* n_steps_after_i{n_i->find_next_node(steps)};
        CHECK(n_j == n_steps_after_i);
      }
    }
  }

  SECTION("find_prev_node") {
    cout << "Iterating through nodes backwards with find_prev_node():\n  ";
    vector<string> inorder;
    for (Node* n{tree.find_last_node()}; n; n = n->find_prev_node()) {
      inorder.push_back(n->data);
      cout << n->data << " ";
    }
    cout << endl;

    CHECK(equal(
        inorder_golden.begin(), inorder_golden.end(),
        inorder.rbegin(), inorder.rend()));
  }

  SECTION("find_prev_node(steps)") {
    for (size_t i{0}; i < tree.size(); ++i) {
      Node* n_i{tree.find_node_at_index(i)};
      for (size_t steps{0}; steps <= i + 1; ++steps) {
        // j = i - steps
        Node* n_j{steps <= i ? tree.find_node_at_index(i - steps) : nullptr};
        Node* n_steps_before_i{n_i->find_prev_node(steps)};
        CHECK(n_j == n_steps_before_i);
      }
    }
  }

  tree.delete_nodes();
}

TEST_CASE("OrderedBinaryTree -- clone") {
  using Tree = obt::OrderedBinaryTree<string>;
  using Node = typename Tree::Node;

  Tree tree;
  vector<string> list;

  for (size_t times{0}; times < 5; ++times) {
    insert_to_tree(tree, test_insertions_1);
    insert_to_list(list, test_insertions_1);
    {
      Tree cloned_tree{tree.clone()};
      vector<string> cloned_list{list};

      insert_to_tree(cloned_tree, test_insertions_2);
      insert_to_list(cloned_list, test_insertions_2);

      CHECK(tree_equals_list(cloned_tree, cloned_list));
      CHECK(!tree_equals_list(cloned_tree, list));
      CHECK(!tree_equals_list(tree, cloned_list));

      cloned_tree.delete_nodes();
    }
  }
  tree.delete_nodes();
}

TEST_CASE("OrderedBinaryTree -- swap") {
  using Tree = obt::OrderedBinaryTree<string>;
  using Node = typename Tree::Node;

  Tree tree;
  vector<string> list;

  insert_to_tree(tree, test_insertions_1);
  insert_to_list(list, test_insertions_1);

  for (size_t i{0}; i < tree.size(); ++i) {
    for (size_t j{0}; j < tree.size(); ++j) {
      std::swap(list[i], list[j]);

      Node* n_i{tree.find_node_at_index(i)};
      Node* n_j{tree.find_node_at_index(j)};
      tree.swap(n_i, n_j);

      CHECK(tree_equals_list(tree, list));
    }
  }
  tree.delete_nodes();
}

TEST_CASE("OrderedBinaryTree -- insert and remove subtrees") {
  using Tree = obt::OrderedBinaryTree<string>;
  using Node = typename Tree::Node;

  Tree tree_1;
  insert_to_tree(tree_1, test_insertions_2);

  vector<string> inorder_1;
  insert_to_list(inorder_1, test_insertions_2);

  Tree tree_2;
  insert_to_tree(tree_2, test_insertions_3);

  vector<string> inorder_2;
  insert_to_list(inorder_2, test_insertions_3);

  cout << "tree_1:\n";
  dump_tree(cout, tree_1);
  cout << "tree_2:\n";
  dump_tree(cout, tree_2);

  for (size_t i{0}; i <= tree_1.size(); ++i) {
    Tree tree_a{tree_1.clone()};
    Tree tree_b{tree_2.clone()};
    tree_a.insert_at_index(i, tree_b.root);

    cout << "Joined tree_2 to tree_1 at index " << i << ":\n";
    dump_tree(cout, tree_a);

    vector<string> inorder_a{inorder_1};
    inorder_a.insert(
        std::next(inorder_a.begin(), i),
        inorder_2.begin(), inorder_2.end());
    
    CHECK(tree_equals_list(tree_a, inorder_a));

    if (i > 0) {
      // If the subtree was joined at index > 0, removing the subtree at index
      //   `i` would reverse the insert operation.
      Tree tree_c{tree_1.unlink_at_index(i)};
      CHECK(tree_equals_list(tree_c, inorder_2));
      tree_c.delete_nodes();

      CHECK(tree_equals_list(tree_a, inorder_1));
    }

    tree_a.delete_nodes();
    break;
  }

  tree_2.delete_nodes();
  tree_1.delete_nodes();
}

