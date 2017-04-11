#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/format.h>
#include <iostream>

TEST_CASE ("test") {
	std::string blah ("blah");
	auto formatter = duck::format () << blah << " "
	                                 << "hello";
	CHECK (formatter.to_string () == "blah hello");

	auto int_format = duck::format (-42);
	CHECK (int_format.size () == 3);
	CHECK (int_format.to_string () == "-42");
}
