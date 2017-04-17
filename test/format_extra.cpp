#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/format/equality.h>
#include <duck/format/basic.h>
#include <string>

TEST_CASE ("equality") {
	auto s = std::string ("world!");
	auto f = duck::format () << "hello " << s;
	CHECK (f == "hello world!");
	CHECK (f == std::string ("hello world!"));
	CHECK_FALSE (f == "blah");
	CHECK_FALSE (f == "hello");
	CHECK (duck::format () == "");
}
