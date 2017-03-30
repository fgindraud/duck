#pragma once

#include <iterator>
#include <type_traits>

namespace duck {

template <typename Iterator> class Range {
	// Represents a range of iterators.
	// This is a non putable value type (small, can be passed by value).
	// This is a temporary object, has the same invalidation properties as the iterator type used.
	// Range is not mutable: we can only build new ranges from it.
	//
	// TODO slice to end / from start
	// TODO checked operations ?
private:
	using Traits = typename std::iterator_traits<Iterator>;

public:
	using ValueType = typename Traits::value_type;
	using ReferenceType = typename Traits::reference;
	using DifferenceType = typename Traits::difference_type;

	constexpr Range (Iterator begin, Iterator end) noexcept : begin_ (begin), end_ (end) {}

	// Basic access (also makes it for-range capable)
	constexpr Iterator begin () const noexcept { return begin_; }
	constexpr Iterator end () const noexcept { return end_; }

	// Input iterator
	bool empty () const noexcept { return begin_ == end_; }
	ReferenceType front () const noexcept { return *begin_; }
	Range pop_front () const noexcept { return {std::next (begin_), end_}; }

	// Bidirectional iterator
	Range pop_back () const noexcept { return {begin_, std::prev (end_)}; }
	ReferenceType back () const noexcept { return *std::prev (end_); }

	// Random access iterator
	DifferenceType size () const noexcept { return std::distance (begin_, end_); }
	Range pop_front (DifferenceType n) const noexcept { return {shift (begin_, n), end_}; }
	Range pop_back (DifferenceType n) const noexcept { return {begin_, shift (end_, -n)}; }
	ReferenceType operator[] (std::size_t n) const noexcept { return begin_[n]; }
	bool contains (Iterator it) const noexcept { return begin_ <= it && it < end_; }
	DifferenceType offset_of (Iterator it) const noexcept { return std::distance (begin_, it); }

	// "nicer" api (python like slice ; but at(size ()) return end ())
	Iterator at (DifferenceType n) const noexcept {
		auto index = n < 0 ? n + size () : n;
		return shift (begin_, index);
	}
	Range slice (DifferenceType from, DifferenceType to) const noexcept {
		return {at (from), at (to)};
	}

	template <typename Container> Container to_container () const { return Container (begin_, end_); }

private:
	static Iterator shift (Iterator it, DifferenceType n) noexcept {
		std::advance (it, n);
		return it;
	}

	Iterator begin_;
	Iterator end_;
};

// Factory functions for containers (enabled if Container defines the proper iterators)
template <typename Container, typename Iterator = typename Container::iterator>
Range<Iterator> range (Container & container) {
	return {container.begin (), container.end ()};
}
template <typename Container, typename Iterator = typename Container::const_iterator>
Range<Iterator> range (const Container & container) {
	return {container.begin (), container.end ()};
}
}
