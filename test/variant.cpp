#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/variant.h>
#include <iostream>
#include <string>

struct blah {};

TEST_CASE ("test") {
	using MyVariant = duck::Variant::Static<bool, int, blah>;
	CHECK (MyVariant::index_for_type<bool> () == 0);
	CHECK (MyVariant::index_for_type<int> () == 1);
	CHECK (MyVariant::index_for_type<blah> () == 2);

	MyVariant a{3};
	int b = 42;
	MyVariant c{b};
	const int d = 33;
	MyVariant e{d};

	using MyVariant2 = duck::Variant::Dynamic<sizeof (long), alignof (long)>;
	MyVariant2 z{blah{}};
}
