#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/format/basic.h>
#include <limits> // test with limits<T>::max for integers
#include <string>
#include <type_traits> // type checking

template <typename T, typename F> constexpr bool is_type (const F &) {
	return std::is_same<T, F>::value;
}

TEST_CASE ("char") {
	auto f = duck::format ('a');
	CHECK (is_type<duck::Format::SingleChar> (f));
	CHECK (f.size () == 1);
	CHECK (f.to_string () == "a");
}

TEST_CASE ("char array") {
	auto f = duck::format ("blah");
	CHECK (is_type<duck::Format::StaticCharArray<5>> (f));
	CHECK (f.size () == 4);
	CHECK (f.to_string () == "blah");
}

TEST_CASE ("char buf") {
	auto * s = "blah";
	auto f = duck::format (s);
	CHECK (is_type<duck::Format::CStringRef> (f));
	CHECK (f.size () == 4);
	CHECK (f.to_string () == "blah");
}

TEST_CASE ("std::string reference") {
	auto s = std::string ("blah");
	auto f = duck::format (s);
	CHECK (is_type<duck::Format::StringRef> (f));
	CHECK (f.size () == 4);
	CHECK (f.to_string () == "blah");
}

TEST_CASE ("bool") {
	auto t = duck::format (true);
	CHECK (is_type<duck::Format::Bool> (t));
	CHECK (t.size () == 4);
	CHECK (t.to_string () == "true");

	auto f = duck::format (false);
	CHECK (is_type<duck::Format::Bool> (f));
	CHECK (f.size () == 5);
	CHECK (f.to_string () == "false");
}

TEST_CASE ("decimal int") {
	CHECK (duck::format (0).size () == 1);
	CHECK (duck::format (0).to_string () == "0");

	CHECK (duck::format (42).size () == 2);
	CHECK (duck::format (42).to_string () == "42");

	CHECK (duck::format (-42).size () == 3);
	CHECK (duck::format (-42).to_string () == "-42");

	auto int_max = int(std::numeric_limits<int>::max ());
	CHECK (duck::format (int_max).size () == std::numeric_limits<int>::digits10 + 1);
	CHECK (duck::format (int_max).to_string () == std::to_string (int_max));

	// FIXME cannot support min as -min > max...
	auto int_min = int(std::numeric_limits<int>::min () + 1);
	CHECK (duck::format (int_min).size () == std::numeric_limits<int>::digits10 + 2);
	CHECK (duck::format (int_min).to_string () == std::to_string (int_min));
}

TEST_CASE ("std::string value") {
	auto f = duck::format (std::string ("blah"));
	CHECK (is_type<duck::Format::StringValue> (f));
	CHECK (f.size () == 4);
	CHECK (f.to_string () == "blah");
}
