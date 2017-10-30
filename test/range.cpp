#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/range.h>
#include <type_traits>

TEST_CASE ("integer iterator") {
	auto it = duck::Range::IntegerIterator<int>{42};
	CHECK (*it == 42);
	CHECK (it == it);
	auto it2 = it - 2;
	CHECK (it - it2 == 2);
	CHECK (it2 < it);
}

#include <iostream>
#include <vector>
template <typename T> struct TD;

TEST_CASE ("basic tests") {
	// auto r = duck::range (42);
	std::vector<int> v{1, 2, 3, 4};
	auto r = duck::range (v);
	CHECK (r.size () == 4);
	//TD<typename decltype (r)::Iterator> a;
}
