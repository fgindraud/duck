#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <algorithm>
#include <duck/range/combinator.h>
#include <duck/range/range.h>
#include <list>
#include <vector>

// FIXME
#include <iostream>
template <typename Derived>
static std::ostream & operator<< (std::ostream & os, const duck::Range::Base<Derived> & r) {
	os << "Range(";
	for (const auto & v : r.derived ())
		os << v << ",";
	return os << ")";
}

TEST_CASE ("tests") {
	std::vector<int> vec{0, 1, 2, 3, 4};
	auto r = duck::range (vec);

	auto reversed = duck::Range::reversed (r);
	CHECK (reversed.size () == r.size ());
	CHECK (std::equal (reversed.begin (), reversed.end (), vec.rbegin ()));

	auto c_r = duck::Range::counted<int> (r);
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
