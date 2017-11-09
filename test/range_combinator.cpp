#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

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
	using namespace duck::Range::Combinator;
	std::vector<int> vec{0, 1, 2, 3, 4};
	auto r = duck::range (vec);

	auto r_r = r | reversed ();
	CHECK (r_r.size () == r.size ());
	CHECK (r_r == duck::range (vec.rbegin (), vec.rend ()));

	auto c_r = r | counted<int> ();
	for (auto & iv : c_r) {
		CHECK (iv.index == iv.value ());
	}

	auto f_r = r | filter ([](int i) { return i % 2 == 0; });
	CHECK (f_r.size () == 3);
	CHECK (f_r == duck::range ({0, 2, 4}));
	auto chained_f_r =
	    r | filter ([](int i) { return i < 2; }) | filter ([](int i) { return i > 0; });
	CHECK (chained_f_r.size () == 1);
	CHECK (chained_f_r.front () == 1);

	auto a_r = r | apply ([](int i) { return i - 2; }) | filter ([](int i) { return i >= 0; });
	CHECK (a_r.size () == 3);
	CHECK (a_r == duck::range (0, 3));
}
