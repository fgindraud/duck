#pragma once

// Range V2
// STATUS: WIP

#include <algorithm>
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
	 * Reverse order.
	 */
	template <typename R> class Reversed;

	template <typename R> struct RangeTraits<Reversed<R>> {
		using Iterator = std::reverse_iterator<typename RangeTraits<R>::Iterator>;
		using SizeType = typename RangeTraits<R>::SizeType;
	};

	template <typename R> class Reversed : public Base<Reversed<R>> {
		static_assert (IsRange<R>::value, "Reversed<R>: R must be a range");

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

	namespace Combinator {
		template <typename R> Reversed<R> reversed (const R & r) { return {r}; }

		struct ReversedTag {};
		inline ReversedTag reversed () { return {}; }
		template <typename R> auto operator| (const R & r, ReversedTag) -> decltype (reversed (r)) {
			return reversed (r);
		}
	} // namespace Combinator

	/********************************************************************************
	 * Filter with a predicate.
	 */
	template <typename R, typename Predicate> class FilterIterator;
	template <typename R, typename Predicate> class Filter;

	template <typename R, typename Predicate> struct RangeTraits<Filter<R, Predicate>> {
		using Iterator = FilterIterator<R, Predicate>;
		using SizeType = typename std::iterator_traits<Iterator>::difference_type;
	};

	template <typename R, typename Predicate> class Filter : public Base<Filter<R, Predicate>> {
		static_assert (IsRange<R>::value, "Filter<R, Predicate>: R must be a range");
		// TODO predicate is invocable on value

	public:
		using typename Base<Filter<R, Predicate>>::Iterator;

		constexpr Filter (const R & r, const Predicate & predicate)
		    : inner_ (r), predicate_ (predicate) {}

		Iterator begin () const;
		Iterator end () const;

	private:
		friend class FilterIterator<R, Predicate>;

		using InnerIterator = typename RangeTraits<R>::Iterator;
		InnerIterator next (InnerIterator from) const {
			return std::find_if (from, inner_.end (), predicate_);
		}
		InnerIterator next_after (InnerIterator from) const {
			if (from == inner_.end ())
				return from;
			else
				return next (++from);
		}
		InnerIterator previous_before (InnerIterator from) const {
			if (from == inner_.begin ()) {
				return from;
			} else {
				// Try to find a match before from
				// If this fails, from was first, return it
				using RevIt = std::reverse_iterator<InnerIterator>;
				auto rev_end = RevIt{inner_.begin ()};
				auto rev_next = std::find_if (RevIt{--from}, rev_end, predicate_);
				if (rev_next != rev_end)
					return rev_next.base ();
				else
					return from;
			}
		}

		R inner_;
		Predicate predicate_;
	};

	template <typename R, typename Predicate> class FilterIterator {
		static_assert (IsRange<R>::value, "FilterIterator<R, Predicate>: R must be a range");

	public:
		using InnerIterator = typename RangeTraits<R>::Iterator;

		// At most this is a bidirectional_iterator
		using iterator_category = typename std::common_type<
		    std::bidirectional_iterator_tag,
		    typename std::iterator_traits<InnerIterator>::iterator_category>::type;
		using value_type = typename std::iterator_traits<InnerIterator>::value_type;
		using difference_type = typename std::iterator_traits<InnerIterator>::difference_type;
		using pointer = typename std::iterator_traits<InnerIterator>::pointer;
		using reference = typename std::iterator_traits<InnerIterator>::reference;

		constexpr FilterIterator () = default;
		constexpr FilterIterator (InnerIterator it, const Filter<R, Predicate> & range)
		    : it_ (it), range_ (&range) {}

		// Input / output
		FilterIterator & operator++ () { return it_ = range_->next_after (it_), *this; }
		constexpr reference operator* () const { return *it_; }
		constexpr pointer operator-> () const { return it_.operator-> (); }
		constexpr bool operator== (const FilterIterator & o) const { return it_ == o.it_; }
		constexpr bool operator!= (const FilterIterator & o) const { return it_ != o.it_; }

		// Forward
		FilterIterator operator++ (int) {
			FilterIterator tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir
		FilterIterator & operator-- () { return it_ = range_->previous_before (it_), *this; }
		FilterIterator operator-- (int) {
			FilterIterator tmp (*this);
			--*this;
			return tmp;
		}

	private:
		InnerIterator it_{};
		const Filter<R, Predicate> * range_{nullptr};
	};

	template <typename R, typename Predicate> auto Filter<R, Predicate>::begin () const -> Iterator {
		return {next (inner_.begin ()), *this};
	}
	template <typename R, typename Predicate> auto Filter<R, Predicate>::end () const -> Iterator {
		return {inner_.end (), *this};
	}

	namespace Combinator {
		template <typename R, typename Predicate>
		Filter<R, Predicate> filter (const R & r, const Predicate & predicate) {
			return {r, predicate};
		}

		template <typename Predicate> struct FilterTag { Predicate predicate; };
		template <typename Predicate> FilterTag<Predicate> filter (const Predicate & predicate) {
			return {predicate};
		}
		template <typename R, typename Predicate>
		auto operator| (const R & r, const FilterTag<Predicate> & tag)
		    -> decltype (filter (r, tag.predicate)) {
			return filter (r, tag.predicate);
		}
	} // namespace Combinator

	/********************************************************************************
	 * Counted range.
	 * Returned value_type has an index field, and a value() member.
	 *
	 * end() pointer index is UB.
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
		static_assert (IsRange<R>::value, "Counted<R, IntType>: R must be a range");
		static_assert (std::is_integral<IntType>::value,
		               "Counted<R, IntType>: IntType must be an integral type");

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

	namespace Combinator {
		template <typename IntType, typename R> Counted<R, IntType> counted (const R & r) {
			return {r};
		}

		template <typename IntType> struct CountedTag {};
		template <typename IntType> CountedTag<IntType> counted () { return {}; }
		template <typename IntType, typename R>
		auto operator| (const R & r, CountedTag<IntType>) -> decltype (counted<IntType> (r)) {
			return counted<IntType> (r);
		}
	} // namespace Combinator
} // namespace Range
} // namespace duck
