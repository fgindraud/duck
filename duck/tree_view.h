#pragma once

// Tree topology view
// STATUS: WIP

#include <cassert>
#include <cstdint>
#include <duck/range/algorithm.h>
#include <iterator>
#include <vector>

namespace duck {

/* Implementation independent indexes.
 * These indexes abstract away what the underlying data structure uses.
 * They are defined as a std::intptr_t.
 * This means they can store any integer or pointer, which should be general enough.
 * Indexes are represented by a struct, which prevents implicit conversions (error prone).
 */
struct topology_node_id {
	std::intptr_t value;
	explicit topology_node_id (std::intptr_t v) : value (v) {}
	bool operator== (const topology_node_id & o) const { return value == o.value; }
	bool operator!= (const topology_node_id & o) const { return value != o.value; }
};
struct topology_edge_id {
	std::intptr_t value;
	explicit topology_edge_id (std::intptr_t v) : value (v) {}
	bool operator== (const topology_edge_id & o) const { return value == o.value; }
	bool operator!= (const topology_edge_id & o) const { return value != o.value; }
};

/* Tree like structure, only for consultation.
 */
class downward_tree_topology {
	// Can navigate from root to leaves
public:
	virtual ~downward_tree_topology () = default;
	virtual topology_node_id invalid_node () const = 0;
	virtual topology_edge_id invalid_edge () const = 0;
	virtual topology_node_id root_node () const = 0;
	virtual topology_node_id child_node (topology_edge_id id) const = 0;
	virtual std::vector<topology_edge_id> child_edges (topology_node_id id) const = 0;
};
class bidirectional_tree_topology : public downward_tree_topology {
	// Can navigate up too
public:
	virtual topology_node_id father_node (topology_edge_id id) const = 0;
	virtual topology_edge_id father_edge (topology_node_id id) const = 0;
};

/* Fixed state DFS walk of the tree.
 * Does not use a stack to store nodes to be visited later.
 * Only relies on local rules.
 * Lower performance, as it needs to query the tree structure often.
 * Provides a forward iterator (can resume from anywhere).
 */
class forward_tree_dfs_range {
public:
	class iterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = topology_node_id;
		using difference_type = std::ptrdiff_t;
		using pointer = const value_type *;
		using reference = value_type;

		iterator () = default;
		iterator (const bidirectional_tree_topology & tree, const topology_node_id & id)
		    : tree_ (&tree), node_ (id) {}

		// input / output
		iterator & operator++ () {
			auto child_edges = tree_->child_edges (node_);
			if (!child_edges.empty ()) {
				// Go to first child
				node_ = tree_->child_node (child_edges.front ());
			} else {
				// Go to "next sibling"
				auto invalid_edge = tree_->invalid_edge ();
				auto invalid_node = tree_->invalid_node ();
				while (true) {
					// First go up to father ; stop if we reach root (invalid_sth)
					auto edge = tree_->father_edge (node_);
					if (edge == invalid_edge) {
						node_ = invalid_node;
						break;
					}
					node_ = tree_->father_node (edge);
					if (node_ == invalid_node) {
						break;
					}
					// Go to next sibling. If no next sibling, just loop again.
					auto father_child_edges = tree_->child_edges (node_);
					auto previous_child_position = duck::find (father_child_edges, edge);
					assert (previous_child_position != father_child_edges.end ()); // should be found
					auto next_sibling_edge = ++previous_child_position;
					if (next_sibling_edge != father_child_edges.end ()) {
						node_ = tree_->child_node (*next_sibling_edge);
						break;
					}
				}
			}
			return *this;
		}
		reference operator* () const { return node_; }
		pointer operator-> () const { return &node_; }
		bool operator== (const iterator & o) const { return node_ == o.node_; }
		bool operator!= (const iterator & o) const { return node_ != o.node_; }

		// forward
		iterator operator++ (int) {
			auto tmp = *this;
			++*this;
			return tmp;
		}

	private:
		const bidirectional_tree_topology * tree_{nullptr};
		topology_node_id node_{0};
	};

	forward_tree_dfs_range (const bidirectional_tree_topology & tree) : tree_ (tree) {}

	iterator begin () const { return {tree_, tree_.root_node ()}; }
	iterator end () const { return {tree_, tree_.invalid_node ()}; }

private:
	const bidirectional_tree_topology & tree_;
};
inline forward_tree_dfs_range dfs_range (const bidirectional_tree_topology & tree) {
	return {tree};
}

// TODO add input_tree_dfs_range, with stack
// TODO input_bfs with queue

} // namespace duck
