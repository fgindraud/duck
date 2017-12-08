#pragma once

// Tree topology view
// STATUS: WIP

#include <cstdint>
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
		bool operator== (const node_id_t & o) { return value == o.value; }
		bool operator!= (const node_id_t & o) { return value != o.value; }
	};
	struct edge_id_t {
		std::intptr_t value;
		explicit edge_id_t (std::intptr_t v) : value (v) {}
		bool operator== (const edge_id_t & o) { return value == o.value; }
		bool operator!= (const edge_id_t & o) { return value != o.value; }
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

} // namespace duck
