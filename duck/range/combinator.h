#pragma once

// Defines combinator iterators, and factory functions to use them in ranges.

#include <algorithm>
#include <duck/iterator.h>
#include <duck/range.h>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

namespace duck {

	// TODO always manipulate base
	// TODO use iterator helper classes
	// TODO iterator helper classes : define a selector

// Factory function to reverse a range.
template <typename It,
          typename = Iterator::EnableIfHasCategory<std::bidirectional_iterator_tag, It>>
auto reverse_range (Range<It> r) {
	using RevIt = std::reverse_iterator<It>;
	return range (RevIt{r.end ()}, RevIt{r.begin ()});
}

namespace Iterator {
	template <typename It, typename UnaryPredicate> class Filter : private UnaryPredicate {
		/* Iterate only on elements where UnaryPredicate is true (skip the others).
		 * UnaryPredicate is EBO-used to keep the iterator small (must be an object type !).
		 * Result is a forward iterator only.
		 * TODO Bidir ?
		 */
	public:
		using iterator_category = std::common_type_t<GetCategory<It>, std::forward_iterator_tag>;
		using value_type = GetValueType<It>;
		using difference_type = GetDifferenceType<It>;
		using pointer = GetPointerType<It>;
		using reference = GetReferenceType<It>;

		// iterator
		Filter () = default;
		Filter (It end, UnaryPredicate p) : UnaryPredicate (p), it_ (end), end_ (end) {}
		Filter (It it, It end, UnaryPredicate p) : UnaryPredicate (p), it_ (it), end_ (end) {
			it_ = next_from (it_);
		}
		// + default ctors
		void swap (Filter & it) {
			std::swap (static_cast<UnaryPredicate &> (*this), static_cast<UnaryPredicate &> (it));
			std::swap (it_, it.it_);
		}
		reference operator* () { return *it_; }
		Filter & operator++ () {
			if (it_ != end_)
				it_ = next_from (std::next (it_));
			return *this;
		}

		// input/forward iterator
		bool operator== (const Filter & rhs) const { return it_ == rhs.it_; }
		bool operator!= (const Filter & rhs) const { return it_ != rhs.it_; }
		//TODO continue

		// random access iterator (partial)
		bool operator< (const Filter & rhs) const { return it_ < rhs.it_; }
		bool operator> (const Filter & rhs) const { return it_ > rhs.it_; }
		bool operator<= (const Filter & rhs) const { return it_ <= rhs.it_; }
		bool operator>= (const Filter & rhs) const { return it_ >= rhs.it_; }

	private:
		It next_from (It it) const {
			return std::find_if (it, end_, static_cast<const UnaryPredicate &> (*this));
		}

		It it_;
		const It end_;
	};

	// Out of class functions for Filter
	template <typename It, typename UnaryPredicate>
	inline void swap (Filter<It, UnaryPredicate> & lhs, Filter<It, UnaryPredicate> & rhs) {
		lhs.swap (rhs);
	}
}

// Factory function to make a filter range.
template <typename It, typename UnaryPredicate> auto filter_range (Range<It> r, UnaryPredicate p) {
	using FilterIt = Iterator::Filter<It, UnaryPredicate>;
	return range (FilterIt{r.begin (), r.end (), p}, FilterIt{r.end (), p});
}

namespace Iterator {
	template <typename Function, typename... Iters> class Apply : private Function {
		/* Iterator type that returns the result of a function over a set of iterators.
		 * Function is EBO-used to keep the iterator small (must be an object type !).
		 *
		 * Using this as output iterator is possible... if Function returns a reference.
		 * Using the pointer is asking for troubles... (pointer to temporary object).
		 */
	public:
		using iterator_category = std::common_type_t<GetCategory<Iters>...>;
		using value_type = std::result_of_t<Function (GetValueType<Iters>...)>;
		using difference_type = std::ptrdiff_t;
		using pointer = void;         // Pointer to temporary object is a bad idea.
		using reference = value_type; // Is actually a reference if Function returns a reference.

		// iterator
		Apply () = default;
		Apply (Function f, Iters... iterators) : Function (f), iterators_ (iterators...) {}
		// + other default constructors
		void swap (Apply & it) noexcept {
			std::swap (static_cast<Function &> (*this), static_cast<Function &> (it));
			std::swap (iterators_, it.iterators_);
		}
		reference operator* () const {
			return apply_to_tuple (
			    [this](Iters... iterators) { return Function::operator() (*iterators...); },
			    std::index_sequence_for<Iters...> ());
		}
		Apply & operator++ () {
			apply_to_tuple (
			    [this](Iters &... iterators) { (void) std::tuple<Iters...> (++iterators...); },
			    std::index_sequence_for<Iters...> ());
			return *this;
		}

		// input/forward iterator
		bool operator== (const Apply & rhs) const { return false; }
		// TODO problem of end()
		// comps should be an && of piecewise comps
		// to stop at end if ranges do not match: special end() comp...
		// better to have a normalizer combinator to handle this case before hand ?

	private:
		template <typename Callable, std::size_t... I>
		auto apply_to_tuple (Callable && c, std::index_sequence<I...>) const {
			return std::forward<Callable> (c) (std::get<I> (iterators_)...);
		}
		template <typename Callable, std::size_t... I>
		auto apply_to_tuple (Callable && c, std::index_sequence<I...>) {
			return std::forward<Callable> (c) (std::get<I> (iterators_)...);
		}

		std::tuple<Iters...> iterators_;
	};

	// Out of class functions for Apply
	template <typename Function, typename... Iters>
	inline void swap (Apply<Function, Iters...> & lhs, Apply<Function, Iters...> & rhs) {
		lhs.swap (rhs);
	}
}

template <typename Function, typename... Iters>
Range<Iterator::Apply<Function, Iters...>> apply_range (Function f, Range<Iters>... ranges) {
	return {{f, ranges.begin ()...}, {f, ranges.end ()...}};
}
}
