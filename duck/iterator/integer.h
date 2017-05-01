#pragma once

// Defines IntegerIterator (iterates on integral number spaces).
// Allows to generate an integer range with all range features.

#include <iterator>

namespace duck {

namespace Iterator {
	template <typename Int> class Integer {
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = Int;
		using difference_type = std::ptrdiff_t;
		using pointer = const value_type *;
		using reference = value_type; // Not standard compliant

		constexpr Integer () noexcept = default;
		constexpr Integer (Int n) noexcept : n_{n} {}

		// Input / output
		Integer & operator++ () noexcept { return ++n_, *this; }
		constexpr reference operator* () const noexcept { return n_; }
		constexpr pointer operator-> () const noexcept { return &n_; }
		constexpr bool operator== (const Integer & o) const noexcept { return n_ == o.n_; }
		constexpr bool operator!= (const Integer & o) const noexcept { return n_ != o.n_; }

		// Forward
		Integer operator++ (int) noexcept {
			Integer tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir
		Integer & operator-- () noexcept { return --n_, *this; }
		Integer operator-- (int) noexcept {
			Integer tmp (*this);
			--*this;
			return tmp;
		}

		// Random access
		Integer & operator+= (difference_type n) noexcept { return n_ += n, *this; }
		constexpr Integer operator+ (difference_type n) const noexcept { return Integer (n_ + n); }
		friend constexpr Integer operator+ (difference_type n, const Integer & it) noexcept {
			return it + n;
		}
		Integer & operator-= (difference_type n) noexcept { return n_ -= n, *this; }
		constexpr Integer operator- (difference_type n) const noexcept { return Integer (n_ - n); }
		constexpr difference_type operator- (const Integer & o) const noexcept { return n_ - o.n_; }
		constexpr reference operator[] (difference_type n) const noexcept { return n_ + n; }
		constexpr bool operator< (const Integer & o) const noexcept { return n_ < o.n_; }
		constexpr bool operator> (const Integer & o) const noexcept { return n_ > o.n_; }
		constexpr bool operator<= (const Integer & o) const noexcept { return n_ <= o.n_; }
		constexpr bool operator>= (const Integer & o) const noexcept { return n_ >= o.n_; }

	private:
		Int n_{};
	};
}
}
