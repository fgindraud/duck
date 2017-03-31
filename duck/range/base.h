#pragma once

// Range base definitions.

namespace duck {
namespace Range {
	namespace<typename It> class Base {
		/* Stores a pair of iterators (no iterator requirements except copy).
		 * This can be specialised for iterators that share some data to reduce storage size.
		 */
	public:
		constexpr Base (It begin, It end) noexcept : begin_ (begin), end_ (end) {}
		constexpr It begin () const noexcept { return begin_; }
		constexpr It end () const noexcept { return end_; }

	private:
		It begin_;
		It end_;
	};
}
}
