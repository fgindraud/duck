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

TYPE_TO_STRING (std::vector<int>);
TYPE_TO_STRING (std::list<int>);
using TestedContainerTypes = doctest::Types<std::vector<int>, std::list<int>>;

TEST_CASE_TEMPLATE ("reverse, counted, filter, apply", Container, TestedContainerTypes) {
	using namespace duck::Range::Combinator;
	Container vec{0, 1, 2, 3, 4};
	auto r = duck::range (vec);

	auto reversed_range = r | reversed ();
	CHECK (reversed_range.size () == r.size ());
	CHECK (reversed_range == duck::range (vec.rbegin (), vec.rend ()));

	auto counted_range = r | counted<int> ();
	for (auto & iv : counted_range) {
		CHECK (iv.index == iv.value ());
	}

	auto filtered_range = r | filter ([](int i) { return i % 2 == 0; });
	CHECK (filtered_range.size () == 3);
	CHECK (filtered_range == duck::range ({0, 2, 4}));
	auto chained_filtered_range =
	    r | filter ([](int i) { return i < 2; }) | filter ([](int i) { return i > 0; });
	CHECK (chained_filtered_range.size () == 1);
	CHECK (chained_filtered_range.front () == 1);

	auto applied_range =
	    r | apply ([](int i) { return i - 2; }) | filter ([](int i) { return i >= 0; });
	CHECK (applied_range.size () == 3);
	CHECK (applied_range == duck::range (0, 3));
}
