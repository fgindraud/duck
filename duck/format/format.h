#pragma once

// Formatter objects for strings.

#include <algorithm> // std::copy
#include <cstring>   // printing char* (strlen)
#include <duck/type_traits.h>
#include <iosfwd>   // std::ostream declaration
#include <iterator> // std::ostreambuf_iterator for ostream<< output
#include <limits>   // printing ints
#include <memory>   // unique_ptr for Polymorphic
#include <string>   // printing string
#include <utility>  // forward / move

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
	 *
	 * Internally, formatters define a compile-time tree structure of formatter classes.
	 * Leaves of the tree are simple elements that holds some value and can stringify them.
	 * Pair<Left, Right> is the only composite element, and allow to form the tree.
	 * It does nothing else than propoagting api calls to the right leaves.
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

	template <typename F, typename = typename std::enable_if<IsFormatter<F>::value>::type>
	std::ostream & operator<< (std::ostream & os, const F & formatter) {
		formatter.write (std::ostreambuf_iterator<char> (os));
		return os;
	}

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
	 * Composite elements like Pair must manage placeholders manually.
	 * A PlaceHolder value named "placeholder" can be used in format() to have a short syntax.
	 * For correct lookup of operator()(...), PlaceHolder is defined after format() overloads (below).
	 *
	 * Technical doc
	 * TODO will change
	 * The current system for placeholder is compile-time only (so no substitution for Dynamic).
	 * Each formatter defines nb_placeholder (): number of placeholders in the subtree.
	 * It also defines an operator(Args...) (sizeof... Args == nb_placeholder) that does the subst.
	 * Substitution is done recursively (lots of copies !).
	 * An internal (but public) function substitute<index, Args...> is used to do the recursion.
	 * index indicates the current offset in the argument pack that the subtree should consider.
	 *
	 * A previous implementation tried to split the arg pack between sub-formatters in pair.
	 * It was just too complicated, so it was drop.
	 * Hopefully with the current one we can also support placeholders that lookup specific indexes.
	 * (as all arguments are forwarded).
	 * FIXME what about multiple move from ? ....
	 * Do everything by const & and copy ?
	 */

	template <typename Derived> struct SimpleBase : Base<Derived> {
		// Placeholder support for simple element: no placeholder, always return self
		static constexpr std::size_t nb_placeholder () { return 0; }
		Derived operator() () const { return this->self (); }
		template <int index, typename... Args> Derived substitute (Args &&...) const {
			return this->self ();
		}
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
		explicit constexpr StaticCharArray (const char (&str)[N]) : str_ (str) {}
		constexpr std::size_t size () const { return N - 1; }
		template <typename OutputIt> constexpr OutputIt write (OutputIt it) const {
			return std::copy (std::begin (str_), std::end (str_) - 1, it);
		}

	private:
		const char (&str_)[N];
	};
	template <std::size_t N> constexpr StaticCharArray<N> format (const char (&str)[N]) {
		return StaticCharArray<N>{str};
	}

	class StringRef : public SimpleBase<StringRef> {
		// Reference to std::string
	public:
		explicit StringRef (const std::string & str) : str_ (str) {}
		std::size_t size () const { return str_.size (); }
		template <typename OutputIt> OutputIt write (OutputIt it) const {
			return std::copy (std::begin (str_), std::end (str_), it);
		}

	private:
		const std::string & str_;
	};
	inline StringRef format (const std::string & str) { return StringRef{str}; }

	class CStringRef : public SimpleBase<CStringRef> {
		// Reference to c-string (const char *)
	public:
		CStringRef (const char * str, std::size_t len) : str_ (str), len_ (len) {}
		explicit CStringRef (const char * str) : CStringRef (str, std::strlen (str)) {}
		std::size_t size () const { return len_; }
		template <typename OutputIt> OutputIt write (OutputIt it) const {
			return std::copy_n (str_, len_, it);
		}

	private:
		const char * str_;
		std::size_t len_;
	};
	inline CStringRef format (const char * str) { return CStringRef{str}; }

	template <typename Int> class DecimalInteger : public SimpleBase<DecimalInteger<Int>> {
		// Prints an integer in decimal base
	public:
		explicit DecimalInteger (Int i) : i_ (i) {}
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
	DecimalInteger<T> format (T t) {
		return DecimalInteger<T>{t};
	}

	/* Composite formatter that represents a sequence of two formatters.
	 * Will apply Left then Right.
	 * The sub-formatters are stored by copy.
	 *
	 * We also define format(a, b), and operator<< (a, b).
	 *
	 * TODO forwarding constructor
	 * FIXME c++11: decltype () for substitution operations
	 * TODO restricts argument count of operator() (enable if)
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
		template <typename... Args> auto operator() (Args &&... args) const {
			return substitute<0, Args...> (std::forward<Args> (args)...);
		}
		template <int index, typename... Args> auto substitute (Args &&... args) const {
			return format (left_.substitute<index, Args...> (std::forward<Args> (args)...),
			               right_.substitute<index + Left::nb_placeholder (), Args...> (
			                   std::forward<Args> (args)...));
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
	template <typename F, typename T, typename = typename std::enable_if<IsFormatter<F>::value>::type>
	auto operator<< (F && f, T && t) -> decltype (format (std::forward<F> (f), std::forward<T> (t))) {
		return format (std::forward<F> (f), std::forward<T> (t));
	}

	// Delayed placeholder definition
	namespace Detail {
		template <int index> struct ForwardNthArg {
			template <typename First, typename... Others>
			static auto forward (First &&, Others &&... args) {
				return ForwardNthArg<index - 1>::forward (std::forward<Others> (args)...);
			}
		};
		template <> struct ForwardNthArg<0> {
			template <typename First, typename... Others> static auto forward (First && f, Others &&...) {
				return std::forward<First> (f);
			}
		};
	}
	struct PlaceHolder : public Tag {
		static constexpr std::size_t nb_placeholder () { return 1; }
		template <typename F>
		auto operator() (F && f) const -> decltype (format (std::forward<F> (f))) {
			return format (std::forward<F> (f));
		}
		template <int index, typename... Args> auto substitute (Args &&... args) const {
			return operator() (Detail::ForwardNthArg<index>::forward (std::forward<Args> (args)...));
		}
	};
	static constexpr PlaceHolder placeholder{};

	/* Polymorphic formatter.
	 * Stores a runtime variable formatter type (allow to store heterogeneous formatters in a vector).
	 *
	 * Use the runtime concept system (use a template generated virtual wrapper class hierarchy).
	 * Thus this class, and other formatter class stays inheritance-free.
	 * This class has value semantics (copy/move, etc).
	 *
	 * However, write() is not template anymore (impossible with virtual functions).
	 * write() is provided for common useful types:
	 * - char* : raw access
	 * - std::string::iterator : to allow to_string()
	 * - ostreambuf_iterator<char> : to support operator<< (std::ostream)
	 *
	 * Currently, polymorphic formatters do not support placeholder substitution...
	 * TODO partial support ?
	 */
	class Dynamic : public Base<Dynamic> {
	public:
		Dynamic () = default;
		Dynamic (const Dynamic & p) : model_ (p.model_->clone ()) {}
		Dynamic (Dynamic &&) = default;
		Dynamic & operator= (const Dynamic & p) { return *this = Dynamic (p); }
		Dynamic & operator= (Dynamic &&) = default;
		~Dynamic () = default;

		template <typename F,
		          typename = typename std::enable_if<Traits::NonSelf<F, Dynamic>::value>::type>
		explicit Dynamic (F && formatter) : model_ (new Model<F> (std::forward<F> (formatter))) {}

		// Wrappers
		std::size_t size () const { return model_->size (); }
		template <typename OutputIt> OutputIt write (OutputIt it) const { return model_->write (it); }

	private:
		struct Interface {
			virtual ~Interface () = default;
			virtual Interface * clone () const = 0;
			virtual std::size_t size () const = 0;
			virtual char * write (char *) const = 0;
			virtual std::string::iterator write (std::string::iterator) const = 0;
			virtual std::ostreambuf_iterator<char> write (std::ostreambuf_iterator<char>) const = 0;
		};
		template <typename F> struct Model final : public Interface {
			F formatter_;
			Model (const F & f) : formatter_ (f) {}
			Model (F && f) : formatter_ (std::move (f)) {}
			Model * clone () const override { return new Model (*this); }
			std::size_t size () const override { return formatter_.size (); }
			char * write (char * p) const override { return formatter_.write (p); }
			std::string::iterator write (std::string::iterator it) const override {
				return formatter_.write (it);
			}
			std::ostreambuf_iterator<char> write (std::ostreambuf_iterator<char> it) const override {
				return formatter_.write (it);
			}
		};
		std::unique_ptr<Interface> model_{};
	};
}

using Format::format;
using Format::placeholder;
}
