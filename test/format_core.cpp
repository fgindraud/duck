#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/format/core.h>
#include <iterator> // stream iterators, std::begin/end
#include <sstream>  // stringstream
#include <string>
#include <type_traits> // type checking
#include <vector>      // vector of polymorphic

// Pre c++14 std::forward is not constexpr (and duck::format too). Do not test constexpr then.
#if (__cplusplus >= 201402L)
#define CONSTEXPR_FORMAT constexpr
#else
#define CONSTEXPR_FORMAT const
#endif

template <typename F> constexpr bool is_null_formatter (const F &) {
	return std::is_same<F, duck::Format::Null>::value;
}

template <typename F> std::string to_string_by_stream (const F & f) {
	std::ostringstream os;
	os << f;
	return os.str ();
}
template <typename F> std::string to_string_by_stream_it (const F & f) {
	std::ostringstream os;
	f.write (std::ostream_iterator<char> (os));
	return os.str ();
}

TEST_CASE ("Null formatter") {
	// Test basic Null properties
	using duck::Format::Null;
	constexpr auto n = Null{};
	constexpr auto null_size = n.size ();
	CHECK (null_size == 0);
	constexpr auto null_write_result = n.write (int(42));
	CHECK (null_write_result == 42);

	// Test collapsing of Pair
	CHECK (is_null_formatter (duck::format ()));
	CHECK (is_null_formatter (duck::format (Null{}, Null{})));
	CHECK (is_null_formatter (Null{} << Null{}));
	CHECK (is_null_formatter ((Null{} << Null{}) << (Null{} << Null{})));
}

/* Define 3 dummy classes and formatters:
 * - one in duck::Format namespace, represents internal extension.
 * - one in global namespace
 * - one in its own namespace
 *
 * The goal is to check that we can add overloads, and if they are recognized.
 * This is not a runtime test, more a compile-time one (compilation error).
 * It also test if constexpr capabilities are kept by pair.
 */

// duck::Format extension (for single char)
namespace duck {
namespace Format {
	struct Char : public ElementBase<Char> {
		char c;
		explicit constexpr Char (char c) : c (c) {}
		constexpr std::size_t size () const { return 1; }
		template <typename OutputIt> OutputIt write (OutputIt it) const {
			*it = c;
			return ++it;
		}
	};
	constexpr Char format_element (char c, AdlTag) { return Char (c); }
}
}

template <class T> class ST;

TEST_CASE ("duck::Format char formatter") {
	// Basic duck::format usage
	CONSTEXPR_FORMAT auto char_f = duck::format ('a');
	using IsCharFormatter = std::is_same<decltype (char_f), const duck::Format::Char>;
	CHECK (IsCharFormatter::value);
	CHECK (char_f.size () == 1);
	char buf_char_f[1];
	CHECK (char_f.write (std::begin (buf_char_f)) == std::end (buf_char_f));
	CHECK (buf_char_f[0] == 'a');
	CHECK (char_f.to_string () == "a");
	CHECK (to_string_by_stream (char_f) == "a");
	CHECK (to_string_by_stream_it (char_f) == "a");

	// Test format bypass
	CONSTEXPR_FORMAT auto should_be_char_f = duck::format (char_f);
	using IsCharFormatter2 = std::is_same<decltype (should_be_char_f), const duck::Format::Char>;
	CHECK (IsCharFormatter2::value);
	char buf_schar_f[1];
	CHECK (should_be_char_f.write (std::begin (buf_schar_f)) == std::end (buf_schar_f));
	CHECK (buf_schar_f[0] == 'a');
	CHECK (should_be_char_f.to_string () == "a");
	CHECK (to_string_by_stream (should_be_char_f) == "a");
	CHECK (to_string_by_stream_it (should_be_char_f) == "a");

	// Testing basic pair usage
	CONSTEXPR_FORMAT auto char_pair = duck::format ('a', 'b');
	using IsPairOfCharFormatter =
	    std::is_same<decltype (char_pair),
	                 const duck::Format::Pair<duck::Format::Char, duck::Format::Char>>;
	CHECK (IsPairOfCharFormatter::value);
	CHECK (char_pair.size () == 2);
	char buf_char_pair[2];
	CHECK (char_pair.write (std::begin (buf_char_pair)) == std::end (buf_char_pair));
	CHECK (buf_char_pair[0] == 'a');
	CHECK (buf_char_pair[1] == 'b');
	CHECK (char_pair.to_string () == "ab");
	CHECK (to_string_by_stream (char_pair) == "ab");
	CHECK (to_string_by_stream_it (char_pair) == "ab");

	// Testing operator<< and Null collapsing
	CONSTEXPR_FORMAT auto char_pair_2 = duck::format () << 'p' << 'o' << duck::format ();
	using IsPairOfCharFormatter2 =
	    std::is_same<decltype (char_pair_2),
	                 const duck::Format::Pair<duck::Format::Char, duck::Format::Char>>;
	CHECK (IsPairOfCharFormatter2::value);
	CHECK (char_pair_2.size () == 2);
	char buf_char_pair_2[2];
	CHECK (char_pair_2.write (std::begin (buf_char_pair_2)) == std::end (buf_char_pair_2));
	CHECK (buf_char_pair_2[0] == 'p');
	CHECK (buf_char_pair_2[1] == 'o');
	CHECK (char_pair_2.to_string () == "po");
	CHECK (to_string_by_stream (char_pair_2) == "po");
	CHECK (to_string_by_stream_it (char_pair_2) == "po");
}

// Global namespace formatter
struct Gl {
	char c;
	explicit constexpr Gl (char c_) : c (c_) {}
};
struct GlFormatter : public duck::Format::ElementBase<GlFormatter> {
	char c;
	explicit constexpr GlFormatter (const Gl & b) : c (b.c) {}
	constexpr std::size_t size () const { return 1; }
	template <typename OutputIt> OutputIt write (OutputIt it) const {
		*it = c;
		return ++it;
	}
};
constexpr GlFormatter format_element (const Gl & gl, duck::Format::AdlTag) {
	return GlFormatter (gl);
}

TEST_CASE ("global namespace char formatter") {
	// Basic duck::format usage
	CONSTEXPR_FORMAT auto char_f = duck::format (Gl ('a'));
	using IsCharFormatter = std::is_same<decltype (char_f), const GlFormatter>;
	CHECK (IsCharFormatter::value);
	CHECK (char_f.size () == 1);
	char buf_char_f[1];
	CHECK (char_f.write (std::begin (buf_char_f)) == std::end (buf_char_f));
	CHECK (buf_char_f[0] == 'a');
	CHECK (char_f.to_string () == "a");
	CHECK (to_string_by_stream (char_f) == "a");
	CHECK (to_string_by_stream_it (char_f) == "a");

	// Test format bypass
	CONSTEXPR_FORMAT auto should_be_char_f = duck::format (char_f);
	using IsCharFormatter2 = std::is_same<decltype (should_be_char_f), const GlFormatter>;
	CHECK (IsCharFormatter2::value);
	char buf_schar_f[1];
	CHECK (should_be_char_f.write (std::begin (buf_schar_f)) == std::end (buf_schar_f));
	CHECK (buf_schar_f[0] == 'a');
	CHECK (should_be_char_f.to_string () == "a");
	CHECK (to_string_by_stream (should_be_char_f) == "a");
	CHECK (to_string_by_stream_it (should_be_char_f) == "a");

	// Testing basic pair usage
	CONSTEXPR_FORMAT auto char_pair = duck::format (Gl ('a'), Gl ('b'));
	using IsPairOfCharFormatter =
	    std::is_same<decltype (char_pair), const duck::Format::Pair<GlFormatter, GlFormatter>>;
	CHECK (IsPairOfCharFormatter::value);
	CHECK (char_pair.size () == 2);
	char buf_char_pair[2];
	CHECK (char_pair.write (std::begin (buf_char_pair)) == std::end (buf_char_pair));
	CHECK (buf_char_pair[0] == 'a');
	CHECK (buf_char_pair[1] == 'b');
	CHECK (char_pair.to_string () == "ab");
	CHECK (to_string_by_stream (char_pair) == "ab");
	CHECK (to_string_by_stream_it (char_pair) == "ab");

	// Testing operator<< and Null collapsing
	CONSTEXPR_FORMAT auto char_pair_2 = duck::format () << Gl ('p') << Gl ('o') << duck::format ();
	using IsPairOfCharFormatter2 =
	    std::is_same<decltype (char_pair_2), const duck::Format::Pair<GlFormatter, GlFormatter>>;
	CHECK (IsPairOfCharFormatter2::value);
	CHECK (char_pair_2.size () == 2);
	char buf_char_pair_2[2];
	CHECK (char_pair_2.write (std::begin (buf_char_pair_2)) == std::end (buf_char_pair_2));
	CHECK (buf_char_pair_2[0] == 'p');
	CHECK (buf_char_pair_2[1] == 'o');
	CHECK (char_pair_2.to_string () == "po");
	CHECK (to_string_by_stream (char_pair_2) == "po");
	CHECK (to_string_by_stream_it (char_pair_2) == "po");
}

// Extension in a namespace
namespace NS {
struct Blah {
	char c;
	constexpr Blah (char c_) : c (c_) {}
};
struct BlahFormatter : public duck::Format::ElementBase<BlahFormatter> {
	char c;
	constexpr BlahFormatter (const Blah & b) : c (b.c) {}
	constexpr std::size_t size () const { return 1; }
	template <typename OutputIt> OutputIt write (OutputIt it) const {
		*it = c;
		return ++it;
	}
};
constexpr BlahFormatter format_element (const Blah & b, duck::Format::AdlTag) {
	return BlahFormatter (b);
}
}

TEST_CASE ("inside namespace char formatter") {
	// Basic duck::format usage
	CONSTEXPR_FORMAT auto char_f = duck::format (NS::Blah ('a'));
	using IsCharFormatter = std::is_same<decltype (char_f), const NS::BlahFormatter>;
	CHECK (IsCharFormatter::value);
	CHECK (char_f.size () == 1);
	char buf_char_f[1];
	CHECK (char_f.write (std::begin (buf_char_f)) == std::end (buf_char_f));
	CHECK (buf_char_f[0] == 'a');
	CHECK (char_f.to_string () == "a");
	CHECK (to_string_by_stream (char_f) == "a");
	CHECK (to_string_by_stream_it (char_f) == "a");

	// Test format bypass
	CONSTEXPR_FORMAT auto should_be_char_f = duck::format (char_f);
	using IsCharFormatter2 = std::is_same<decltype (should_be_char_f), const NS::BlahFormatter>;
	CHECK (IsCharFormatter2::value);
	char buf_schar_f[1];
	CHECK (should_be_char_f.write (std::begin (buf_schar_f)) == std::end (buf_schar_f));
	CHECK (buf_schar_f[0] == 'a');
	CHECK (should_be_char_f.to_string () == "a");
	CHECK (to_string_by_stream (should_be_char_f) == "a");
	CHECK (to_string_by_stream_it (should_be_char_f) == "a");

	// Testing basic pair usage
	CONSTEXPR_FORMAT auto char_pair = duck::format (NS::Blah ('a'), NS::Blah ('b'));
	using IsPairOfCharFormatter =
	    std::is_same<decltype (char_pair),
	                 const duck::Format::Pair<NS::BlahFormatter, NS::BlahFormatter>>;
	CHECK (IsPairOfCharFormatter::value);
	CHECK (char_pair.size () == 2);
	char buf_char_pair[2];
	CHECK (char_pair.write (std::begin (buf_char_pair)) == std::end (buf_char_pair));
	CHECK (buf_char_pair[0] == 'a');
	CHECK (buf_char_pair[1] == 'b');
	CHECK (char_pair.to_string () == "ab");
	CHECK (to_string_by_stream (char_pair) == "ab");
	CHECK (to_string_by_stream_it (char_pair) == "ab");

	// Testing operator<< and Null collapsing
	CONSTEXPR_FORMAT auto char_pair_2 = duck::format () << NS::Blah ('p') << NS::Blah ('o')
	                                                    << duck::format ();
	using IsPairOfCharFormatter2 =
	    std::is_same<decltype (char_pair_2),
	                 const duck::Format::Pair<NS::BlahFormatter, NS::BlahFormatter>>;
	CHECK (IsPairOfCharFormatter2::value);
	CHECK (char_pair_2.size () == 2);
	char buf_char_pair_2[2];
	CHECK (char_pair_2.write (std::begin (buf_char_pair_2)) == std::end (buf_char_pair_2));
	CHECK (buf_char_pair_2[0] == 'p');
	CHECK (buf_char_pair_2[1] == 'o');
	CHECK (char_pair_2.to_string () == "po");
	CHECK (to_string_by_stream (char_pair_2) == "po");
	CHECK (to_string_by_stream_it (char_pair_2) == "po");
}

TEST_CASE ("combinations of multiple custom formatter definitions") {
	auto f = duck::format () << 'b' << duck::format (Gl ('l'), NS::Blah ('a')) << 'h';
	CHECK (f.to_string () == "blah");
}

/* Test of placeholder system.
 * FIXME currently disabled as it is not stabilised yet.
 */
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

TEST_CASE ("polymorphic formatters") {
	// Construction
	std::vector<duck::Format::Dynamic> formatters;
	formatters.emplace_back (duck::format () << 'n' << Gl ('o') << NS::Blah ('0'));
	formatters.emplace_back (duck::format () << NS::Blah ('n') << duck::format ('o', '1'));

	// Polymorphic access
	CHECK (formatters[0].to_string () == "no0");
	CHECK (to_string_by_stream (formatters[0]) == "no0");
	CHECK (formatters[1].to_string () == "no1");
	CHECK (to_string_by_stream (formatters[1]) == "no1");

	// Assignment
	// FIXME fails to recurse lookup decltype(format(...)) in format def.
	// formatters[0] = duck::Format::Dynamic (duck::format ('o', 'o', 'p'));
	//	CHECK (formatters[0].to_string () == "oops");
}
