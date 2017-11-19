#pragma once

// Range V2
// STATUS: WIP

#include <algorithm>
#include <duck/range/range_v2.h>
#include <limits>

namespace duck {
namespace Range {
	/* Ranges base combinators lib.
	 */

	/********************************************************************************
	 * Type traits.
	 */

	// Test if we at least support RequiredCategory
	template <typename Category, typename RequiredCategory>
	using HasRequiredCategory = std::is_base_of<RequiredCategory, Category>;
	template <typename It, typename RequiredCategory>
	using IteratorHasRequiredCategory =
	    HasRequiredCategory<typename std::iterator_traits<It>::iterator_category, RequiredCategory>;
	template <typename R, typename RequiredCategory>
	using RangeHasRequiredCategory =
	    IteratorHasRequiredCategory<typename RangeTraits<R>::Iterator, RequiredCategory>;

	// Is callable
	template <typename Callable, typename Arg, typename = void>
	struct IsCallable : std::false_type {};
	template <typename Callable, typename Arg>
	struct IsCallable<Callable, Arg,
	                  void_t<decltype (std::declval<Callable &> () (std::declval<Arg> ()))>>
	    : std::true_type {};

	// Is predicate
	template <typename Predicate, typename Arg, typename = void>
	struct IsPredicate : std::false_type {};
	template <typename Predicate, typename Arg>
	struct IsPredicate<
	    Predicate, Arg,
	    void_t<decltype (static_cast<bool> (std::declval<Predicate &> () (std::declval<Arg> ())))>>
	    : std::true_type {};

	/********************************************************************************
	 * Slicing. TODO
	 * Just pack lambdas that transform iterators ?
	 */

	/********************************************************************************
	 * Reverse order.
	 */
	template <typename R> class Reverse;

	template <typename R> struct RangeTraits<Reverse<R>> {
		using Iterator = std::reverse_iterator<typename RangeTraits<R>::Iterator>;
		using SizeType = typename RangeTraits<R>::SizeType;
	};

	template <typename R> class Reverse : public Base<Reverse<R>> {
		static_assert (IsRange<R>::value, "Reverse<R>: R must be a range");
		static_assert (RangeHasRequiredCategory<R, std::bidirectional_iterator_tag>::value,
		               "Reverse<R>: R must be at least bidirectional_iterator");

	public:
		using typename Base<Reverse<R>>::Iterator;
		using typename Base<Reverse<R>>::SizeType;

		Reverse (const R & r) : inner_ (r) {}
		Reverse (R && r) : inner_ (std::move (r)) {}

		Iterator begin () const { return Iterator{inner_.end ()}; }
		Iterator end () const { return Iterator{inner_.begin ()}; }
		SizeType size () const { return inner_.size (); }

	private:
		R inner_;
	};

	namespace Combinator {
		template <typename R> Reverse<decay_t<R>> reverse (R && r) { return {std::forward<R> (r)}; }

		struct ReverseTag {};
		inline ReverseTag reverse () { return {}; }
		template <typename R>
		auto operator| (R && r, ReverseTag) -> decltype (reverse (std::forward<R> (r))) {
			return reverse (std::forward<R> (r));
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
		static_assert (IsPredicate<Predicate, typename std::iterator_traits<
		                                          typename RangeTraits<R>::Iterator>::reference>::value,
		               "Filter<R, Predicate>: Predicate must be callable on R values");

	public:
		using typename Base<Filter<R, Predicate>>::Iterator;

		Filter (const R & r, const Predicate & predicate) : inner_ (r), predicate_ (predicate) {}
		Filter (const R & r, Predicate && predicate) : inner_ (r), predicate_ (std::move (predicate)) {}
		Filter (R && r, const Predicate & predicate) : inner_ (std::move (r)), predicate_ (predicate) {}
		Filter (R && r, Predicate && predicate)
		    : inner_ (std::move (r)), predicate_ (std::move (predicate)) {}

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
		static_assert (IsPredicate<Predicate, typename std::iterator_traits<
		                                          typename RangeTraits<R>::Iterator>::reference>::value,
		               "Filter<R, Predicate>: Predicate must be callable on R values");

	public:
		using InnerIterator = typename RangeTraits<R>::Iterator;

		// At most this is a bidirectional_iterator
		using iterator_category =
		    common_type_t<std::bidirectional_iterator_tag,
		                  typename std::iterator_traits<InnerIterator>::iterator_category>;
		using value_type = typename std::iterator_traits<InnerIterator>::value_type;
		using difference_type = typename std::iterator_traits<InnerIterator>::difference_type;
		using pointer = typename std::iterator_traits<InnerIterator>::pointer;
		using reference = typename std::iterator_traits<InnerIterator>::reference;

		FilterIterator () = default;
		FilterIterator (InnerIterator it, const Filter<R, Predicate> & range)
		    : it_ (it), range_ (&range) {}

		InnerIterator base () const { return it_; }

		// Input / output
		FilterIterator & operator++ () { return it_ = range_->next_after (it_), *this; }
		reference operator* () const { return *it_; }
		pointer operator-> () const { return it_.operator-> (); }
		bool operator== (const FilterIterator & o) const { return it_ == o.it_; }
		bool operator!= (const FilterIterator & o) const { return it_ != o.it_; }

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
		Filter<decay_t<R>, decay_t<Predicate>> filter (R && r, Predicate && predicate) {
			return {std::forward<R> (r), std::forward<Predicate> (predicate)};
		}

		template <typename Predicate> struct FilterTag {
			FilterTag (const Predicate & p) : predicate (p) {}
			FilterTag (Predicate && p) : predicate (std::move (p)) {}
			Predicate predicate;
		};
		template <typename Predicate> FilterTag<decay_t<Predicate>> filter (Predicate && predicate) {
			return {std::forward<Predicate> (predicate)};
		}
		template <typename R, typename Predicate>
		auto operator| (R && r, FilterTag<Predicate> tag)
		    -> decltype (filter (std::forward<R> (r), std::move (tag.predicate))) {
			return filter (std::forward<R> (r), std::move (tag.predicate));
		}
	} // namespace Combinator

	/********************************************************************************
	 * Processed range.
	 * Apply function f to each element.
	 */
	template <typename R, typename Function> class ApplyIterator;
	template <typename R, typename Function> class Apply;

	template <typename R, typename Function> struct RangeTraits<Apply<R, Function>> {
		using Iterator = ApplyIterator<R, Function>;
		using SizeType = typename RangeTraits<R>::SizeType;
	};

	template <typename R, typename Function> class Apply : public Base<Apply<R, Function>> {
		static_assert (IsRange<R>::value, "Apply<R, Function>: R must be a range");
		static_assert (IsCallable<Function, typename std::iterator_traits<
		                                        typename RangeTraits<R>::Iterator>::reference>::value,
		               "Apply<R, Function>: Function must be callable on R values");

	public:
		using typename Base<Apply<R, Function>>::Iterator;
		using typename Base<Apply<R, Function>>::SizeType;

		Apply (const R & r, const Function & function) : inner_ (r), function_ (function) {}
		Apply (const R & r, Function && function) : inner_ (r), function_ (std::move (function)) {}
		Apply (R && r, const Function & function) : inner_ (std::move (r)), function_ (function) {}
		Apply (R && r, Function && function)
		    : inner_ (std::move (r)), function_ (std::move (function)) {}

		Iterator begin () const;
		Iterator end () const;
		SizeType size () const { return inner_.size (); }

	private:
		friend class ApplyIterator<R, Function>;
		R inner_;
		Function function_;
	};

	template <typename R, typename Function> class ApplyIterator {
		static_assert (IsRange<R>::value, "ApplyIterator<R, Function>: R must be a range");
		static_assert (IsCallable<Function, typename std::iterator_traits<
		                                        typename RangeTraits<R>::Iterator>::reference>::value,
		               "ApplyIterator<R, Function>: Function must be callable on R values");

	public:
		using InnerIterator = typename RangeTraits<R>::Iterator;

		using iterator_category = typename std::iterator_traits<InnerIterator>::iterator_category;
		using value_type = decltype (std::declval<const Function &> () (
		    std::declval<typename std::iterator_traits<InnerIterator>::reference> ()));
		using difference_type = typename std::iterator_traits<InnerIterator>::difference_type;
		using pointer = void;
		using reference = value_type; // No way to take references on function_ result

		ApplyIterator () = default;
		ApplyIterator (InnerIterator it, const Apply<R, Function> & range)
		    : it_ (it), range_ (&range) {}

		InnerIterator base () const { return it_; }

		// Input / output
		ApplyIterator & operator++ () { return ++it_, *this; }
		reference operator* () const { return range_->function_ (*it_); }
		// No operator->
		bool operator== (const ApplyIterator & o) const { return it_ == o.it_; }
		bool operator!= (const ApplyIterator & o) const { return it_ != o.it_; }

		// Forward
		ApplyIterator operator++ (int) {
			ApplyIterator tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir
		ApplyIterator & operator-- () { return --it_, *this; }
		ApplyIterator operator-- (int) {
			ApplyIterator tmp (*this);
			--*this;
			return tmp;
		}

		// Random access
		ApplyIterator & operator+= (difference_type n) { return it_ += n, *this; }
		ApplyIterator operator+ (difference_type n) const { return ApplyIterator (it_ + n, range_); }
		friend ApplyIterator operator+ (difference_type n, const ApplyIterator & it) { return it + n; }
		ApplyIterator & operator-= (difference_type n) { return it_ -= n, *this; }
		ApplyIterator operator- (difference_type n) const { return ApplyIterator (it_ - n, range_); }
		difference_type operator- (const ApplyIterator & o) const { return it_ - o.it_; }
		reference operator[] (difference_type n) const { return *(*this + n); }
		bool operator< (const ApplyIterator & o) const { return it_ < o.it_; }
		bool operator> (const ApplyIterator & o) const { return it_ > o.it_; }
		bool operator<= (const ApplyIterator & o) const { return it_ <= o.it_; }
		bool operator>= (const ApplyIterator & o) const { return it_ >= o.it_; }

	private:
		InnerIterator it_{};
		const Apply<R, Function> * range_{nullptr};
	};

	template <typename R, typename Function> auto Apply<R, Function>::begin () const -> Iterator {
		return {inner_.begin (), *this};
	}
	template <typename R, typename Function> auto Apply<R, Function>::end () const -> Iterator {
		return {inner_.end (), *this};
	}

	namespace Combinator {
		template <typename R, typename Function>
		Apply<decay_t<R>, decay_t<Function>> apply (R && r, Function && function) {
			return {std::forward<R> (r), std::forward<Function> (function)};
		}

		template <typename Function> struct ApplyTag {
			ApplyTag (const Function & p) : function (p) {}
			ApplyTag (Function && p) : function (std::move (p)) {}
			Function function;
		};
		template <typename Function> ApplyTag<decay_t<Function>> apply (Function && function) {
			return {std::forward<Function> (function)};
		}
		template <typename R, typename Function>
		auto operator| (R && r, ApplyTag<Function> tag)
		    -> decltype (apply (std::forward<R> (r), std::move (tag.function))) {
			return apply (std::forward<R> (r), std::move (tag.function));
		}
	} // namespace Combinator

	/********************************************************************************
	 * Indexed range.
	 * Returned value_type has an index field, and a value() member.
	 * end() iterator index is UB.
	 */
	template <typename It, typename IntType> class IndexIterator {
		static_assert (IsIterator<It>::value,
		               "IndexIterator<It, IntType>: It must be an iterator type");
		static_assert (std::is_integral<IntType>::value,
		               "IndexIterator<It, IntType>: IntType must be integral");

	public:
		using iterator_category = typename std::iterator_traits<It>::iterator_category;
		struct value_type {
			It it;
			IntType index;
			value_type () = default;
			value_type (It it_arg, IntType index_arg) : it (it_arg), index (index_arg) {}
			typename std::iterator_traits<It>::reference value () const { return *it; }
		};
		using difference_type = typename std::iterator_traits<It>::difference_type;
		using pointer = const value_type *;
		using reference = const value_type &;

		IndexIterator () = default;
		IndexIterator (It it, IntType index) : d_ (it, index) {}

		It base () const { return d_.it; }

		// Input / output
		IndexIterator & operator++ () { return ++d_.it, ++d_.index, *this; }
		reference operator* () const { return d_; }
		pointer operator-> () const { return &d_; }
		bool operator== (const IndexIterator & o) const { return d_.it == o.d_.it; }
		bool operator!= (const IndexIterator & o) const { return d_.it != o.d_.it; }

		// Forward
		IndexIterator operator++ (int) {
			IndexIterator tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir
		IndexIterator & operator-- () { return --d_.it, --d_.index, *this; }
		IndexIterator operator-- (int) {
			IndexIterator tmp (*this);
			--*this;
			return tmp;
		}

		// Random access
		IndexIterator & operator+= (difference_type n) { return d_.it += n, d_.index += n, *this; }
		IndexIterator operator+ (difference_type n) const {
			return IndexIterator (d_.it + n, d_.index + n);
		}
		friend IndexIterator operator+ (difference_type n, const IndexIterator & it) { return it + n; }
		IndexIterator & operator-= (difference_type n) { return d_.it -= n, d_.index -= n, *this; }
		IndexIterator operator- (difference_type n) const {
			return IndexIterator (d_.it - n, d_.index - n);
		}
		difference_type operator- (const IndexIterator & o) const { return d_.it - o.d_.it; }
		value_type operator[] (difference_type n) const { return *(*this + n); }
		bool operator< (const IndexIterator & o) const { return d_.it < o.d_.it; }
		bool operator> (const IndexIterator & o) const { return d_.it > o.d_.it; }
		bool operator<= (const IndexIterator & o) const { return d_.it <= o.d_.it; }
		bool operator>= (const IndexIterator & o) const { return d_.it >= o.d_.it; }

	private:
		value_type d_{};
	};

	template <typename R, typename IntType> class Index;

	template <typename R, typename IntType> struct RangeTraits<Index<R, IntType>> {
		using Iterator = IndexIterator<typename RangeTraits<R>::Iterator, IntType>;
		using SizeType = typename RangeTraits<R>::SizeType;
	};

	template <typename R, typename IntType> class Index : public Base<Index<R, IntType>> {
		static_assert (IsRange<R>::value, "Index<R, IntType>: R must be a range");
		static_assert (std::is_integral<IntType>::value,
		               "Index<R, IntType>: IntType must be an integral type");

	public:
		using typename Base<Index<R, IntType>>::Iterator;
		using typename Base<Index<R, IntType>>::SizeType;

		Index (const R & r) : inner_ (r) {}
		Index (R && r) : inner_ (std::move (r)) {}

		Iterator begin () const { return {inner_.begin (), 0}; }
		Iterator end () const { return {inner_.end (), std::numeric_limits<IntType>::max ()}; }
		SizeType size () const { return inner_.size (); }

	private:
		R inner_;
	};

	namespace Combinator {
		template <typename IntType, typename R> Index<decay_t<R>, IntType> index (R && r) {
			return {std::forward<R> (r)};
		}

		template <typename IntType> struct IndexTag {};
		template <typename IntType = int> IndexTag<IntType> index () { return {}; }
		template <typename IntType, typename R>
		auto operator| (R && r, IndexTag<IntType>) -> decltype (index<IntType> (std::forward<R> (r))) {
			return index<IntType> (std::forward<R> (r));
		}
	} // namespace Combinator
} // namespace Range
} // namespace duck
