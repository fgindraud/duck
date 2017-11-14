#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <algorithm>
#include <duck/range/range.h>
#include <list>
#include <type_traits>
#include <vector>

// Type deduction dummy types
struct DummyIteratorTraits {
	using iterator_category = std::random_access_iterator_tag;
	using value_type = void;
	using difference_type = std::ptrdiff_t;
	using reference = void;
	using pointer = void;
};
struct MyIterable {
	struct Iterator : DummyIteratorTraits {};
	struct ConstIterator : DummyIteratorTraits {};
	Iterator begin () { return {}; }
	ConstIterator begin () const { return {}; }
	Iterator end () { return {}; }
	ConstIterator end () const { return {}; }
};
struct MyContainer : MyIterable {
	using size_type = int;
	bool empty () const { return true; }
	int size () const { return 0; }
};

// Internal IteratorTypeOf
static_assert (
    std::is_same<duck::Range::IteratorTypeOf<const MyIterable &>, MyIterable::ConstIterator>::value,
    "");
static_assert (std::is_same<duck::Range::IteratorTypeOf<MyIterable &>, MyIterable::Iterator>::value,
               "");

template <typename T> using DeductedRangeType = decltype (duck::range (std::declval<T> ()));
template <typename T>
using DeductedRangeTraitIterator =
    typename duck::Range::RangeTraits<DeductedRangeType<T>>::Iterator;
template <typename T>
using DeductedRangeBeginType = decltype (std::declval<DeductedRangeType<T>> ().begin ());

// const Iter &
static_assert (std::is_same<DeductedRangeType<const MyIterable &>,
                            duck::Range::Iterable<const MyIterable &>>::value,
               "range(const MyIterable &) -> Iterable<const MyIterable &>");
static_assert (
    std::is_same<DeductedRangeTraitIterator<const MyIterable &>, MyIterable::ConstIterator>::value,
    "range(const MyIterable &)::Iterator == MyIterable::ConstIterator");
static_assert (
    std::is_same<DeductedRangeBeginType<const MyIterable &>, MyIterable::ConstIterator>::value,
    "range(const MyIterable &).begin () -> MyIterable::ConstIterator");
static_assert (std::is_same<DeductedRangeType<const MyContainer &>,
                            duck::Range::Container<const MyContainer &>>::value,
               "range(const MyContainer &) -> Container<const MyContainer &>");

// Iter &
static_assert (
    std::is_same<DeductedRangeType<MyIterable &>, duck::Range::Iterable<MyIterable &>>::value,
    "range(MyIterable &) -> Iterable<MyIterable &>");
static_assert (std::is_same<DeductedRangeTraitIterator<MyIterable &>, MyIterable::Iterator>::value,
               "range(MyIterable &)::Iterator == MyIterable::Iterator");
static_assert (std::is_same<DeductedRangeBeginType<MyIterable &>, MyIterable::Iterator>::value,
               "range(MyIterable &).begin () -> MyIterable::Iterator");
static_assert (
    std::is_same<DeductedRangeType<MyContainer &>, duck::Range::Container<MyContainer &>>::value,
    "range(MyContainer &) -> Container<MyContainer &>");

// Iter &&
static_assert (
    std::is_same<DeductedRangeType<MyIterable &&>, duck::Range::Iterable<MyIterable>>::value,
    "range(MyIterable &&) -> Iterable<const MyIterable &>");
static_assert (
    std::is_same<DeductedRangeTraitIterator<MyIterable &&>, MyIterable::ConstIterator>::value,
    "range(MyIterable &&)::Iterator == MyIterable::ConstIterator");
static_assert (
    std::is_same<DeductedRangeBeginType<MyIterable &&>, MyIterable::ConstIterator>::value,
    "range(MyIterable &&).begin () -> MyIterable::ConstIterator");
static_assert (
    std::is_same<DeductedRangeType<MyContainer &&>, duck::Range::Container<MyContainer>>::value,
    "range(MyContainer &&) -> Container<const MyContainer &>");

// Int
static_assert (std::is_same<DeductedRangeType<int>,
                            duck::Range::IteratorPair<duck::Range::IntegerIterator<int>>>::value,
               "range(int) -> IteratorPair<IntegerIterator<int>>");
static_assert (
    std::is_same<DeductedRangeTraitIterator<int>, duck::Range::IntegerIterator<int>>::value,
    "range(int)::Iterator == IntegerIterator<int>");
static_assert (std::is_same<DeductedRangeBeginType<int>, duck::Range::IntegerIterator<int>>::value,
               "range(int).begin () -> IntegerIterator<int>");

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
