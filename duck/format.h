#pragma once

// Formatter objects for strings.

// -> Use formatter elements directly to override formatting
// + placeholders : gen a function (printf like with the right arguments)
// + dynamicity : polymorphic formatter class
// left/right padding (formatter, len, char)

// TODO longterm: optimize (remove nulls ?)

#include <algorithm>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

namespace duck {
namespace Format {
	/* Formatter base classes.
	 *
	 * "Tag" is a type tag used to recognize a formatter class.
	 * All formatter classes must inherit from it to be recognized by <<, format(), ...
	 *
	 * Base<Derived> is a CRTP class that adds a common API to all formatter classes.
	 * All formatter classes must inherit from it (struct T : Base<T>, as per CRTP).
	 * Base inherit from Tag so no need to explicitely inherit from Tag.
	 *
	 * Formatters must define two functions:
	 * - std::size_t size () const: returns the size required to format the element.
	 * - template<typename It> It write (It) const:
	 *   - format the element in It output iterator
	 *   - must increment it exactly size() times
	 *   - return the increment iterator
	 */
	struct Tag {};
	template <typename T> using IsFormatter = std::is_base_of<Tag, T>;

	template <typename Derived> class Base : public Tag {
	public:
		// Generates a std::string
		std::string to_string () const {
			std::string s (self ().size (), ' ');
			self ().write (s.begin ());
			return s;
		}

	private:
		constexpr const Derived & self () const { return static_cast<const Derived &> (*this); }
	};

	/* Element formatters.
	 */

	struct Null : public Base<Null> {
		// Empty formatter
		constexpr std::size_t size () const { return 0; }
		template <typename OutputIt> constexpr OutputIt write (OutputIt it) const { return it; }
	};

	template <std::size_t N> class StaticCharArray : public Base<StaticCharArray<N>> {
		// Reference to static char array (remove end '\0')
	public:
		constexpr StaticCharArray (const char (&str)[N]) : str_ (str) {}
		constexpr std::size_t size () const { return N - 1; }
		template <typename OutputIt> constexpr OutputIt write (OutputIt it) const {
			return std::copy (std::begin (str_), std::end (str_) - 1, it);
		}

	private:
		const char (&str_)[N];
	};

	class StringRef : public Base<StringRef> {
		// Reference to std::string
	public:
		StringRef (const std::string & str) : str_ (str) {}
		std::size_t size () const { return str_.size (); }
		template <typename OutputIt> OutputIt write (OutputIt it) const {
			return std::copy (std::begin (str_), std::end (str_), it);
		}

	private:
		const std::string & str_;
	};

	class CStringRef : public Base<CStringRef> {
		// Reference to c-string (const char *)
	public:
		CStringRef (const char * str, std::size_t len) : str_ (str), len_ (len) {}
		CStringRef (const char * str) : CStringRef (str, std::strlen (str)) {}
		std::size_t size () const { return len_; }
		template <typename OutputIt> OutputIt write (OutputIt it) const {
			return std::copy_n (str_, len_, it);
		}

	private:
		const char * str_;
		std::size_t len_;
	};

	template <typename Int> class DecimalInteger : public Base<DecimalInteger<Int>> {
		// Prints an integer in decimal base
	public:
		DecimalInteger (Int i) : i_ (i) {}
		std::size_t size () const {
			Int i = i_;
			if (i == 0)
				return 1; // "0"
			std::size_t digits = 0;
			if (i < 0) {
				i = -i;
				++digits;
			}
			while (i != 0) {
				++digits;
				i /= 10;
			}
			return digits;
		}
		template <typename OutputIt> OutputIt write (OutputIt it) const {
			Int i = i_;
			if (i == 0) {
				*it = '0';
				return ++it;
			}
			if (i < 0) {
				i = -i;
				*it++ = '-';
			}
			char buf[max_digits];
			auto buf_it = std::end (buf);
			while (i != 0) {
				*--buf_it = (i % 10) + '0';
				i /= 10;
			}
			return std::copy (buf_it, std::end (buf), it);
		}

	private:
		static constexpr int max_digits = std::numeric_limits<Int>::digits10 + 1;
		Int i_;
	};

	// Pair (sequence of 2 formatters)
	template <typename Left, typename Right> class Pair : public Base<Pair<Left, Right>> {
	public:
		Pair (Left left, Right right) : left_ (left), right_ (right) {}

		constexpr std::size_t size () const { return left_.size () + right_.size (); }
		template <typename OutputIt> constexpr OutputIt write (OutputIt it) const {
			return right_.write (left_.write (it));
		}

	private:
		Left left_;
		Right right_;
	};
}
/* Format free function.
 * This function is a factory that creates formatter classes.
 * Defines lots of overloads for many cases.
 */

// If argument is already a formatter, format() is a no-op
template <typename F, typename = typename std::enable_if<Format::IsFormatter<F>::value>::type>
F format (F f) {
	return f;
}

// Base case, generates empty format (allow "format () << stuff")
inline constexpr Format::Null format () {
	return {};
}

// Overloads for single elements
template <std::size_t N> constexpr Format::StaticCharArray<N> format (const char (&str)[N]) {
	return {str};
}
inline Format::StringRef format (const std::string & str) {
	return {str};
}
inline Format::CStringRef format (const char * str) {
	return {str};
}
template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
Format::DecimalInteger<T> format (T t) {
	return {t};
}

/* Sequencing of formatter.
 *
 * First, define format(a, b).
 * Takes anything but applies format() to each element before passing them to Pair{}.
 * This ensures formatters are generated (if already formatter, bypass).
 *
 * Also define operator<<, restricted to a formatter on the left.
 * Put in namespace Format to let ADL find it (required !).
 *
 * TODO format(<more than 2 args, variadic>) ?
 */
template <typename Left, typename Right>
constexpr auto format (Left && left, Right && right)
    -> Format::Pair<decltype (format (std::forward<Left> (left))),
                    decltype (format (std::forward<Right> (right)))> {
	return {format (std::forward<Left> (left)), format (std::forward<Right> (right))};
}
namespace Format {
	template <typename F, typename T, typename = typename std::enable_if<IsFormatter<F>::value>::type>
	auto operator<< (F && f, T && t) -> decltype (format (std::forward<F> (f), std::forward<T> (t))) {
		return format (std::forward<F> (f), std::forward<T> (t));
	}
}
}
