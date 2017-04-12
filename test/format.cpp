#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/format.h>
#include <limits>
#include <string>
#include <type_traits>

namespace MyNamespace {
struct blah {};
auto format (blah) {
	return duck::format ("blah");
}
}

TEST_CASE ("basic operations") {
	// Basic string substitution
	std::string blah ("blah");
	const char * space = " ";
	auto blah_hello = duck::format () << blah << space << "hello";
	auto blah_hello_str = blah_hello.to_string ();
	using BlahHelloStrIsString = std::is_same<decltype (blah_hello_str), std::string>;
	CHECK (BlahHelloStrIsString::value);
	CHECK (blah_hello_str == "blah hello");

	// Decimal ints
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

	// Check user defined
	CHECK ((duck::format () << MyNamespace::blah{}).to_string () == "blah");
}

TEST_CASE ("placeholders system") {
	// Single placeholder
	auto single_placeholder = duck::placeholder;
	CHECK (single_placeholder.nb_placeholder () == 1);
	auto hello_instance = single_placeholder ("hello");
	CHECK (hello_instance.nb_placeholder () == 0);
	CHECK (hello_instance.to_string () == "hello");
	auto blah_instance = single_placeholder (MyNamespace::blah{});
	CHECK (blah_instance.nb_placeholder () == 0);
	CHECK (blah_instance.to_string () == "blah");
	auto placeholder_instance = single_placeholder (duck::placeholder);
	CHECK (placeholder_instance.nb_placeholder () == 1);

	// Multi placeholder
	auto formatter = duck::format () << "Je m'appelle " << duck::placeholder << " et j'ai "
	                                 << duck::placeholder << " ans !\n";
	CHECK (formatter.nb_placeholder () == 2);
}
