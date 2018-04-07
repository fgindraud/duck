#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <dynamic_struct.h>

// struct S : Struct<int, int> {};

TEST_CASE ("Blah") {
	CHECK (TypeInfo<int>::size () == sizeof (int));
}
