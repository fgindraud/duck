#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/range_v3.h>
#include <iterator>
#include <type_traits>
#include <vector>

/* Define multiple type of containers to test with range.
 * Containers and expected properties are encoded as a struct.
 * -> how to build instances
 * -> expected types
 */

/* Integer vector test case.
 */
struct IntVector {
	// Expected types
	using Type = std::vector<int>;
	using MutableIteratorType = typename Type::iterator;
	using ConstIteratorType = typename Type::const_iterator;
	using HasEmpty = std::true_type;
	using HasSize = std::true_type;
	using SizeType = typename Type::size_type;

	// Create instances
	static Type make_empty () { return {}; }
	static Type make_0_4 () { return {0, 1, 2, 3, 4}; }
};
TYPE_TO_STRING (IntVector);

/* Basic int range, which uses ADL to call begin and end.
 */
namespace TestADL {
struct DummyIntRange {
	int end_;
};
struct DummyIntIterator {
	using iterator_category = std::forward_iterator_tag;
	using value_type = int;
	using reference = int;
	using difference_type = int;
	using pointer = void;

	int i;

	int operator* () const { return i; }
	DummyIntIterator & operator++ () { return ++i, *this; }
	bool operator== (DummyIntIterator o) const { return i == o.i; }
	bool operator!= (DummyIntIterator o) const { return i != o.i; }
};
inline DummyIntIterator begin (DummyIntRange) {
	return {0};
}
inline DummyIntIterator end (DummyIntRange r) {
	return {r.end_};
}
} // namespace TestADL
struct ADLDummyIntRange {
	using Type = TestADL::DummyIntRange;
	using ConstIteratorType = TestADL::DummyIntIterator;
	using MutableIteratorType = TestADL::DummyIntIterator;
	using HasEmpty = std::false_type;
	using HasSize = std::false_type;
	using SizeType = typename TestADL::DummyIntIterator::difference_type;

	static Type make_empty () { return {0}; }
	static Type make_0_4 () { return {5}; }
};
TYPE_TO_STRING (ADLDummyIntRange);

// List of tested type cases
using RangeTypes = doctest::Types<IntVector, ADLDummyIntRange>;

/* Template test cases.
 * Test properties like returned types, and properties on iterators.
 * Tested for const T&, T&, with empty or non empty T.
 * TODO decompose in subtests
 */
TEST_CASE_TEMPLATE ("types", Container, RangeTypes) {
	auto empty = Container::make_empty ();
	static_assert (std::is_same<decltype (empty), typename Container::Type>::value,
	               "empty Container is of unexpected type");
	auto r_0_4 = Container::make_0_4 ();
	static_assert (std::is_same<decltype (r_0_4), typename Container::Type>::value,
	               "0_4 Container is of unexpected type");
	static_assert (duck::Internal::HasEmptyMethod<decltype (empty)>::value ==
	                   Container::HasEmpty::value,
	               "HasEmptyMethod trait failed");
	static_assert (duck::Internal::HasSizeMethod<decltype (empty)>::value ==
	                   Container::HasSize::value,
	               "HasSizeMethod trait failed");
}

TEST_CASE_TEMPLATE ("test", Container, RangeTypes) {
	{
		auto range = Container::make_empty (); // Empty range
		{
			auto & mut = range; // Mutable
			auto b = duck::begin (mut);
			auto e = duck::end (mut);
			static_assert (std::is_same<decltype (b), typename Container::MutableIteratorType>::value,
			               "begin (Container &) is not Container::MutableIteratorType");
			CHECK (b == e);
			CHECK (duck::empty (mut));
			auto s = duck::size (mut);
			static_assert (std::is_same<decltype (s), typename Container::SizeType>::value,
			               "size (Container &) is not Container::SizeType");
			CHECK (s == 0);
		}
		{
			const auto & konst = range; // Const
			auto b = duck::begin (konst);
			auto e = duck::end (konst);
			static_assert (std::is_same<decltype (b), typename Container::ConstIteratorType>::value,
			               "begin (const Container &) is not Container::ConstIteratorType");
			CHECK (b == e);
			CHECK (duck::empty (konst));
			auto s = duck::size (konst);
			static_assert (std::is_same<decltype (s), typename Container::SizeType>::value,
			               "size (const Container &) is not Container::SizeType");
			CHECK (s == 0);
		}
	}
	{
		auto range = Container::make_0_4 (); // Non empty
		{
			auto & mut = range; // Mutable
			auto b = duck::begin (mut);
			auto e = duck::end (mut);
			CHECK (b != e);
			CHECK_FALSE (duck::empty (mut));
			CHECK (duck::size (mut) == 5);
		}
		{
			const auto & konst = range; // Const
			auto b = duck::begin (konst);
			auto e = duck::end (konst);
			CHECK (b != e);
			CHECK_FALSE (duck::empty (konst));
			CHECK (duck::size (konst) == 5);
		}
	}
}

#if 0
TEST_CASE ("integer iterator") {
	auto it = duck::Range::IntegerIterator<int>{42};
	CHECK (*it == 42);
	CHECK (it == it);
	auto it2 = it - 2;
	CHECK (it - it2 == 2);
	CHECK (it2 < it);
	it++;
	CHECK (*it == 43);
}

TEST_CASE ("integer range & range basic primitives") {
	auto r = duck::range (4, 10);
	CHECK (*r.begin () == 4);
	CHECK (r.front () == 4);
	CHECK (*r.end () == 10); // Not UB as IntegerIterator is friendly :)
	CHECK (r.back () == 9);
	CHECK (r[2] == 6);
	CHECK_FALSE (r.empty ());
	CHECK (r.size () == 6);
	CHECK (*r.at (1) == 5);
	CHECK (*r.at (-3) == 7);

	auto r2 = duck::range (0);
	CHECK (r2.empty ());
	CHECK (r2.size () == 0);

	CHECK (r2 == r2);
	CHECK_FALSE (r == r2);
	CHECK (r == r);
}

TEST_CASE ("container ref range") {
	std::vector<int> vec{0, 1, 2, 3, 4};
	auto vec_r = duck::range (vec);
	CHECK_FALSE (vec_r.empty ());
	CHECK (vec_r.size () == 5);
	CHECK (vec_r.begin () == vec.begin ());
	CHECK (vec_r.end () == vec.end ());
	*vec_r.begin () = 42;
	CHECK (vec[0] == 42);
	CHECK (std::equal (vec.begin (), vec.end (), vec_r.begin ()));
	CHECK (vec_r.to_container<std::vector<int>> () == vec);
}

TEST_CASE ("container value range") {
	auto r = duck::range ({1, 2, 3, 4});
	CHECK_FALSE (r.empty ());
	CHECK (r.size () == 4);
	CHECK (*r.begin () == 1);
}

TEST_CASE ("strings & arrays") {
	auto & literal = "hello world";

	auto s = std::string{literal};

	// String literal is just an array for range
	auto r_lit = duck::range (literal);
	CHECK (r_lit.size () == s.size () + 1); // Includes '\0'

	// Use char_range to consider it as null_terminated
	auto cr_lit = duck::char_range (literal);
	CHECK (cr_lit.size () == s.size ());
}
#endif
