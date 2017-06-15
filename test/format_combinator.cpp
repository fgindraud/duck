#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/format/combinator.h>
#include <duck/format/basic.h>

TEST_CASE ("repeated") {
	auto f = duck::Format::repeated(duck::format ('#'), 5);
	CHECK (f.to_string () == "#####");
}
