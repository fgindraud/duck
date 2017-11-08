#pragma once

// Range V2
// STATUS: WIP

#include <duck/range/range.h>
#include <limits>

namespace duck {
namespace Range {
	/* Ranges base combinators lib.
	 *
	 * TODO improve generation : use tags with operator| overloads ?
	 */

	/********************************************************************************
	 * Slicing. TODO
	 * Just pack lambdas that transform iterators ?
	 */

	/********************************************************************************
	 * Reverse order.
	 * Same rules as
	 */
	template <typename R> class Reversed;

	template <typename R> struct RangeTraits<Reversed<R>> {
		using Iterator = std::reverse_iterator<typename RangeTraits<R>::Iterator>;
		using SizeType = typename RangeTraits<R>::SizeType;
	};

	template <typename R> class Reversed : public Base<Reversed<R>> {
	public:
		using typename Base<Reversed<R>>::Iterator;
		using typename Base<Reversed<R>>::SizeType;

		constexpr Reversed (const R & r) : inner_ (r) {}
		constexpr Reversed (R && r) : inner_ (std::move (r)) {}

		constexpr Iterator begin () const { return Iterator{inner_.end ()}; }
		constexpr Iterator end () const { return Iterator{inner_.begin ()}; }
		constexpr SizeType size () const { return inner_.size (); }

	private:
		R inner_;
	};

	template <typename R> Reversed<R> reversed (const R & r) { return {r}; }

	/********************************************************************************
	 * Counted range.
	 *
	 * end() pointer index is UB.
	 * TODO finish, be careful of ref/pointers to temporaries
	 */
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

	template <typename R, typename IntType> class Counted;

	template <typename R, typename IntType> struct RangeTraits<Counted<R, IntType>> {
		using Iterator = CountedIterator<typename RangeTraits<R>::Iterator, IntType>;
		using SizeType = typename RangeTraits<R>::SizeType;
	};

	template <typename R, typename IntType> class Counted : public Base<Counted<R, IntType>> {
	public:
		using typename Base<Counted<R, IntType>>::Iterator;
		using typename Base<Counted<R, IntType>>::SizeType;

		constexpr Counted (const R & r) : inner_ (r) {}
		constexpr Counted (R && r) : inner_ (std::move (r)) {}

		constexpr Iterator begin () const { return {inner_.begin (), 0}; }
		constexpr Iterator end () const {
			return {inner_.end (), std::numeric_limits<IntType>::max ()};
		}
		constexpr SizeType size () const { return inner_.size (); }

	private:
		R inner_;
	};

	template <typename IntType, typename R> Counted<R, IntType> counted (const R & r) { return {r}; }
} // namespace Range
} // namespace duck
