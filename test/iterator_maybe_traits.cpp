#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/iterator/maybe_traits.h>
#include <type_traits>

/* --------------------------------- Iterator typedefs ----------------------------- */

struct GoodIterator {
	using value_type = int;
	using reference = value_type &;
	using pointer = value_type *;
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::random_access_iterator_tag;
};
struct BadIterator {};

// SFINAE context test
template <typename T, typename = duck::Iterator::GetReferenceType<T>> static bool test_has_ref (T) {
	return true;
}
static bool test_has_ref (...) {
	return false;
}

TEST_CASE ("iterator typedefs traits") {
	using namespace duck::Iterator;

	// Testing the typedef presence.
	CHECK (MaybeValueType<GoodIterator>::value);
	CHECK_FALSE (MaybeValueType<BadIterator>::value);
	CHECK_FALSE (MaybeValueType<int>::value);

	CHECK (MaybeDifferenceType<GoodIterator>::value);
	CHECK_FALSE (MaybeDifferenceType<BadIterator>::value);
	CHECK_FALSE (MaybeDifferenceType<int>::value);

	CHECK (MaybeReferenceType<GoodIterator>::value);
	CHECK_FALSE (MaybeReferenceType<BadIterator>::value);
	CHECK_FALSE (MaybeReferenceType<int>::value);

	CHECK (MaybePointerType<GoodIterator>::value);
	CHECK_FALSE (MaybePointerType<BadIterator>::value);
	CHECK_FALSE (MaybePointerType<int>::value);

	// Testing the SFINAE capability (reference only)
	CHECK (test_has_ref (GoodIterator{}));
	CHECK_FALSE (test_has_ref (BadIterator{}));
}

/* ------------------------------ Container iterator type -------------------------- */

struct DummyIterator {};
struct DummyConstIterator {};

struct ContainerWithBeginMethod {
	// A container with a begin method
	DummyIterator begin () { return {}; }
	DummyConstIterator begin () const { return {}; }
};

namespace MyNamespace {
struct ContainerWithBeginFunc {
	// A container with externalized befgin functions
};
DummyIterator begin (ContainerWithBeginFunc &) {
	return {};
}
DummyConstIterator begin (const ContainerWithBeginFunc &) {
	return {};
}
}

// SFINAE context test
template <typename T, typename = duck::Iterator::GetContainerIteratorType<T>>
static bool test_has_it (T) {
	return true;
}
static bool test_has_it (...) {
	return false;
}

TEST_CASE ("container iterator traits") {
	using namespace duck::Iterator;

	// Iterator type (begin method)
	using IteratorOfBeginMethod_Is_DummyIterator =
	    std::is_same<GetContainerIteratorType<ContainerWithBeginMethod &>, DummyIterator>;
	CHECK (IteratorOfBeginMethod_Is_DummyIterator::value);
	using IteratorOfConstBeginMethod_Is_DummyConstIterator =
	    std::is_same<GetContainerIteratorType<const ContainerWithBeginMethod &>, DummyConstIterator>;
	CHECK (IteratorOfConstBeginMethod_Is_DummyConstIterator::value);

	// Iterator type (begin function)
	using IteratorOfBeginFunc_Is_DummyIterator =
	    std::is_same<GetContainerIteratorType<MyNamespace::ContainerWithBeginFunc &>, DummyIterator>;
	CHECK (IteratorOfBeginFunc_Is_DummyIterator::value);
	using IteratorOfConstBeginFunc_Is_DummyConstIterator =
	    std::is_same<GetContainerIteratorType<const MyNamespace::ContainerWithBeginFunc &>,
	                 DummyConstIterator>;
	CHECK (IteratorOfConstBeginFunc_Is_DummyConstIterator::value);

	// Test SFINAE capability (global iterator only)
	CHECK (test_has_it (ContainerWithBeginMethod{}));
	CHECK (test_has_it (MyNamespace::ContainerWithBeginFunc{}));
	CHECK_FALSE (test_has_it (int{}));
}
