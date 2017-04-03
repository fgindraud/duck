#pragma once

// Range base class and base free functions.

/* Input/output only iterators are annoying.
 * They might invoke UB easily if parsed twice.
 * With this lib, it can happen easily (parsing can be triggered by size ()).
 * FIXME try to handle them one day...
 * Possible solutions:
 * - move only iterator if non-forward (fails the Iterator property...)
 * - adding enable_if everywhere...
 */

#include <iterator>
#include <type_traits>
#include <utility>

namespace duck {
namespace Range {
	template <typename It> class Base {
		/* Stores a pair of iterators (no iterator requirements except copy).
		 * This can be specialized for iterators that share some data to reduce storage size.
		 */
	public:
		// Main constructor.
		template <typename T, typename U,
		          typename = std::enable_if_t<std::is_constructible<It, T>::value>,
		          typename = std::enable_if_t<std::is_constructible<It, U>::value>>
		constexpr Base (T && begin, U && end) noexcept
		    : begin_ (std::forward<T> (begin)), end_ (std::forward<U> (end)) {}

		constexpr const It & begin () const noexcept { return begin_; }
		constexpr It & begin () noexcept { return begin_; }
		constexpr const It & end () const noexcept { return end_; }
		constexpr It & begin () noexcept { return begin_; }

	private:
		It begin_;
		It end_;
	};

	/* Functionalities are provided as free functions.
	 *
	 * The provided version is a default one relying exclusively on Base<It> API.
	 * They can be specialized for specific iterators types.
	 *
	 * These functions are sorted by iterator category.
	 * Some are only available from that category (like back).
	 * Others may be usable before their category but have high cost (size ()).
	 */

	// Input/forward iterator
	template <typename It> bool empty (const Base<It> & r) { return r.begin () == r.end (); }
	template <typename It> auto front (const Base<It> & r) { return *r.begin (); }
	template <typename It> void pop_front (Base<It> & r) { ++r.begin (); }

	// Bidirectional iterator
	template <typename It> auto back (const Base<It> & r) { return *std::prev (r.end ()); }
	template <typename It> void pop_back (Base<It> & r) { --r.end (); }

	// Random access iterator
	template <typename It> auto size (const Base<It> & r) {
		return std::distance (r.begin (), r.end ());
	}
	template <typename It> auto value_at (const Base<It> & r, std::size_t n) { return r.begin ()[n]; }
	template <typename It> void pop_front (Base<It> & r, std::size_t n) {
		std::advance (r.begin (), n);
	}
	template <typename It> void pop_back (Base<It> & r, std::size_t n) {
		std::advance (r.end (), -n);
	}

	// TODO other funcs ?
	template <typename It> auto iterator_at (const Base<It> & r, std::size_t n) {
		return std::next (r.begin (), n);
	}
}
}
