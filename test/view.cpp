#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <vector>

#include <duck/view.h>

using duck::string_view;

namespace doctest {
template <typename T> struct StringMaker<std::vector<T>> {
	static String convert (const std::vector<T> & v) {
		using doctest::toString;
		doctest::String s = "Vec{";
		if (v.size () > 0) {
			s += toString (v[0]);
		}
		for (std::size_t i = 1; i < v.size (); ++i) {
			s += toString (", ") + toString (v[i]);
		}
		return s + toString ("}");
	}
};
template <> struct StringMaker<string_view> {
	static String convert (string_view sv) {
		using doctest::toString;
		return toString ("\"") + toString (duck::to_string (sv)) + toString ("\"");
	}
};
} // namespace doctest

TEST_CASE ("split") {
	using duck::split;
	CHECK (split (',', "") == std::vector<string_view>{""});
	CHECK (split (',', ",") == std::vector<string_view>{"", ""});
	CHECK (split (',', ",,") == std::vector<string_view>{"", "", ""});
	CHECK (split (',', "a,b,c") == std::vector<string_view>{"a", "b", "c"});
	CHECK (split (',', "a,b") == std::vector<string_view>{"a", "b"});
	CHECK (split (',', "a,") == std::vector<string_view>{"a", ""});
	CHECK (split (',', ",b") == std::vector<string_view>{"", "b"});
	CHECK (split (',', " ,b ") == std::vector<string_view>{" ", "b "});
}
