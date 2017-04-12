#pragma once

// Formatter objects for strings.

// -> Use formatter elements directly to override formatting
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
	 * A "formatter" is a class that stringify some data.
	 * It can be simple (one piece of data) or composite (think of format strings + args).
	 * A formatter can be "executed": its content is written as chars to something.
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

	protected:
		constexpr const Derived & self () const { return static_cast<const Derived &> (*this); }
	};

	/* In addition to regular formatter, a formatter can have placeholders.
	 * The PlaceHolder type represents a placeholder.
	 * It is not a valid formatter (no size/write method).
	 * Thus a formatter with placeholders cannot be executed.
	 *
	 * It must first be "instantiated" by removing the placeholders.
	 * Instantiation is done by operator()(...), which accepts as many arguments as placeholders.
	 * It returns a new formatter with placeholders substituted.
	 * PlaceHolder values can be given to instantiation for a partial instantiation.
	 * operator()(...) accepts any value that format() would accept.
	 *
	 * Inheriting from SimpleBase<Derived> adds definition to indicate a non-placeholder element.
	 * Composite elements must manually manage
	 * TODO doc nb_placeholder
	 *
	 * A PlaceHolder value named "placeholder" can be used in format() to have a short syntax.
	 * For correct lookup of operator()(...), PlaceHolder is defined after format() overloads (below).
	 */

	template <typename Derived> struct SimpleBase : Base<Derived> {
		// Placeholder support for simple element: no placeholder, no substitution.
		static constexpr std::size_t nb_placeholder () { return 0; }
		Derived operator() () const { return this->self (); }
	};

	/* Null formatter, and format() free function.
	 *
	 * A format() function with lots of overloads can be used to build formatters easily.
	 * It deduces types and avoids explicitely calling constructors.
	 * To force a specific deduction, a constructor can still be called explicitely.
	 *
	 * format() returns a Null formatter (empty string)
	 * format(T):
	 * - tries to create a formatter from T with lots of overloads
	 * - return T (bypass) if T is a formatter
	 *
	 * The null formatter is useful to start a chain of formatter like:
	 * auto formatter = format() << "blah" << 42 << ...;
	 *
	 * Format can be extended to your own types.
	 * The overload must be in the global namespace or the same namespace as the overload type.
	 * Placing it in namespace duck{} will NOT work (fails the name lookup).
	 */
	template <typename F, typename = typename std::enable_if<IsFormatter<F>::value>::type>
	F format (F f) {
		return f;
	}

	struct Null : public SimpleBase<Null> {
		constexpr std::size_t size () const { return 0; }
		template <typename OutputIt> constexpr OutputIt write (OutputIt it) const { return it; }
	};
	inline constexpr Null format () { return {}; }

	/* Simple formatters.
	 * Builds a formatter for common cases (strings, integrals, etc).
	 */
	template <std::size_t N> class StaticCharArray : public SimpleBase<StaticCharArray<N>> {
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
	template <std::size_t N> constexpr StaticCharArray<N> format (const char (&str)[N]) {
		return {str};
	}

	class StringRef : public SimpleBase<StringRef> {
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
	inline StringRef format (const std::string & str) { return {str}; }

	class CStringRef : public SimpleBase<CStringRef> {
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
	inline CStringRef format (const char * str) { return {str}; }

	template <typename Int> class DecimalInteger : public SimpleBase<DecimalInteger<Int>> {
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
	template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
	Format::DecimalInteger<T> format (T t) {
		return {t};
	}

	/* Composite formatter that represents a sequence of two formatters.
	 * Will apply Left then Right.
	 * The sub-formatters are stored by copy.
	 *
	 * We also define format(a, b), and operator<< (a, b).
	 */
	template <typename Left, typename Right> class Pair : public Base<Pair<Left, Right>> {
	public:
		Pair (Left left, Right right) : left_ (std::move (left)), right_ (std::move (right)) {}

		constexpr std::size_t size () const { return left_.size () + right_.size (); }
		template <typename OutputIt> constexpr OutputIt write (OutputIt it) const {
			return right_.write (left_.write (it));
		}

		// Non standard placeholder support, must redirect arguments
		static constexpr std::size_t nb_placeholder () {
			return Left::nb_placeholder () + Right::nb_placeholder ();
		}
		// TODO

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
	template <typename F, typename T, typename = typename std::enable_if<IsFormatter<F>::value>::type>
	auto operator<< (F && f, T && t) -> decltype (format (std::forward<F> (f), std::forward<T> (t))) {
		return format (std::forward<F> (f), std::forward<T> (t));
	}

	// TODO Polymorphic restricted formatter
	class Polymorphic : public Base<Polymorphic> {};

	// Delayed placeholder definition
	struct PlaceHolder : public Tag {
		static constexpr std::size_t nb_placeholder () { return 1; }
		template <typename F> auto operator() (F && f) -> decltype (format (std::forward<F> (f))) {
			return format (std::forward<F> (f));
		}
	};
	static constexpr PlaceHolder placeholder{};
}

using Format::format;
using Format::placeholder;
}
