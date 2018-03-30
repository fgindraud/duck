#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <duck/dynamic_struct.h>

//struct S : duck::Struct<int, int> {};

TEST_CASE ("Blah") {
	CHECK (duck::TypeInfo<int>::size () == sizeof (int));
}
