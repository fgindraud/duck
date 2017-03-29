#pragma once

#include <cassert>
#include <iterator>
#include <type_traits>

template <typename T> class Range {
	/* Represents a range of elements.
	 * T should be a small copyable type.
	 */
private:
	const T start_;
	const T end_;

public:
	constexpr Range (T start, T end) : start_ (start), end_ (end) { assert (start <= end); }
	constexpr Range (T n) : Range (T (), n) {} // expects an integer T

	// Accessors
	constexpr T first (void) const { return start_; }
	constexpr T last (void) const { return end_; }

	constexpr bool contains (T t) const { return first () <= t && t < last (); }
	constexpr size_t size (void) const { return last () - first (); }

	constexpr bool includes (const Range & r) const {
		return first () <= r.first () && r.last () <= last ();
	}

	// Iterator
	struct const_iterator : public std::iterator<std::bidirectional_iterator_tag, T> {
	private:
		T i_;
		constexpr const_iterator (T i) : i_ (i) {}
		friend class Range;

	public:
		constexpr const_iterator () : const_iterator (T ()) {}
		constexpr bool operator== (const_iterator other) const { return i_ == other.i_; }
		constexpr bool operator!= (const_iterator other) const { return i_ != other.i_; }
		constexpr const_iterator & operator++ (void) {
			++i_;
			return *this;
		}
		constexpr const_iterator & operator-- (void) {
			--i_;
			return *this;
		}
		constexpr const_iterator operator++ (int) {
			auto cpy = *this;
			++*this;
			return cpy;
		}
		constexpr const_iterator operator-- (int) {
			auto cpy = *this;
			--*this;
			return cpy;
		}
		constexpr T operator* (void) const { return i_; }
		constexpr const T * operator-> (void) const { return &i_; }
	};

	constexpr const_iterator begin (void) const { return {start_}; }
	constexpr const_iterator end (void) const { return {end_}; }
};

// Constructors
template <typename T> constexpr Range<T> range (T start, T end) {
	return {start, end};
}
template <typename T> constexpr Range<T> range (T n) {
	return {n};
}
template <typename T, typename U> constexpr auto range_from_offset (T start, U offset) {
	return range (start, start + offset);
}

// Operators
template <typename T, typename U> constexpr auto operator* (const Range<T> & r, U u) {
	return range (r.first () * u, r.last () * u);
}
template <typename T, typename U> constexpr auto operator* (U u, const Range<T> & r) {
	return range (u * r.first (), u * r.last ());
}
template <typename T, typename U> constexpr auto operator+ (const Range<T> & r, U u) {
	return range (r.first () + u, r.last () + u);
}
template <typename T, typename U> constexpr auto operator+ (U u, const Range<T> & r) {
	return range (u + r.first (), u + r.last ());
}
