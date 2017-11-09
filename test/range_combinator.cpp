#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/combinator.h>
#include <duck/range/range.h>
#include <list>
#include <vector>
#include <algorithm>

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
	using namespace duck::Range::Combinator;
	std::vector<int> vec{0, 1, 2, 3, 4};
	auto r = duck::range (vec);

	auto r_r = r | reversed ();
	CHECK (r_r.size () == r.size ());
	CHECK (std::equal (r_r.begin (), r_r.end (), vec.rbegin ()));

	auto c_r = r | counted<int> ();
	for (auto & iv : c_r) {
		CHECK (iv.index == iv.value ());
	}

	auto f_r = r | filter ([](int i) { return i % 2 == 0; });
	CHECK (f_r.size () == 3);
}

#if 0
TEST_CASE ("filled vector") {
	std::vector<int> v = {1, 2, 3, 4};
	auto r = duck::range (v);

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
