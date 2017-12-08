#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/tree_view.h>

#include <memory>
#include <vector>

struct node_t {
	std::vector<std::unique_ptr<node_t>> childrens;
};

std::unique_ptr<node_t> append (std::unique_ptr<node_t> n) {
	return n;
}
template <typename... Types>
std::unique_ptr<node_t> append (std::unique_ptr<node_t> n, std::unique_ptr<node_t> dep,
                                Types &&... other_deps) {
	n->childrens.push_back (std::move (dep));
	return append (std::move (n), std::forward<Types> (other_deps)...);
}
template <typename... Args> std::unique_ptr<node_t> N (Args &&... args) {
	return append (std::unique_ptr<node_t>{new node_t{}}, std::forward<Args> (args)...);
}

TEST_CASE ("test") {
	auto tree = N (N (), N ());
}
