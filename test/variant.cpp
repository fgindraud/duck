#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/variant.h>
#include <iostream>
#include <string>

struct blah {};

TEST_CASE ("test") {
	using namespace duck::Variant;
	CHECK (Detail::max (1) == 1);
	CHECK (Detail::max (1, 2) == 2);
	CHECK (Detail::max (1, 2, 3) == 3);
	CHECK (Detail::max (1, 2, 3, 4) == 4);
	CHECK (Detail::max (4, 3, 2, 1) == 4);
	CHECK (Detail::max (1, 4, 2, 4) == 4);
	CHECK (Detail::max (4, 4) == 4);

	using MyVariant = duck::Variant::StaticList<bool, int, blah>;
	CHECK (MyVariant::id_for_type<bool> () == 0);
	CHECK (MyVariant::id_for_type<int> () == 1);
	CHECK (MyVariant::id_for_type<blah> () == 2);

	MyVariant a{3};
	int b = 42;
	MyVariant c{b};
	const int d = 33;
	MyVariant e{d};

	using MyVariant2 = duck::Variant::Dynamic<sizeof (long), alignof (long)>;
	MyVariant2 z{blah{}};
}
