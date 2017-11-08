#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/combinator.h>
#include <duck/range/range.h>
#include <list>
#include <vector>

#include <iostream>
#include <iterator>

// FIXME
template <typename Derived>
static std::ostream & operator<< (std::ostream & os, const duck::Range::Base<Derived> & r) {
	os << "Range(";
	for (const auto & v : r)
		os << v << ",";
	return os << ")";
}

TEST_CASE ("counted range") {
	auto r = duck::range ({0, 1, 2, 3, 4});
	auto c_r = duck::Range::counted<int>(r);
	for (auto & iv : c_r) {
		CHECK (iv.index == iv.value ());
	}
}

#if 0
TEST_CASE ("filled vector") {
	std::vector<int> v = {1, 2, 3, 4};
	auto r = duck::range (v);

	// Reversed
	auto reversed = r.reverse ();
	CHECK (reversed.size () == v.size ());
	CHECK (std::equal (reversed.begin (), reversed.end (), v.rbegin ()));

	// Filter
	auto filtered = r.filter ([](int i) { return i % 2 == 0; });
	CHECK (sizeof (filtered) == sizeof (r)); // Should be the same size
	CHECK (filtered.size () == r.size () / 2);
	std::vector<int> filtered_result = {2, 4};
	CHECK (filtered.to_container<std::vector<int>> () == filtered_result);

	// Chained filter
	auto chained_filter = r.filter ([](int i) { return i < 2; }).filter ([](int i) { return i > 0; });
	CHECK (sizeof (chained_filter) == sizeof (r));
	CHECK (chained_filter.size () == 1);
	CHECK (chained_filter.front () == 1);
}
#endif
