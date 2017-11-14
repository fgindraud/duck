#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <algorithm>
#include <duck/old_range/range.h>
#include <vector>

TEST_CASE ("empty vector") {
	std::vector<int> empty_vect;
	auto r = duck::range (empty_vect);
	CHECK (r.empty ());
	CHECK (r.size () == 0);
	CHECK_FALSE (r.contains (empty_vect.begin ()));
	CHECK_FALSE (r.contains (empty_vect.end ()));
}

TEST_CASE ("filled vector") {
	std::vector<int> v1234 = {1, 2, 3, 4};
	auto r = duck::range (v1234);
	CHECK (r.contains (v1234.begin ()));
	CHECK (r.contains (v1234.begin () + 2));
	CHECK_FALSE (r.contains (v1234.end ()));
	// Slice
	auto slice = r.slice (1, 3);
	CHECK (std::equal (v1234.begin () + 1, v1234.begin () + 3, slice.begin ()));
	slice[1] = 3;
	CHECK (v1234[2] == 3);
	// Slice from end
	auto rslice = r.slice (1, -1);
	CHECK (std::equal (slice.begin (), slice.end (), rslice.begin ()));
}

