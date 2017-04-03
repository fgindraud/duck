#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range_combinator.h>
#include <list>
#include <vector>

#include <iostream>
#include <iterator>

#define PRINTABLE_TYPE(name)                                                                       \
	const char * _get_type_name (name) { return #name; }
PRINTABLE_TYPE (std::random_access_iterator_tag)
PRINTABLE_TYPE (std::input_iterator_tag)
PRINTABLE_TYPE (std::forward_iterator_tag)
PRINTABLE_TYPE (std::bidirectional_iterator_tag)

template <typename It> std::ostream & operator<< (std::ostream & os, duck::Range<It> r) {
	os << "Range(";
	for (const auto & v : r)
		os << v << ",";
	return os << ")";
}

TEST_CASE ("filled vector") {
	std::vector<int> v = {1, 2, 3, 4};
	auto l = duck::reverse_range (duck::range (v)).to_container<std::list<int>> ();
	CHECK (std::equal (v.begin (), v.end (), l.rbegin (), l.rend ()));

	auto filtered = duck::filter_range (duck::range (l), [](int i) { return i % 2 == 0; });
	std::cout << _get_type_name (decltype (filtered)::IteratorCategory{}) << std::endl;
	std::cout << sizeof (filtered) << std::endl;
	std::cout << filtered << std::endl;

	auto generator = duck::apply_range ([](int i) { return i + 42; }, duck::range (v));
	std::cout << _get_type_name (decltype (generator)::IteratorCategory{}) << std::endl;
	std::cout << sizeof (generator) << std::endl;
	std::cout << generator.front () << std::endl;
}
