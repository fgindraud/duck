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
	using IteratorOfBeginMethodIsDummyIterator =
	    std::is_same<GetContainerIteratorType<ContainerWithBeginMethod>, DummyIterator>;
	CHECK (IteratorOfBeginMethodIsDummyIterator::value);
	using IteratorOfConstBeginMethodIsDummyConstIterator =
	    std::is_same<GetContainerIteratorType<const ContainerWithBeginMethod>, DummyConstIterator>;
	CHECK (IteratorOfConstBeginMethodIsDummyConstIterator::value);

	// Iterator type (begin function)
	using IteratorOfBeginFuncIsDummyIterator =
	    std::is_same<GetContainerIteratorType<MyNamespace::ContainerWithBeginFunc>, DummyIterator>;
	CHECK (IteratorOfBeginFuncIsDummyIterator::value);
	using IteratorOfConstBeginFuncIsDummyConstIterator =
	    std::is_same<GetContainerIteratorType<const MyNamespace::ContainerWithBeginFunc>,
	                 DummyConstIterator>;
	CHECK (IteratorOfConstBeginFuncIsDummyConstIterator::value);

	// Fails on wrong input
	CHECK_FALSE (ContainerHasIterator<int>::value);
	CHECK_FALSE (ContainerHasIterator<float>::value);
	CHECK_FALSE (ContainerHasIterator<DummyIterator>::value);
}

struct GoodIterator {
	using value_type = int;
	using reference = value_type &;
	using pointer = value_type *;
	using difference_type = std::ptrdiff_t;
};
using BadIterator = DummyIterator;

TEST_CASE ("iterator typedefs traits") {
	using namespace duck::Iterator;

	// Tests
	CHECK (HasValueType<GoodIterator>::value);
	CHECK_FALSE (HasValueType<BadIterator>::value);
	
	CHECK (HasDifferenceType<GoodIterator>::value);
	CHECK_FALSE (HasDifferenceType<BadIterator>::value);
	
	CHECK (HasReferenceType<GoodIterator>::value);
	CHECK_FALSE (HasReferenceType<BadIterator>::value);
	
	CHECK (HasPointerType<GoodIterator>::value);
	CHECK_FALSE (HasPointerType<BadIterator>::value);
}
