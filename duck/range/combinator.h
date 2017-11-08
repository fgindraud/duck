#pragma once

// Range V2
// STATUS: WIP

#include <duck/range/range.h>
#include <limits>

namespace duck {
namespace Range {
	/* Ranges base combinators lib.
	 */

	/********************************************************************************
	 * Slicing. TODO
	 * Just pack lambdas that transform iterators ?
	 */

	/********************************************************************************
	 * Counted range.
	 *
	 * end() pointer index is UB.
	 * TODO finish, be careful of ref/pointers to temporaries
	 */
	template <typename R, typename IntType> class Counted;

	template <typename It, typename IntType> class CountedIterator {
		static_assert (IsIterator<It>::value, "It must be an iterator type");
		static_assert (std::is_integral<IntType>::value, "IntType must be integral");

	public:
		using iterator_category = typename std::iterator_traits<It>::iterator_category;
		struct value_type {
			It it;
			IntType index;
			constexpr value_type () = default;
			constexpr value_type (It it_arg, IntType index_arg) : it (it_arg), index (index_arg) {}
			typename std::iterator_traits<It>::reference value () const { return *it; }
		};
		using difference_type = typename std::iterator_traits<It>::difference_type;
		using pointer = const value_type *;
		using reference = const value_type &;

		constexpr CountedIterator () = default;
		constexpr CountedIterator (It it, IntType index) : d_ (it, index) {}

		// Input / output
		CountedIterator & operator++ () { return ++d_.it, ++d_.index, *this; }
		constexpr reference operator* () const { return d_; }
		constexpr pointer operator-> () const { return &d_; }
		constexpr bool operator== (const CountedIterator & o) const { return d_.it == o.d_.it; }
		constexpr bool operator!= (const CountedIterator & o) const { return d_.it != o.d_.it; }

		// Forward
		CountedIterator operator++ (int) {
			CountedIterator tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir
		CountedIterator & operator-- () { return --d_.it, --d_.index, *this; }
		CountedIterator operator-- (int) {
			CountedIterator tmp (*this);
			--*this;
			return tmp;
		}

		// Random access
		CountedIterator & operator+= (difference_type n) { return d_.it += n, d_.index += n, *this; }
		constexpr CountedIterator operator+ (difference_type n) const {
			return CountedIterator (d_.it + n, d_.index + n);
		}
		friend constexpr CountedIterator operator+ (difference_type n, const CountedIterator & it) {
			return it + n;
		}
		CountedIterator & operator-= (difference_type n) { return d_.it -= n, d_.index -= n, *this; }
		constexpr CountedIterator operator- (difference_type n) const {
			return CountedIterator (d_.it - n, d_.index - n);
		}
		constexpr difference_type operator- (const CountedIterator & o) const {
			return d_.it - o.d_.it;
		}
		constexpr value_type operator[] (difference_type n) const { return *(*this + n); }
		constexpr bool operator< (const CountedIterator & o) const { return d_.it < o.d_.it; }
		constexpr bool operator> (const CountedIterator & o) const { return d_.it > o.d_.it; }
		constexpr bool operator<= (const CountedIterator & o) const { return d_.it <= o.d_.it; }
		constexpr bool operator>= (const CountedIterator & o) const { return d_.it >= o.d_.it; }

	private:
		value_type d_{};
	};

	template <typename R, typename IntType> struct RangeTraits<Counted<R, IntType>> {
		using Iterator = CountedIterator<typename RangeTraits<R>::Iterator, IntType>;
		using SizeType = typename RangeTraits<R>::SizeType;
	};

	template <typename R, typename IntType> class Counted : public Base<Counted<R, IntType>> {
	public:
		constexpr Counted (const R & r) : inner_ (r) {}
		constexpr Counted (R && r) : inner_ (std::move (r)) {}

		constexpr typename RangeTraits<Counted<R, IntType>>::Iterator begin () const {
			return {inner_.begin (), 0};
		}
		constexpr typename RangeTraits<Counted<R, IntType>>::Iterator end () const {
			return {inner_.end (), std::numeric_limits<IntType>::max ()};
		}

	private:
		R inner_;
	};

	template <typename IntType, typename R> Counted<R, IntType> counted (const R & r) { return {r}; }

	// TODO add type tags (Combinators namespace ?)

} // namespace Range
} // namespace duck
