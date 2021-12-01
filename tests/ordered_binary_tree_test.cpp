#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <ordered_binary_trees/ordered_binary_tree.hpp>
#include <ordered_binary_trees/ordered_binary_tree_node.hpp>
#include <ordered_binary_trees/ordered_binary_tree_iterator.hpp>

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
template<class NodeT>
void dump_tree(
    std::ostream& os,
    obt::OrderedBinaryTree<NodeT> const& tree,
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
  Node const* n{tree.first};
  while (n && i < list.size()) {
    if (list[i] != n->data) {
      return false;
    }
    n = n->find_next_node();
    ++i;
  }
  if (i != list.size()) {
    return false;
  }
  n = tree.last;
  while (n && i > 0) {
    --i;
    if (list[i] != n->data) {
      return false;
    }
    n = n->find_prev_node();
  }
  return i == 0;
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
  using Node = obt::OrderedBinaryTreeNode<string>;
  using Tree = obt::OrderedBinaryTree<Node>;

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

  SECTION("find_next_node and find_prev_node") {
    cout << "Iterating through nodes with "
        "find_next_node() and find_prev_node()...\n";
    CHECK(tree_equals_list(tree, inorder_golden));
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

  tree.destroy_all_nodes();
}

TEST_CASE("OrderedBinaryTree -- clone") {
  using Node = obt::OrderedBinaryTreeNode<string>;
  using Tree = obt::OrderedBinaryTree<Node>;

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

      cloned_tree.destroy_all_nodes();
    }
  }
  tree.destroy_all_nodes();
}

TEST_CASE("OrderedBinaryTree -- insert positions") {
  using Node = obt::OrderedBinaryTreeNode<string>;
  using Tree = obt::OrderedBinaryTree<Node>;
  using InsertPosition = Node::InsertPosition;

  Tree tree;
  vector<string> list;
  insert_to_tree(tree, test_insertions_1);
  insert_to_list(list, test_insertions_1);

  for (size_t i{0}; i < tree.size(); ++i) {
    Node* n{tree.find_node_at_index(i)};

    InsertPosition pos_1{tree.get_insert_position_for_index(i)};
    InsertPosition pos_2{n->get_prev_insert_position()};
    CHECK(pos_1 == pos_2);

    InsertPosition pos_3{tree.get_insert_position_for_index(i + 1)};
    InsertPosition pos_4{n->get_next_insert_position()};
    CHECK(pos_3 == pos_4);
  }

  tree.destroy_all_nodes();
}

TEST_CASE("OrderedBinaryTree -- insert and remove subtrees") {
  using Node = obt::OrderedBinaryTreeNode<string>;
  using Tree = obt::OrderedBinaryTree<Node>;

  Tree tree_1;
  vector<string> list_1;
  insert_to_tree(tree_1, test_insertions_2);
  insert_to_list(list_1, test_insertions_2);

  SECTION("insert and unlink") {
    Tree tree_2;
    vector<string> list_2;
    insert_to_tree(tree_2, test_insertions_3);
    insert_to_list(list_2, test_insertions_3);

    cout << "tree_1:\n";
    dump_tree(cout, tree_1);
    cout << "tree_2:\n";
    dump_tree(cout, tree_2);

    for (size_t i{0}; i <= tree_1.size(); ++i) {
      Tree tree_a{tree_1.clone()};
      Tree tree_b{tree_2.clone()};
      tree_a.link_subtree_at_index(i, std::move(tree_b));

      cout << "Joined tree_2 to tree_1 at index " << i << ":\n";
      dump_tree(cout, tree_a);

      CHECK(tree_b.empty());

      vector<string> list_a{list_1};
      list_a.insert(
          std::next(list_a.begin(), i),
          list_2.begin(), list_2.end());
      
      CHECK(tree_equals_list(tree_a, list_a));

      if (i > 0) {
        // If the subtree was joined at index > 0, removing the subtree at index
        //   `i` would reverse the insert operation.
        Tree tree_c{tree_1.unlink_subtree_at_index(i).first};
        CHECK(tree_equals_list(tree_c, list_2));
        tree_c.destroy_all_nodes();

        CHECK(tree_equals_list(tree_a, list_1));
      }

      tree_a.destroy_all_nodes();
      break;
    }

    tree_2.destroy_all_nodes();
  }

  SECTION("unlink and insert") {
    for (size_t i{0}; i < tree_1.size(); ++i) {
      Tree tree_a{tree_1.clone()};

      cout << "Starting tree:\n";
      dump_tree(cout, tree_a);

      auto unlink_result{tree_a.unlink_subtree_at_index(i)};
      Tree tree_b{std::move(unlink_result.first)};
      auto pos{unlink_result.second};

      cout << "After unlinking at index " << i
          << " (data = " << tree_b.root->data << ")\n";
      dump_tree(cout, tree_a);

      cout << "Unlinked subtree:\n";
      dump_tree(cout, tree_b);

      CHECK(tree_a.size() + tree_b.size() == tree_1.size());

      tree_a.link_subtree(pos, std::move(tree_b));

      CHECK(tree_b.empty());

      CHECK(tree_equals_list(tree_a, list_1));

      tree_a.destroy_all_nodes();
    }
  }

  tree_1.destroy_all_nodes();
}

TEST_CASE("OrderedBinaryTree -- rotate") {
  using Node = obt::OrderedBinaryTreeNode<string>;
  using Tree = obt::OrderedBinaryTree<Node>;

  Tree tree;
  vector<string> list;
  insert_to_tree(tree, test_insertions_1);
  insert_to_list(list, test_insertions_1);

  cout << "Starting tree:\n";
  dump_tree(cout, tree);

  SECTION("rotate_left") {
    for (size_t i{0}; i < tree.size(); ++i) {
      Node* n{tree.find_node_at_index(i)};
      if (!n->right_child) {
        continue;
      }
      tree.rotate_left(n);
      n->update_size();
      if (n->parent) {
        n->parent->update_size();
      }
      cout << "After rotate_left at index " << i
          << " (value: " << n->data << "):\n";
      dump_tree(cout, tree);
      CHECK(tree_equals_list(tree, list));
    }
  }

  SECTION("rotate_right") {
    for (size_t i{0}; i < tree.size(); ++i) {
      Node* n{tree.find_node_at_index(i)};
      if (!n->left_child) {
        continue;
      }
      tree.rotate_right(n);
      n->update_size();
      if (n->parent) {
        n->parent->update_size();
      }
      cout << "After rotate_right at index " << i
          << " (value: " << n->data << "):\n";
      dump_tree(cout, tree);
      CHECK(tree_equals_list(tree, list));
    }
  }
}

TEST_CASE("OrderedBinaryTree -- splay") {
  using Node = obt::OrderedBinaryTreeNode<string>;
  using Tree = obt::OrderedBinaryTree<Node>;

  Tree tree;
  vector<string> list;
  insert_to_tree(tree, test_insertions_1);
  insert_to_list(list, test_insertions_1);

  SECTION("splay to root") {
    cout << "Starting tree:\n";
    dump_tree(cout, tree);

    for (size_t i{0}; i < tree.size(); ++i) {
      Node* n{tree.find_node_at_index(i)};
      if (n->is_root()) {
        continue;
      }
      tree.splay(n);
      cout << "After splaying at index " << i
          << " (value: " << n->data << ") to root:\n";
      dump_tree(cout, tree);
      CHECK(tree_equals_list(tree, list));
      CHECK(n == tree.root);
      CHECK(n->is_root());
    }
  }

  SECTION("splay under root") {
    cout << "Starting tree:\n";
    dump_tree(cout, tree);

    for (size_t i{0}; i < tree.size(); ++i) {
      Node* n{tree.find_node_at_index(i)};
      Node* root{tree.root};
      if (n == root) {
        continue;
      }
      tree.splay(n, root);
      cout << "After splaying at index " << i
          << " (value: " << n->data << ") under root:\n";
      dump_tree(cout, tree);
      CHECK(tree_equals_list(tree, list));
      CHECK(n->parent == root);
    }
  }
}

TEST_CASE("OrderedBinaryTree -- swap") {
  using Node = obt::OrderedBinaryTreeNode<string>;
  using Tree = obt::OrderedBinaryTree<Node>;

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
  tree.destroy_all_nodes();
}

TEST_CASE("OrderedBinaryTree -- erase") {
  using Node = obt::OrderedBinaryTreeNode<string>;
  using Tree = obt::OrderedBinaryTree<Node>;

  Tree tree;
  vector<string> list;

  insert_to_tree(tree, test_insertions_1);
  insert_to_list(list, test_insertions_1);

  for (size_t step_size{0}; step_size + 1 < tree.size(); ++step_size) {
    Tree tree_a{tree.clone()};
    vector<string> list_a{list};

    cout << "Starting tree:\n";
    dump_tree(cout, tree_a);

    size_t i{0};
    for (; tree_a.size() > 0; i += step_size) {
      size_t j{i % tree_a.size()};
      cout << "Erase at index " << j
          << " (data = " << tree_a.find_node_at_index(j)->data << "):\n";

      if (step_size % 2 == 0) {
        delete std::get<2>(tree_a.erase_at_index(j));
      } else {
        tree_a.delete_at_index(j);
      }

      list_a.erase(list_a.begin() + j);

      dump_tree(cout, tree_a);
      CHECK(tree_equals_list(tree_a, list_a));
    }

    tree_a.destroy_all_nodes();
  }
  tree.destroy_all_nodes();
}

TEST_CASE("OrderedBinaryTreeIterator") {
  using Node = obt::OrderedBinaryTreeNode<string>;
  using Tree = obt::OrderedBinaryTree<Node>;
  using Iterator = obt::OrderedBinaryTreeIterator<Tree, false, false>;
  using ConstIterator = obt::OrderedBinaryTreeIterator<Tree, true, false>;
  using ReverseIterator = obt::OrderedBinaryTreeIterator<Tree, false, true>;
  using ConstReverseIterator =
      obt::OrderedBinaryTreeIterator<Tree, true, true>;

  Tree tree;
  vector<string> list;

  insert_to_tree(tree, test_insertions_1);
  insert_to_list(list, test_insertions_1);

  Iterator begin{&tree, tree.first};
  Iterator end{&tree, nullptr};
  ConstIterator cbegin{&tree, tree.first};
  ConstIterator cend{&tree, nullptr};
  ReverseIterator rbegin{&tree, tree.last};
  ReverseIterator rend{&tree, nullptr};
  ConstReverseIterator crbegin{&tree, tree.last};
  ConstReverseIterator crend{&tree, nullptr};

  cout << "Tree:\n";
  dump_tree(cout, tree);

  SECTION("one step forward") {
    cout << "Testing operator++ -- iterating forward:\n ";
    auto i{begin};
    auto ci{cbegin};
    for (; i != end && ci != cend; ++i, ci++) {
      cout << " " << *i;
      CHECK(*i == *ci);
    }
    cout << "\n";
    CHECK(i == end);
    CHECK(ci == cend);

    CHECK(std::equal(list.begin(), list.end(), begin, end));
    CHECK(std::equal(list.cbegin(), list.cend(), cbegin, cend));
  }

  SECTION("one step backward") {
    cout << "Testing operator-- -- iterating backward:\n ";
    auto i{begin};
    auto ci{cbegin};
    while (i != begin && ci != cbegin) {
      i--;
      --ci;
      cout << " " << *i;
      CHECK(*i == *ci);
    }
    cout << "\n";
    CHECK(i == begin);
    CHECK(ci == cbegin);
  }

  SECTION("multiple steps") {
    for (size_t i{0}; i <= tree.size(); ++i) {
      auto it_i{begin + i};
      CHECK(it_i == i + begin);
      for (size_t j{0}; j <= tree.size(); ++j) {
        auto it_j{begin + j};
        ssize_t dist{static_cast<ssize_t>(j) - static_cast<ssize_t>(i)};

        // iterator + distance
        CHECK(it_i + dist == it_j);
        // distance + iterator
        CHECK(dist + it_i == it_j);
        // iterator - distance
        CHECK(it_i == it_j - dist);
        // -distance + iterator
        CHECK(it_i == -dist + it_j);
        // iterator - iterator
        CHECK(it_j - it_i == dist);

        // operator[]
        if (i < tree.size()) {
          CHECK(it_i[0] == list[i]);
          CHECK(it_j[-dist] == *it_i);
        }
        if (j < tree.size()) {
          CHECK(it_j[0] == list[j]);
          CHECK(it_i[dist] == *it_j);
        }

        // get_index
        CHECK(it_i.get_index() == i);
        CHECK(it_j.get_index() == j);

        // Comparison operators
        CHECK((i == j ? (it_i == it_j) : true));
        CHECK((i != j ? (it_i != it_j) : true));
        CHECK((i > j ? (it_i > it_j) : true));
        CHECK((i >= j ? (it_i >= it_j) : true));
        CHECK((i < j ? (it_i < it_j) : true));
        CHECK((i <= j ? (it_i <= it_j) : true));

        // Assignment
        it_j = it_i;
        CHECK(it_j == it_i);
      }
    }
  }

  SECTION("conversion to const") {
    auto i{begin};
    auto ci{cbegin};
    while (true) {
      // Constructor
      ConstIterator const_i_1{i};
      CHECK(const_i_1 == ci);

      // Assignment
      ConstIterator const_i_2;
      const_i_2 = i;
      CHECK(const_i_2 == ci);
      const_i_2 = ci;
      CHECK(const_i_2 == ci);
      ConstIterator const_i_3{ci};
      CHECK(const_i_2 == const_i_3);

      if (i == end || ci == cend) {
        break;
      }
      i++;
      ++ci;
    }
  }

  SECTION("reverse iterators") {
    CHECK(std::equal(list.rbegin(), list.rend(), rbegin, rend));
    CHECK(std::equal(list.crbegin(), list.crend(), crbegin, crend));

    // get_index
    auto ri{rbegin};
    for (size_t i = 0; i < tree.size() && ri != rend; ++i, ++ri) {
      CHECK(ri.get_index() == i);
      CHECK(list[tree.size() - 1 - ri.get_index()] == *ri);
    }
    CHECK(ri == rend);
    CHECK(ri.get_index() == tree.size());
  }

  SECTION("conversion to reverse") {
    auto i{begin};
    auto ri{rend};
    while (true) {
      CHECK(i.make_reverse_iterator() == ri);
      CHECK(ri.make_reverse_iterator() == i);
      if (ri == rbegin) {
        CHECK(i == end);
        break;
      }
      ri--;
      CHECK(*i == *ri);
      ++i;
    }
  }

  tree.destroy_all_nodes();
}
