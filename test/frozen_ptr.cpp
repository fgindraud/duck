#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/frozen_ptr.h>

struct Base {
	virtual ~Base () = default;
};
struct Derived : Base {};

TEST_CASE ("FreezableUniquePtr") {
	auto empty = duck::FreezableUniquePtr<int>{};
	CHECK (!empty);

	auto p = duck::make_freezable_unique<int> (42);
	CHECK (p);
	CHECK (*p == 42);
	*p = 3;
	CHECK (*p == 3);

	auto p2 = std::move (p);
	CHECK (p2);
	CHECK (!p);
	CHECK (*p2 == 3);

	auto derived = duck::make_freezable_unique<Derived> ();
	duck::FreezableUniquePtr<Base> base{std::move (derived)};
}

TEST_CASE ("FrozenSharedPtr") {
	auto p = duck::make_freezable_unique<int>(44);
	auto sp = std::move (p).freeze();
	CHECK (!p);
	CHECK (sp);
	CHECK (*sp == 44);
}
