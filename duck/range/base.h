#pragma once

// Range base class.

#include <duck/iterator/traits.h>
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
}
}
