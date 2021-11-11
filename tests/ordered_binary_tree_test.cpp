#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <ordered_binary_trees/ordered_binary_tree.hpp>

#include <catch2/catch_test_macros.hpp>

namespace obt = ordered_binary_trees;

// Dumps a binary tree to an output stream.
//
// `width` must be an odd number.
template<class Node>
void dump_tree(std::ostream& os,
    Node* n,
    size_t width = 5) {
  using Tree = typename Node::Tree;
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
        os << std::string(width * Tree::get_size(l->left_child), ' ');
        os << std::string(width / 2, ' ') << '+' << std::string(width / 2, '-');
        os << std::string(width * Tree::get_size(l->right_child), '-');
        next_level_nodes.push_back(l);
        has_next_level = true;
      }
      os << '[' << std::setw(width - 2) << n->size << ']';
      next_level_nodes.push_back(nullptr);
      Node* r{n->right_child};
      if (r) {
        os << std::string(width * Tree::get_size(r->left_child), '-');
        os << std::string(width / 2, '-') << '+' << std::string(width / 2, ' ');
        os << std::string(width * Tree::get_size(r->right_child), ' ');
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
        os << std::string(width * Tree::get_size(l->left_child), ' ');
        os << std::string(width / 2, ' ') << '|' << std::string(width / 2, ' ');
        os << std::string(width * Tree::get_size(l->right_child), ' ');
      }
      os << std::setw(width) << n->data;
      Node* r{n->right_child};
      if (r) {
        os << std::string(width * Tree::get_size(r->left_child), ' ');
        os << std::string(width / 2, ' ') << '|' << std::string(width / 2, ' ');
        os << std::string(width * Tree::get_size(r->right_child), ' ');
      }
    }
    os << '\n';
  }
}

template<class D>
void dump_tree(std::ostream& os,
    typename obt::OrderedBinaryTree<D>& tree,
    size_t width = 5) {
  dump_tree(os, tree.root, width);
}

template<class Tree, class... Args>
void create_node_at_index(Tree& tree, size_t index, Args&&... args) {
  using Node = typename Tree::Node;
  
  Node* node{Node::with_data(std::forward<Args>(args)...)};
  tree.insert_at_index(index, node);
}

obt::OrderedBinaryTree<std::string> make_test_tree() {
  using Tree = obt::OrderedBinaryTree<std::string>;
  using Node = typename Tree::Node;
  
  Tree tree;

  create_node_at_index(tree, 0, "a");
  create_node_at_index(tree, 0, "b");
  create_node_at_index(tree, 0, "c");
  create_node_at_index(tree, 0, "d");
  create_node_at_index(tree, 1, "e");
  create_node_at_index(tree, 1, "f");
  create_node_at_index(tree, 3, "g");
  create_node_at_index(tree, 3, "h");
//  create_node_at_index(tree, 3, "i");
//  create_node_at_index(tree, 5, "j");

  return tree;
}

template<class D>
void deallocate_tree(obt::OrderedBinaryTree<D>& tree) {
  using Tree = obt::OrderedBinaryTree<D>;
  using Node = typename Tree::Node;
  Tree::traverse_postorder(tree.root, [](Node* n) { delete n; });
}

using namespace std;

TEST_CASE("OrderedBinaryTree") {
  using Tree = obt::OrderedBinaryTree<string>;
  using Node = typename Tree::Node;

  Tree tree;

  create_node_at_index(tree, 0, "a");
  create_node_at_index(tree, 0, "b");
  create_node_at_index(tree, 0, "c");
  create_node_at_index(tree, 0, "d");
  create_node_at_index(tree, 1, "e");
  create_node_at_index(tree, 1, "f");
  create_node_at_index(tree, 3, "g");
  create_node_at_index(tree, 3, "h");
  create_node_at_index(tree, 8, "i");
  create_node_at_index(tree, 9, "j");
  create_node_at_index(tree, 10, "k");
  create_node_at_index(tree, 11, "l");
  create_node_at_index(tree, 9, "m");
  create_node_at_index(tree, 10, "n");
  create_node_at_index(tree, 9, "o");
  create_node_at_index(tree, 8, "p");
  create_node_at_index(tree, 8, "q");
  create_node_at_index(tree, 8, "r");
  create_node_at_index(tree, 8, "s");
  create_node_at_index(tree, 11, "t");
  create_node_at_index(tree, 12, "u");
  create_node_at_index(tree, 20, "v");
  create_node_at_index(tree, 21, "w");

  dump_tree(cout, tree);

  vector<string> inorder;

  cout << "Iterating by calling traverse_inorder:\n  ";
  Tree::traverse_inorder(
      tree.root,
      [&inorder](Node* n) {
          inorder.push_back(n->data);
          cout << n->data << " ";
      });
  cout << "\n";

  for (size_t i = 0; i < tree.root->size; ++i) {
    Node* n{Tree::find_node_at_index(tree.root, i)};
    REQUIRE(n);
    CHECK(n->data == inorder[i]);
  }

  cout << "Iterating by calling find_next_node:\n  ";
  vector<string> inorder2;
  for (Node* n{Tree::find_first_node(tree.root)}
      ; n
      ; n = Tree::find_next_node(n)) {
    inorder2.push_back(n->data);
    cout << n->data << " ";
  }
  cout << "\n";
  
  CHECK(equal(
      inorder.begin(), inorder.end(),
      inorder2.begin(), inorder2.end()));

  deallocate_tree(tree);
}
