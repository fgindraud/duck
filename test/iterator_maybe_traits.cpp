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
template <typename T, typename = duck::Maybe::Unpack<duck::Iterator::MaybeReferenceType<T>>>
static bool test_has_ref (T) {
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
