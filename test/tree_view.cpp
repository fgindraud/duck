#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/combinator.h>
#include <duck/tree_view.h>

#include <memory>
#include <vector>

struct node_t {
	std::vector<std::unique_ptr<node_t>> childrens;
	node_t * parent{nullptr};
	int value;
	node_t (int v) : value (v) {}
};

std::unique_ptr<node_t> append (std::unique_ptr<node_t> n) {
	return n;
}
template <typename... Types>
std::unique_ptr<node_t> append (std::unique_ptr<node_t> n, std::unique_ptr<node_t> dep,
                                Types &&... other_deps) {
	dep->parent = n.get ();
	n->childrens.push_back (std::move (dep));
	return append (std::move (n), std::forward<Types> (other_deps)...);
}
template <typename... Args> std::unique_ptr<node_t> N (int v, Args &&... args) {
	return append (std::unique_ptr<node_t>{new node_t{v}}, std::forward<Args> (args)...);
}

struct tree_view : duck::tree_topology_view {
	const node_t * root;

	tree_view (const node_t * root) : root (root) {}
	tree_view (const std::unique_ptr<node_t> & root) : tree_view (root.get ()) {}

	static inline const node_t * convert (node_id_t id) {
		return reinterpret_cast<const node_t *> (id.value);
	}
	static inline node_id_t convert (const node_t * node) {
		return node_id_t (reinterpret_cast<std::intptr_t> (node));
	}

	node_id_t invalid_node () const final { return convert (nullptr); }
	edge_id_t invalid_edge () const final { return father_edge (invalid_node ()); }
	node_id_t root_node () const final { return convert (root); }
	node_id_t father_node (edge_id_t id) const final {
		auto * child = convert (child_node (id));
		return convert (child->parent);
	}
	node_id_t child_node (edge_id_t id) const final { return node_id_t (id.value); }
	edge_id_t father_edge (node_id_t id) const final { return edge_id_t (id.value); }
	std::vector<edge_id_t> child_edges (node_id_t id) const final {
		return duck::to_container<std::vector<edge_id_t>> (
		    convert (id)->childrens | duck::map ([this](const std::unique_ptr<node_t> & child) {
			    return father_edge (convert (child.get ()));
		    }));
	}
};

TEST_CASE ("test") {
	auto tree = N (2, N (0), N (1));
	CHECK (tree != nullptr);

	auto view = tree_view (tree);
	CHECK (view.root_node () != view.invalid_node ());
	CHECK (tree.get () == tree_view::convert (view.root_node()));

	auto r = duck::dfs_range (view);
	auto b = r.begin();
	CHECK (b != r.end());
	CHECK (*b == view.root_node());

	auto root_index = duck::front (r);
	CHECK (tree.get () == tree_view::convert (root_index));
}
