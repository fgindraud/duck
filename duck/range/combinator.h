#pragma once

// Range V2
// STATUS: WIP

#include <duck/range/range.h>

namespace duck {
namespace Range {
	/* Ranges base combinators lib.
	 */

	/********************************************************************************
	 * Slice. TODO
	 */
	template <typename R> class Sliced;

	template <typename R> struct RangeTraits<Sliced<R>> : RangeTraits<R> {};

	template <typename R> class Sliced : public Base<Sliced<R>> {};

	/********************************************************************************
	 * Counted range.
	 * TODO finish, be careful of ref/pointers to temporaries
	 */
	template <typename R, typename IntType> class Counted;

	template <typename It, typename IntType> class CountedIterator {
		static_assert (IsIterator<It>::value, "It must be an iterator type");
		static_assert (std::is_integral<IntType>::value, "IntType must be integral");

	public:
		using iterator_category = typename std::iterator_traits<It>::iterator_category;
		struct value_type {
			typename std::iterator_traits<It>::reference value; // FIXME make value / index methods
			IntType index;
		};
		using difference_type = typename std::iterator_traits<It>::difference_type;
		using pointer = const value_type *;
		using reference = value_type; // Not standard compliant

		constexpr CountedIterator () noexcept = default;
		constexpr CountedIterator (It it, IntType n) : it_ (it), n_ (n) {}

		// Input / output
		CountedIterator & operator++ () noexcept { return ++it_, ++n_, *this; }
		constexpr reference operator* () const noexcept { return {*it_, n_}; }
		constexpr pointer operator-> () const noexcept { return &n_; }
		constexpr bool operator== (const CountedIterator & o) const noexcept { return it_ == o.it_; }
		constexpr bool operator!= (const CountedIterator & o) const noexcept { return it_ != o.it_; }

		// Forward
		CountedIterator operator++ (int) noexcept {
			CountedIterator tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir
		CountedIterator & operator-- () noexcept { return --it_, --n_, *this; }
		CountedIterator operator-- (int) noexcept {
			CountedIterator tmp (*this);
			--*this;
			return tmp;
		}

		// Random access
		CountedIterator & operator+= (difference_type n) noexcept { return it_ += n, n_ += n, *this; }
		constexpr CountedIterator operator+ (difference_type n) const noexcept {
			return CountedIterator (it_ + n, n_ + n);
		}
		friend constexpr CountedIterator operator+ (difference_type n,
		                                            const CountedIterator & it) noexcept {
			return it + n;
		}
		CountedIterator & operator-= (difference_type n) noexcept { return it_ -= n, n_ -= n,*this; }
		constexpr CountedIterator operator- (difference_type n) const noexcept {
			return CountedIterator (it_ - n, n_ - n);
		}
		constexpr difference_type operator- (const CountedIterator & o) const noexcept {
			return it_ - o.it_;
		}
		constexpr reference operator[] (difference_type n) const noexcept { return *(*this + n); }
		constexpr bool operator< (const CountedIterator & o) const noexcept { return it_ < o.it_; }
		constexpr bool operator> (const CountedIterator & o) const noexcept { return it_ > o.it_; }
		constexpr bool operator<= (const CountedIterator & o) const noexcept { return it_ <= o.it_; }
		constexpr bool operator>= (const CountedIterator & o) const noexcept { return it_ >= o.it_; }

	private:
		It it_{};
		IntType n_{};
	};

	template <typename R, typename IntType> struct RangeTraits<Counted<R, IntType>> {
		using Iterator = CountedIterator<typename RangeTraits<R>::Iterator, IntType>;
		using SizeType = typename RangeTraits<R>::SizeType;
	};

	template <typename R, typename IntType> class Counted : public Base<Counted<R, IntType>> {};

	// TODO add type tags (Combinators namespace ?)

} // namespace Range
} // namespace duck
