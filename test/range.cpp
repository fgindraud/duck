#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <algorithm>
#include <duck/range/range.h>
#include <list>
#include <string>
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
	CHECK_FALSE (r.empty ());
	CHECK (r.front () == v1234.front ());
	CHECK (r.back () == v1234.back ());
	CHECK (r.size () == v1234.size ());
	CHECK (r.contains (v1234.begin ()));
	CHECK (r.contains (v1234.begin () + 2));
	CHECK_FALSE (r.contains (v1234.end ()));
	CHECK (std::equal (v1234.begin (), v1234.end (), r.begin ()));
	CHECK (*r.at (1) == 2);
	CHECK (*r.at (-3) == 2);
	// Modify
	r[2] = 42;
	CHECK (std::equal (v1234.begin (), v1234.end (), r.begin ()));
	// Slice
	auto slice = r.slice (1, 3);
	CHECK (std::equal (v1234.begin () + 1, v1234.begin () + 3, slice.begin ()));
	slice[1] = 3;
	CHECK (v1234[2] == 3);
	// Slice from end
	auto rslice = r.slice (1, -1);
	CHECK (std::equal (slice.begin (), slice.end (), rslice.begin ()));
}

TEST_CASE ("between vector, list, string") {
	// Test exchange between list, string, and others. List has more costly and restricted api (bidir
	// only ops).
	std::string s ("hello world");
	auto v = duck::range (s).to_container<std::vector<char>> ();
	CHECK (std::equal (v.begin (), v.end (), s.begin ()));
	auto l = duck::range (v).pop_back (6).to_container<std::list<char>> ();
	auto s2 = duck::range (l).to_container<std::string> ();
	CHECK (s2 == "hello");
}

TEST_CASE ("integer range") {
	auto r = duck::range (10);
	CHECK (r.size () == 10);
	auto v = r.slice (4, -4).to_container<std::vector<int>> ();
	CHECK (v.size () == 2);
	CHECK (v[0] == 4);
	CHECK (v[1] == 5);
}
