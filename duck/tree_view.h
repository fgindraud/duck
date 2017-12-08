#pragma once

// Tree topology view
// STATUS: WIP

#include <cstdint>
#include <iterator>
#include <vector>

namespace duck {

class tree_topology_view {
	/* Allows to move around a tree topology through node/branch indexes.
	 *
	 * These indexes abstract away what the underlying data structure uses.
	 * They are defined as a std::intptr_t.
	 * This means they can store any integer or pointer, which should be general enough.
	 * Indexes are represented by an *Index struct, which prevents implicit conversions (error prone).
	 */

public:
	struct node_id_t {
		std::intptr_t value;
		explicit node_id_t (std::intptr_t v) : value (v) {}
		bool operator== (const node_id_t & o) const { return value == o.value; }
		bool operator!= (const node_id_t & o) const { return value != o.value; }
	};
	struct edge_id_t {
		std::intptr_t value;
		explicit edge_id_t (std::intptr_t v) : value (v) {}
		bool operator== (const edge_id_t & o) const { return value == o.value; }
		bool operator!= (const edge_id_t & o) const { return value != o.value; }
	};

	virtual ~tree_topology_view () = default;
	virtual node_id_t invalid_node () const = 0;
	virtual edge_id_t invalid_edge () const = 0;
	virtual node_id_t root_node () const = 0;
	virtual node_id_t father_node (edge_id_t id) const = 0;
	virtual node_id_t child_node (edge_id_t id) const = 0;
	virtual edge_id_t father_edge (node_id_t id) const = 0;
	virtual std::vector<edge_id_t> child_edges (node_id_t id) const = 0;
};

class tree_topology_view_dfs_iterator {
public:
	using iterator_category = std::forward_iterator_tag;
	using value_type = typename tree_topology_view::node_id_t;
	using difference_type = std::ptrdiff_t;
	using pointer = const value_type *;
	using reference = value_type;

	tree_topology_view_dfs_iterator () = default;
	tree_topology_view_dfs_iterator (const tree_topology_view & view,
	                                 const tree_topology_view::node_id_t & id)
	    : tree_ (&view), node_ (id) {}

	// input / output
	tree_topology_view_dfs_iterator & operator++ () {
		// TODO
		return *this;
	}
	reference operator* () const { return node_; }
	pointer operator-> () const { return &node_; }
	bool operator== (const tree_topology_view_dfs_iterator & o) const { return node_ == o.node_; }
	bool operator!= (const tree_topology_view_dfs_iterator & o) const { return node_ != o.node_; }

	// forward
	tree_topology_view_dfs_iterator operator++ (int) {
		auto tmp = *this;
		++*this;
		return tmp;
	}

private:
	const tree_topology_view * tree_{nullptr};
	tree_topology_view::node_id_t node_{0};
};

class tree_topology_view_dfs_range {
public:
	tree_topology_view_dfs_range (const tree_topology_view & tree) : tree_ (tree) {}

	tree_topology_view_dfs_iterator begin () const { return {tree_, tree_.root_node ()}; }
	tree_topology_view_dfs_iterator end () const { return {tree_, tree_.invalid_node ()}; }

private:
	const tree_topology_view & tree_;
};
inline tree_topology_view_dfs_range dfs_range (const tree_topology_view & tree) {
	return {tree};
}

} // namespace duck
