#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/iterator/traits.h>
#include <type_traits>

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

	// Traits with begin method
	CHECK (ContainerHasStdIterator<ContainerWithBeginMethod>::value);
	CHECK (ContainerHasStdIterator<const ContainerWithBeginMethod>::value);
	CHECK_FALSE (ContainerHasUserIterator<ContainerWithBeginMethod>::value);
	CHECK_FALSE (ContainerHasUserIterator<const ContainerWithBeginMethod>::value);
	CHECK (ContainerHasIterator<ContainerWithBeginMethod>::value);
	CHECK (ContainerHasIterator<const ContainerWithBeginMethod>::value);

	// Traits with begin function
	CHECK_FALSE (ContainerHasStdIterator<MyNamespace::ContainerWithBeginFunc>::value);
	CHECK_FALSE (ContainerHasStdIterator<const MyNamespace::ContainerWithBeginFunc>::value);
	CHECK (ContainerHasUserIterator<MyNamespace::ContainerWithBeginFunc>::value);
	CHECK (ContainerHasUserIterator<const MyNamespace::ContainerWithBeginFunc>::value);
	CHECK (ContainerHasIterator<MyNamespace::ContainerWithBeginFunc>::value);
	CHECK (ContainerHasIterator<const MyNamespace::ContainerWithBeginFunc>::value);

	// Iterator type (begin method)
	using IteratorOfBeginMethod_Is_DummyIterator =
	    std::is_same<GetContainerIteratorType<ContainerWithBeginMethod>, DummyIterator>;
	CHECK (IteratorOfBeginMethod_Is_DummyIterator::value);
	using IteratorOfConstBeginMethod_Is_DummyConstIterator =
	    std::is_same<GetContainerIteratorType<const ContainerWithBeginMethod>, DummyConstIterator>;
	CHECK (IteratorOfConstBeginMethod_Is_DummyConstIterator::value);

	// Iterator type (begin function)
	using IteratorOfBeginFunc_Is_DummyIterator =
	    std::is_same<GetContainerIteratorType<MyNamespace::ContainerWithBeginFunc>, DummyIterator>;
	CHECK (IteratorOfBeginFunc_Is_DummyIterator::value);
	using IteratorOfConstBeginFunc_Is_DummyConstIterator =
	    std::is_same<GetContainerIteratorType<const MyNamespace::ContainerWithBeginFunc>,
	                 DummyConstIterator>;
	CHECK (IteratorOfConstBeginFunc_Is_DummyConstIterator::value);

	// Fails on wrong input
	CHECK_FALSE (ContainerHasIterator<int>::value);
	CHECK_FALSE (ContainerHasIterator<float>::value);
	CHECK_FALSE (ContainerHasIterator<DummyIterator>::value);

	// Test SFINAE capability (global iterator only)
	CHECK (test_has_it (ContainerWithBeginMethod{}));
	CHECK (test_has_it (MyNamespace::ContainerWithBeginFunc{}));
	CHECK_FALSE (test_has_it (int{}));
}

struct GoodIterator {
	using value_type = int;
	using reference = value_type &;
	using pointer = value_type *;
	using difference_type = std::ptrdiff_t;
	using iterator_category = std::random_access_iterator_tag;
};
using BadIterator = DummyIterator;

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
	CHECK (HasValueType<GoodIterator>::value);
	CHECK_FALSE (HasValueType<BadIterator>::value);
	CHECK_FALSE (HasValueType<int>::value);

	CHECK (HasDifferenceType<GoodIterator>::value);
	CHECK_FALSE (HasDifferenceType<BadIterator>::value);
	CHECK_FALSE (HasDifferenceType<int>::value);

	CHECK (HasReferenceType<GoodIterator>::value);
	CHECK_FALSE (HasReferenceType<BadIterator>::value);
	CHECK_FALSE (HasReferenceType<int>::value);

	CHECK (HasPointerType<GoodIterator>::value);
	CHECK_FALSE (HasPointerType<BadIterator>::value);
	CHECK_FALSE (HasPointerType<int>::value);

	// Testing the SFINAE capability (reference only)
	CHECK (test_has_ref (GoodIterator{}));
	CHECK_FALSE (test_has_ref (BadIterator{}));
}
