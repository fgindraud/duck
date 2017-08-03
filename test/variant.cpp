#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/variant.h>
#include <iostream>
#include <string>

#include <duck/debug.h>
struct blah : duck::Noisy<'a'> {};
struct bloh : duck::Noisy<'o'> {};

TEST_CASE ("test") {
	using MyVariant = duck::Variant::Static<bool, int, blah, bloh>;
	CHECK (MyVariant::index_for_type<bool> () == 0);
	CHECK (MyVariant::index_for_type<int> () == 1);
	CHECK (MyVariant::index_for_type<blah> () == 2);

	MyVariant a{blah{}};
	MyVariant b;
	b = a;
	a = std::move (b);
}

TEST_CASE ("dynamic") {
	// WIP
	using MyVariant = duck::Variant::Dynamic<sizeof (long), alignof (long)>;
	MyVariant z{42};
}
