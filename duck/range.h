#pragma once

// Defines the range base class, and factory functions to build it from container-like objects.

#include <duck/iterator.h>
#include <iterator>
#include <type_traits>

namespace duck {

template <typename It> class Range {
	/* Represents a matching pair of iterators.
	 *
	 * This class has constant value semantics.
	 * It can not be modified, but new ranges can be built from it.
	 * It can be passed by value.
	 *
	 * This class has the same properties has the underlying iterator type.
	 * This applies for invalidation, and which operations can be used.
	 * Some operations may work inefficiently for low capability iterators (like size ()).
	 *
	 * Beware with input iterators, as some read-only operations (like size()) might iterate on them.
	 * This will "invalidate all iterator copies", which can be bad and is difficult to check.
	 * It is better to use this class starting at forward iterators.
	 * Output only iterator are even more dangerous (size() might be UB).
	 *
	 * TODO more slices (to_end, from_start).
	 * TODO checked operations ?
	 */
public:
	using ValueType = Iterator::GetValueType<It>;
	using ReferenceType = Iterator::GetReferenceType<It>;
	using DifferenceType = Iterator::GetDifferenceType<It>;
	using IteratorCategory = Iterator::GetCategory<It>;

	constexpr Range (It begin, It end) noexcept : begin_ (begin), end_ (end) {}

	// Basic access (also makes it for-range capable)
	constexpr It begin () const noexcept { return begin_; }
	constexpr It end () const noexcept { return end_; }

	// Input iterator
	bool empty () const noexcept { return begin_ == end_; }
	ReferenceType front () const noexcept { return *begin_; }
	Range pop_front () const noexcept { return {std::next (begin_), end_}; }

	// Bidirectional iterator
	Range pop_back () const noexcept { return {begin_, std::prev (end_)}; }
	ReferenceType back () const noexcept { return *std::prev (end_); }

	// Random access iterator
	DifferenceType size () const noexcept { return std::distance (begin_, end_); }
	Range pop_front (DifferenceType n) const noexcept {
		return {Iterator::advance (begin_, n), end_};
	}
	Range pop_back (DifferenceType n) const noexcept {
		return {begin_, Iterator::advance (end_, -n)};
	}
	ReferenceType operator[] (std::size_t n) const noexcept { return begin_[n]; }
	bool contains (It it) const noexcept { return begin_ <= it && it < end_; }
	DifferenceType offset_of (It it) const noexcept { return std::distance (begin_, it); }

	// "nicer" api (python like slice ; but at(size ()) return end ())
	It at (DifferenceType n) const noexcept {
		auto index = n < 0 ? n + size () : n;
		return Iterator::advance (begin_, index);
	}
	Range slice (DifferenceType from, DifferenceType to) const noexcept {
		return {at (from), at (to)};
	}

	template <typename Container> Container to_container () const { return Container (begin_, end_); }

private:
	It begin_;
	It end_;
};

// Factory function from pair of iterators (enabled if it defines a category).
template <typename It, typename = Iterator::GetCategory<It>> Range<It> range (It first, It last) {
	return {first, last};
}

// Factory functions for containers (enabled if Container defines the proper iterators)
template <typename Container, typename It = typename Container::iterator>
auto range (Container & container) {
	return Range<It>{container.begin (), container.end ()};
}
template <typename Container, typename It = typename Container::const_iterator>
auto range (const Container & container) {
	return Range<It>{container.begin (), container.end ()};
}
}
