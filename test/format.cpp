#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/format.h>

using namespace duck::Format;

TEST_CASE ("test") {
	std::string blah ("blah");
	auto formatter = format () << blah << " " << "hello";
	auto str = to_string(formatter);
	CHECK (str == "blah hello");
}
