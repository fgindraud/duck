#pragma once

#include <duck/range.h>
#include <iterator>
#include <type_traits>
#include <utility>

namespace duck {

template <typename Int> class IntIterator {
	// Immutable random iterator on integers.
	// Is explicit to allow integer ranges to have a contains() method for ints.
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = Int;
	using difference_type = std::ptrdiff_t;
	using pointer = const value_type *;
	using reference = const value_type &;

	// iterator
	IntIterator () noexcept = default;
	IntIterator (Int n) noexcept : n_ (n) {}
	// + default ctors
	void swap (IntIterator & it) noexcept { std::swap (n_, it.n_); }
	reference operator* () const noexcept { return n_; }
	IntIterator & operator++ () noexcept {
		++n_;
		return *this;
	}

	// input/forward iterator
	bool operator== (const IntIterator & rhs) const noexcept { return n_ == rhs.n_; }
	bool operator!= (const IntIterator & rhs) const noexcept { return n_ != rhs.n_; }
	pointer operator-> () const noexcept { return std::addressof (n_); }
	IntIterator operator++ (int) noexcept {
		IntIterator tmp (*this);
		++*this;
		return tmp;
	}

	// bidirectional iterator
	IntIterator & operator-- () noexcept {
		--n_;
		return *this;
	}
	IntIterator operator-- (int) noexcept {
		IntIterator tmp (*this);
		--*this;
		return tmp;
	}

	// random access iterator
	template <typename T> IntIterator & operator+= (T && t) noexcept {
		return *this = *this + std::forward<T> (t);
	}
	template <typename T> IntIterator operator+ (T && t) const noexcept {
		return IntIterator (n_ + std::forward<T> (t));
	}
	template <typename T> IntIterator & operator-= (T && t) noexcept {
		return *this = *this - std::forward<T> (t);
	}
	template <typename T> IntIterator operator- (T && t) const noexcept {
		return IntIterator (n_ - std::forward<T> (t));
	}
	difference_type operator- (IntIterator it) const noexcept { return n_ - it.n_; }
	reference operator[] (std::size_t n) const noexcept { return *(*this + n); }
	bool operator< (const IntIterator & rhs) const noexcept { return n_ < rhs.n_; }
	bool operator> (const IntIterator & rhs) const noexcept { return n_ > rhs.n_; }
	bool operator<= (const IntIterator & rhs) const noexcept { return n_ <= rhs.n_; }
	bool operator>= (const IntIterator & rhs) const noexcept { return n_ >= rhs.n_; }

private:
	Int n_{};
};

// Out of class functions for IntIterator
template <typename Int> inline void swap (IntIterator<Int> & lhs, IntIterator<Int> & rhs) {
	lhs.swap (rhs);
}
template <typename Int, typename T>
inline IntIterator<Int> operator+ (T && t, IntIterator<Int> it) {
	return it + std::forward<T> (t);
}

// Factory functions for integral types.
template <typename Int, typename = std::enable_if_t<std::is_integral<Int>::value>>
Range<IntIterator<Int>> range (Int from, Int to) {
	return {from, to};
}
template <typename Int, typename = std::enable_if_t<std::is_integral<Int>::value>>
Range<IntIterator<Int>> range (Int to) {
	return {Int (0), to};
}

// Some specific operations for integer ranges.
// TODO * scalar, + other range
}
