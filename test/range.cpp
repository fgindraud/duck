#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/range.h>
#include <type_traits>

// Type deduction dummy types
struct DummyIteratorTraits {
	using iterator_category = std::random_access_iterator_tag;
	using value_type = void;
	using difference_type = void;
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
}
