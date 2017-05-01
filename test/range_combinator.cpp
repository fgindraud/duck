#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/combinator.h>
#include <duck/range/range.h>
#include <list>
#include <vector>

#include <iostream>
#include <iterator>

template <typename It> std::ostream & operator<< (std::ostream & os, duck::Range::Base<It> r) {
	os << "Range(";
	for (const auto & v : r)
		os << v << ",";
	return os << ")";
}

TEST_CASE ("filled vector") {
	std::vector<int> v = {1, 2, 3, 4};
	auto rr = duck::Range::reverse (duck::range (v));
	auto l = std::list<int> (rr.begin (), rr.end ());
	CHECK (std::equal (v.begin (), v.end (), l.rbegin ()));

	auto filtered = duck::Range::filter (duck::range (l), [](int i) { return i % 2 == 0; });
	std::cout << sizeof (filtered) << std::endl;
	std::cout << filtered << std::endl;
}
