#pragma once

// Defines IntegerIterator (iterates on integral number spaces).
// Allows to generate an integer range with all range features.

#include <duck/range.h>
#include <iterator>
#include <type_traits>
#include <utility>

namespace duck {

namespace Iterator {
	template <typename Int> class Integer {
		// Immutable random iterator on integers.
		// Is explicit to allow integer ranges to have a contains() method for ints.
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = Int;
		using difference_type = std::ptrdiff_t;
		using pointer = const value_type *;
		using reference = const value_type &;

		// iterator
		Integer () noexcept = default;
		Integer (Int n) noexcept : n_ (n) {}
		// + default ctors
		void swap (Integer & it) noexcept { std::swap (n_, it.n_); }
		reference operator* () const noexcept { return n_; }
		Integer & operator++ () noexcept {
			++n_;
			return *this;
		}

		// input/forward iterator
		bool operator== (const Integer & rhs) const noexcept { return n_ == rhs.n_; }
		bool operator!= (const Integer & rhs) const noexcept { return n_ != rhs.n_; }
		pointer operator-> () const noexcept { return std::addressof (n_); }
		Integer operator++ (int) noexcept {
			auto tmp = *this;
			++*this;
			return tmp;
		}

		// bidirectional iterator
		Integer & operator-- () noexcept {
			--n_;
			return *this;
		}
		Integer operator-- (int) noexcept {
			auto tmp = *this;
			--*this;
			return tmp;
		}

		// random access iterator
		template <typename T> Integer & operator+= (T && t) noexcept {
			return *this = *this + std::forward<T> (t);
		}
		template <typename T> Integer operator+ (T && t) const noexcept {
			return Integer (n_ + std::forward<T> (t));
		}
		template <typename T> Integer & operator-= (T && t) noexcept {
			return *this = *this - std::forward<T> (t);
		}
		template <typename T> Integer operator- (T && t) const noexcept {
			return Integer (n_ - std::forward<T> (t));
		}
		difference_type operator- (Integer it) const noexcept { return n_ - it.n_; }
		reference operator[] (std::size_t n) const noexcept { return *(*this + n); }
		bool operator< (const Integer & rhs) const noexcept { return n_ < rhs.n_; }
		bool operator> (const Integer & rhs) const noexcept { return n_ > rhs.n_; }
		bool operator<= (const Integer & rhs) const noexcept { return n_ <= rhs.n_; }
		bool operator>= (const Integer & rhs) const noexcept { return n_ >= rhs.n_; }

	private:
		Int n_{};
	};

	// Out of class functions for Integer
	template <typename Int> inline void swap (Integer<Int> & lhs, Integer<Int> & rhs) {
		lhs.swap (rhs);
	}
	template <typename Int, typename T> inline Integer<Int> operator+ (T && t, Integer<Int> it) {
		return it + std::forward<T> (t);
	}
}

// Factory functions for integral types.
template <typename Int, typename = std::enable_if_t<std::is_integral<Int>::value>>
auto range (Int from, Int to) {
	return Range<Iterator::Integer<Int>>{from, to};
}
template <typename Int, typename = std::enable_if_t<std::is_integral<Int>::value>>
auto range (Int to) {
	return range (Int{0}, to);
}

// Index factory for containers (enabled if they define a size_type).
template <typename Container, typename = typename Container::size_type>
auto index_range (const Container & container) {
	return range (container.size ());
}

// Some specific operations for integer ranges.
// TODO * scalar, + other range ?
}
