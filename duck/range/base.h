#pragma once

// Range base class.

#include <duck/iterator.h>
#include <utility>

namespace duck {
namespace Range {
	template <typename It> class Base {
		/* Stores a pair of iterators (no iterator requirements except copy).
		 * This can be specialized for iterators that share some data to reduce storage size.
		 * For simplicity, has value semantics and should be passed by value.
		 */
	public:
		// Main constructor.
		constexpr Base (It begin, It end) noexcept
		    : begin_ (std::move (begin)), end_ (std::move (end)) {}

		constexpr It begin () const noexcept { return begin_; }
		constexpr It end () const noexcept { return end_; }

	private:
		It begin_;
		It end_;
	};

	// Retrieve iterator type
	template<typename T> struct GetIteratorImpl;
	template<typename T> using GetIterator = typename GetIteratorImpl<T>::Type;
	template<typename It> struct GetIteratorImpl<Base<It>> { using Type = It; };

	// Factories for base.

	// From iterator pair (if it is considered an iterator by STL).
	template <typename It, typename = Iterator::GetCategory<It>>
	Base<It> make_base (It begin, It end) {
		return {std::move (begin), std::move (end)};
	}

	// From container (if it accepts std::begin).
	template <typename Container>
	auto make_base (Container & container) -> Base<decltype (std::begin (container))> {
		return {std::begin (container), std::end (container)};
	}
	template <typename Container>
	auto make_base (const Container & container) -> Base<decltype (std::begin (container))> {
		return {std::begin (container), std::end (container)};
	}
}
}
