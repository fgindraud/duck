#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <memory>
#include <vector>

#include <duck/range/combinator.h>
#include <duck/tree_view.h>

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

struct tree_view : duck::bidirectional_tree_topology {
	const node_t * root;

	using node_id = duck::topology_node_id;
	using edge_id = duck::topology_edge_id;

	tree_view (const node_t * root) : root (root) {}
	tree_view (const std::unique_ptr<node_t> & root) : tree_view (root.get ()) {}

	static inline const node_t * convert (node_id id) {
		return reinterpret_cast<const node_t *> (id.value);
	}
	static inline node_id convert (const node_t * node) {
		return node_id (reinterpret_cast<std::intptr_t> (node));
	}

	node_id invalid_node () const final { return convert (nullptr); }
	edge_id invalid_edge () const final { return father_edge (invalid_node ()); }
	node_id root_node () const final { return convert (root); }
	node_id father_node (edge_id id) const final {
		auto * child = convert (child_node (id));
		return convert (child->parent);
	}
	node_id child_node (edge_id id) const final { return node_id (id.value); }
	edge_id father_edge (node_id id) const final { return edge_id (id.value); }
	std::vector<edge_id> child_edges (node_id id) const final {
		return duck::to_container<std::vector<edge_id>> (
		    convert (id)->childrens | duck::map ([this](const std::unique_ptr<node_t> & child) {
			    return father_edge (convert (child.get ()));
		    }));
	}
};

TEST_CASE ("test") {
	auto tree = N (1, N (2, N (3), N (4)), N (5), N (6, N (7), N (8), N (9)));
	CHECK (tree != nullptr);

	auto view = tree_view (tree);
	CHECK (view.root_node () != view.invalid_node ());
	CHECK (tree.get () == tree_view::convert (view.root_node ()));

	{
		// forward_dfs_range
		auto r = duck::forward_dfs_range (view);
		CHECK (tree.get () == tree_view::convert (duck::front (r)));
		CHECK (!duck::empty (r));
		auto dfs_values =
		    r | duck::map ([](tree_view::node_id id) { return tree_view::convert (id)->value; });
		CHECK (dfs_values == duck::range (1, 10));

		CHECK (duck::empty (duck::forward_dfs_range (tree_view (nullptr))));
	}
	{
		// input_dfs_range
		auto r = duck::input_dfs_range (view);
		CHECK (tree.get () == tree_view::convert (duck::front (r)));
		CHECK (!duck::empty (r));
		auto dfs_values =
		    r | duck::map ([](tree_view::node_id id) { return tree_view::convert (id)->value; });
		CHECK (dfs_values == duck::range (1, 10));

		CHECK (duck::empty (duck::input_dfs_range (tree_view (nullptr))));
	}
}
