#pragma once

// Formatter objects for strings.

// TODO have format() as a toplevel
// + CString from char*
// -> Use formatter elements directly to override formatting
// + placeholders : gen a function (printf like with the right arguments)
// + support for parsing ? maybe not, this is different after all
// + dynamicity : polymorphic formatter class
// -> better write() : use a char* ? an "output" class ?
// -> caching of size ?
// -> store as a flattened tuple ?

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <duck/range/range.h>
#include <string>
#include <type_traits>

namespace duck {
namespace Format {
	// Type tag to identify a formatter class
	struct Tag {};
	template <typename T> using IsFormatter = std::is_base_of<Tag, T>;
	template <typename Formatter,
	          typename = typename std::enable_if<IsFormatter<Formatter>::value>::type>
	Formatter format (Formatter f) {
		return f; // If t is already a formatter, format() is a no-op
	}

	// Null formatter
	struct Null : public Tag {
		constexpr std::size_t size () const { return 0; }
		template <typename RangeType> constexpr void write (RangeType) const {}
	};
	constexpr Null format () { return {}; }

	// A static char array
	template <std::size_t N> class StaticCharArray : public Tag {
	public:
		constexpr StaticCharArray (const char (&str)[N]) : str_ (str) {}
		constexpr std::size_t size () const {
			return N - 1; // Remove '\0'
		}
		template <typename RangeType> constexpr void write (RangeType r) const {
			std::copy (std::begin (str_), std::end (str_) - 1, std::begin (r));
		}

	private:
		const char (&str_)[N];
	};
	template <std::size_t N> constexpr StaticCharArray<N> format (const char (&str)[N]) {
		return {str};
	}

	// A std string object
	class StringRef : public Tag {
	public:
		StringRef (const std::string & str) : str_ (str) {}
		std::size_t size () const { return str_.size (); }
		template <typename RangeType> void write (RangeType r) const {
			std::copy (std::begin (str_), std::end (str_), std::begin (r));
		}

	private:
		const std::string & str_;
	};
	StringRef format (const std::string & str) { return {str}; }

	// Pair (sequence of 2 formatters)
	template <typename Left, typename Right> class Pair : public Tag {
	public:
		Pair (Left left, Right right) : left_ (left), right_ (right) {}

		constexpr std::size_t size () const { return left_.size () + right_.size (); }
		template <typename RangeType> constexpr void write (RangeType r) const {
			auto ls = left_.size ();
			left_.write (r.slice_to (ls));
			right_.write (r.slice_from (ls));
		}

	private:
		Left left_;
		Right right_;
	};
	template <typename Left, typename Right>
	constexpr auto format (Left && left, Right && right)
	    -> Pair<decltype (format (std::forward<Left> (left))),
	            decltype (format (std::forward<Right> (right)))> {
		return {format (std::forward<Left> (left)), format (std::forward<Right> (right))};
	}
	template <typename Formatter, typename T,
	          typename = typename std::enable_if<IsFormatter<Formatter>::value>::type>
	auto operator<< (Formatter && f, T && t) -> decltype (format (f, t)) {
		return format (f, t);
	}

	template <typename Formatter,
	          typename = typename std::enable_if<IsFormatter<Formatter>::value>::type>
	std::string to_string (Formatter formatter) {
		std::string res (formatter.size (), ' ');
		formatter.write (range (res));
		return res;
	}
}
}
