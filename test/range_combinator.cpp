#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/combinator.h>
#include <forward_list>
#include <iterator>
#include <list>
#include <vector>

// Lousy wrapper of std vector which is move only : test move construction of ranges
class move_only_int_vector {
private:
	std::vector<int> v;

public:
	move_only_int_vector () = default;
	move_only_int_vector (std::initializer_list<int> l) : v (l) {}

	move_only_int_vector (const move_only_int_vector &) = delete;
	move_only_int_vector (move_only_int_vector &&) = default;
	move_only_int_vector & operator= (const move_only_int_vector &) = delete;
	move_only_int_vector & operator= (move_only_int_vector &&) = default;

	typename std::vector<int>::const_iterator begin () const { return v.begin (); }
	typename std::vector<int>::const_iterator end () const { return v.end (); }
};

// Testing combinators with multiple containers, to test different iterator categories
TYPE_TO_STRING (std::vector<int>);
TYPE_TO_STRING (move_only_int_vector);
TYPE_TO_STRING (std::list<int>);
TYPE_TO_STRING (std::forward_list<int>);

using bidir_container_types =
    doctest::Types<std::vector<int>, move_only_int_vector, std::list<int>>;
using forward_container_types =
    doctest::Types<std::vector<int>, move_only_int_vector, std::list<int>, std::forward_list<int>>;

auto values = {0, 1, 2, 3, 4};

TEST_CASE_TEMPLATE ("reverse", C, bidir_container_types) {
	auto range = C{values} | duck::reverse ();
	CHECK (duck::size (range) == duck::size (values));
	using RevIt = std::reverse_iterator<typename std::initializer_list<int>::iterator>;
	CHECK (range == duck::range (RevIt{duck::end (values)}, RevIt{duck::begin (values)}));

	CHECK (duck::empty (C{} | duck::reverse ()));
}

TEST_CASE_TEMPLATE ("index", C, forward_container_types) {
	for (auto & iv : C{values} | duck::indexed<int> ()) {
		CHECK (iv.index == iv.value ());
	}
	// Empty, also check that index<Int> has a default
	CHECK (duck::empty (C{} | duck::indexed ()));
}
#if 0

TEST_CASE_TEMPLATE ("filter", Container, forward_container_types) {
	auto filtered_range =
	    duck::range (Container{values}) | DRC::filter ([](int i) { return i % 2 == 0; });
	CHECK (duck::size (filtered_range) == 3);
	CHECK (filtered_range == duck::range ({0, 2, 4}));

	auto chained_filtered_range = duck::range (Container{values}) |
	                              DRC::filter ([](int i) { return i < 2; }) |
	                              DRC::filter ([](int i) { return i > 0; });
	CHECK (duck::size (chained_filtered_range) == 1);
	CHECK (duck::front (chained_filtered_range) == 1);

	CHECK ((duck::range (Container{}) | DRC::filter ([](int) { return true; })).empty ());
}

TEST_CASE_TEMPLATE ("apply", Container, forward_container_types) {
	auto applied_range = duck::range (Container{values}) | DRC::apply ([](int i) { return i - 2; }) |
	                     DRC::filter ([](int i) { return i >= 0; });
	CHECK (duck::size (applied_range) == 3);
	CHECK (applied_range == duck::range (0, 3));

	CHECK ((duck::range (Container{}) | DRC::apply ([](int i) { return i; })).empty ());
}
#endif
