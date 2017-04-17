#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/format/basic.h>
#include <iterator> // wierd stream iterators
#include <limits>   // test with limits<T>::max for integers
#include <sstream>  // stringstream
#include <string>
#include <type_traits> // type checking
#include <vector>      // vector of polymorphic

namespace MyNamespace {
struct blah {};
auto format_element (blah, duck::Format::AdlTag) -> decltype (duck::format ("blah")) {
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

	// Test collapsing of Nulls
	auto should_be_just_string_ref = duck::format () << duck::format (blah, duck::format ())
	                                                 << duck::format ();
	using IsStringRef = std::is_same<decltype (should_be_just_string_ref), duck::Format::StringRef>;
	CHECK (IsStringRef::value);

	// test blah hello output with other iterators
	std::ostringstream os;
	os << blah_hello; // std::ostreambuf_iterator
	CHECK (os.str () == "blah hello");
	std::ostringstream os2;
	blah_hello.write (std::ostream_iterator<char> (os2));
	CHECK (os2.str () == "blah hello");
	std::string os3;
	blah_hello.write (std::back_inserter (os3));
	CHECK (os3 == "blah hello");

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

#if 0
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
	auto formatter = duck::format () << "var " << duck::placeholder << " = " << duck::placeholder;
	CHECK (formatter.nb_placeholder () == 2);
	auto subst = formatter ("i", 42);
	CHECK (subst.nb_placeholder () == 0);
	CHECK (subst.to_string () == "var i = 42");
}
#endif

TEST_CASE ("polymorphic formatter objects") {
	// Construction
	std::vector<duck::Format::Dynamic> formatters;
	std::string name = "Gaston";
	formatters.emplace_back (duck::format () << 3 << " + " << 4 << " = " << (3 + 4) << " !");
	formatters.emplace_back (duck::format () << "Bonjour " << name);

	// Polymorphic access
	CHECK (formatters[1].to_string () == "Bonjour Gaston");
	std::ostringstream os;
	os << formatters[0];
	CHECK (os.str () == "3 + 4 = 7 !");
}
