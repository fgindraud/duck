#pragma once

// Formatting library (core components).

#include <duck/type_traits.h>
#include <iosfwd>   // std::ostream declaration
#include <iterator> // std::ostreambuf_iterator for ostream<< output
#include <memory>   // unique_ptr for Polymorphic
#include <string>   // to_string / string iterator
#include <utility>  // forward / move

namespace duck {
namespace Format {
	/* A "formatter" is a class that can stringify some data.
	 * It can be simple (one piece of data) or composite (think of format strings + args).
	 * A formatter can be "executed": its content is written as chars to something.
	 *
	 * Formatters must define two functions:
	 * - std::size_t size () const: returns the size required to format the element.
	 * - template<typename It> It write (It it) const:
	 *   - format the element and write the result to "it" output iterator
	 *   - must increment "it" exactly size() times
	 *   - return the incremented iterator
	 *
	 * In practice, formatters define a compile-time tree structure of formatter classes.
	 * Leaves of the tree are called "elements", they each hold a value and can stringify them.
	 * Pair<Left, Right> is the only composite element, and allow to form the tree.
	 * It does nothing else than propagating API calls to the right leaves.
	 */

	/* "Tag" is a type tag used to recognize a formatter class.
	 * All formatter classes must inherit from it to be recognized by <<, format(), ...
	 */
	struct Tag {};
	template <typename T> using IsFormatter = std::is_base_of<Tag, T>;

	/* Base<Derived> is a CRTP class that adds utility functions to formatter classes.
	 * All formatter classes should inherit from it (struct T : Base<T>, as per CRTP).
	 * Base inherits from Tag so no need to explicitely inherit from Tag.
	 */
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

	/* Overload operator<< (std::ostream&) to output a formatter in a std::ostream.
	 */
	template <typename F, typename = typename std::enable_if<IsFormatter<F>::value>::type>
	std::ostream & operator<< (std::ostream & os, const F & formatter) {
		formatter.write (std::ostreambuf_iterator<char> (os));
		return os;
	}

	/* format_element() is a free function used to generate a formatter from a value.
	 * It creates the right formatter class by selecting one of the many overloads that are defined.
	 * To add support to a type T, an overload of format_element must be placed in either:
	 * - the duck::Format namespace
	 * - the T type namespace
	 * The function prototype MUST also have the dummy AdlTag argument.
	 * It forces the compiler to search in duck::Format namespace under the ADL rules.
	 * This allows definitions of elements formatters and their overloads to be off core.h.
	 * In core.h, the only overload is the bypass overload, which forwards a T if it is a formatter.
	 */
	struct AdlTag {};

	template <typename F, typename = typename std::enable_if<IsFormatter<F>::value>::type>
	constexpr F format_element (F f, AdlTag) {
		return f;
	}

	template <typename T>
	using FormatterTypeFor = decltype (format_element (std::declval<T> (), AdlTag{}));

	/* TODO fix doc, improve code
	 * In addition to regular formatter, a formatter can have placeholders.
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

	template <typename Derived> struct ElementBase : Base<Derived> {
		// Placeholder support for simple element: no placeholder, always return self
		static constexpr std::size_t nb_placeholder () { return 0; }
		template <int index, typename... Args> Derived substitute (Args &&...) const {
			return this->self ();
		}
		Derived operator() () const { return this->self (); }
	};

	namespace Detail {
		template <int index> struct ForwardNthArg {
			template <typename First, typename... Others>
			static auto forward (First &&, Others &&... args)
			    -> decltype (ForwardNthArg<index - 1>::forward (std::forward<Others> (args)...)) {
				return ForwardNthArg<index - 1>::forward (std::forward<Others> (args)...);
			}
		};
		template <> struct ForwardNthArg<0> {
			template <typename First, typename... Others>
			static auto forward (First && f, Others &&...) -> decltype (std::forward<First> (f)) {
				return std::forward<First> (f);
			}
		};
	}
	struct PlaceHolder : public Tag {
		static constexpr std::size_t nb_placeholder () { return 1; }
		template <typename F> auto operator() (F && f) const -> FormatterTypeFor<F> {
			return format_element (std::forward<F> (f), AdlTag{});
		}
		template <int index, typename... Args>
		auto substitute (Args &&... args) const -> FormatterTypeFor<
		    decltype (Detail::ForwardNthArg<index>::forward (std::forward<Args> (args)...))> {
			return operator() (Detail::ForwardNthArg<index>::forward (std::forward<Args> (args)...));
		}
	};
	static constexpr PlaceHolder placeholder{};

	/* Null formatter, equivalent to empty string.
	 */
	struct Null : public ElementBase<Null> {
		constexpr std::size_t size () const { return 0; }
		template <typename OutputIt> constexpr OutputIt write (OutputIt it) const { return it; }
	};

	/* Pair, represents a sequence of two formatters (Left then Right).
	 * This is used to create composite formatters as a template class hierarchy.
	 * Left and Right are stored by copy.
	 * TODO forwarding constructor
	 * TODO restricts argument count of operator() (enable if)
	 */
	template <typename Left, typename Right> class Pair : public Base<Pair<Left, Right>> {
	private:
		Left left_;
		Right right_;

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
		template <int index, typename... Args>
		auto substitute (Args &&... args) const -> decltype (
		    format (left_.template substitute<index, Args...> (std::forward<Args> (args)...),
		            right_.template substitute<index + Left::nb_placeholder (), Args...> (
		                std::forward<Args> (args)...))) {
			return format (left_.template substitute<index, Args...> (std::forward<Args> (args)...),
			               right_.template substitute<index + Left::nb_placeholder (), Args...> (
			                   std::forward<Args> (args)...));
		}
		template <typename... Args>
		auto operator() (Args &&... args) const
		    -> decltype (substitute<0, Args...> (std::forward<Args> (args)...)) {
			return this->template substitute<0, Args...> (std::forward<Args> (args)...);
		}
	};

	/* The frontend API of the formatter library is the duck::format(...) free function.
	 * format() should not be overloaded by external code, prefer overloading format_element.
	 *
	 * format() returns a Null formatter (equivalent to "").
	 * This can be used to start a formatter chain with operator<< : format() << "blah" << ...
	 *
	 * format(Args... args) creates a formatter which is a sequence of formatters built from args.
	 * Each individual formatter will be built by using an overload of format_element.
	 * The sequence is represented by the Pair composition class.
	 *
	 * An overload of operator<< allows composition of formatters with the << syntax.
	 *
	 * Overloads of format below try to remove Null formatters when they can.
	 */
	inline constexpr Null format () { return {}; }

	template <typename T> constexpr auto format (T && t) -> FormatterTypeFor<T> {
		return format_element (std::forward<T> (t), AdlTag{});
	}

	template <typename First>
	constexpr auto format (First && first, Null) -> FormatterTypeFor<First> {
		return format_element (std::forward<First> (first), AdlTag{});
	}

	template <typename Second, typename... Others>
	constexpr auto format (Null, Second && second, Others &&... others)
	    -> decltype (format (std::forward<Second> (second), std::forward<Others> (others)...)) {
		return format (std::forward<Second> (second), std::forward<Others> (others)...);
	}

	template <typename First, typename Second, typename... Others>
	auto format (First && first, Second && second, Others &&... others)
	    -> Pair<FormatterTypeFor<First>,
	            decltype (format (std::forward<Second> (second), std::forward<Others> (others)...))> {
		return {format_element (std::forward<First> (first), AdlTag{}),
		        format (std::forward<Second> (second), std::forward<Others> (others)...)};
	}

	template <typename F, typename T, typename = typename std::enable_if<IsFormatter<F>::value>::type>
	auto operator<< (F && f, T && t) -> decltype (format (std::forward<F> (f), std::forward<T> (t))) {
		return format (std::forward<F> (f), std::forward<T> (t));
	}

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
