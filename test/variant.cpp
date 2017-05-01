#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/variant.h>
#include <string>
#include <iostream>

struct blah {};

TEST_CASE ("test") {
	using MyVariant = duck::Variant::StaticList<bool, int, blah>;
	CHECK (MyVariant::id_for_type<bool>() == 0);
	CHECK (MyVariant::id_for_type<int>() == 1);
	CHECK (MyVariant::id_for_type<blah>() == 2);

	MyVariant a {3};
	int b = 42;
	MyVariant c {b};
	const int d = 33;
	MyVariant e {d};
}
