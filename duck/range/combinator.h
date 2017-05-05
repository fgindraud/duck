#pragma once

// Defines combinator iterators, and factory functions to use them in ranges.

#include <duck/iterator/traits.h>
#include <duck/range/base.h>
#include <utility>

namespace duck {

namespace Range {
	// Reverse a range
	template <typename It> Base<std::reverse_iterator<It>> reverse_base (Base<It> r) {
		return {std::reverse_iterator<It>{r.end ()}, std::reverse_iterator<It>{r.begin ()}};
	}
}

namespace Iterator {
	template <typename It, typename UnaryPredicate> class Filter : private UnaryPredicate {
		/* Iterate only on elements where UnaryPredicate is true (skip the others).
		 * UnaryPredicate is EBO-used to keep the iterator small (must be an object type !).
		 * Result is a forward iterator only.
		 */
	public:
		using iterator_category = RestrictToCategory<It, std::forward_iterator_tag>;
		using value_type = GetValueType<It>;
		using difference_type = GetDifferenceType<It>;
		using pointer = GetPointerType<It>;
		using reference = GetReferenceType<It>;

		// Constructors
		constexpr Filter (It end, UnaryPredicate p) noexcept
		    : UnaryPredicate (std::move (p)), current_ (end), end_ (std::move (end)) {}
		Filter (It current, It end, UnaryPredicate p)
		    : UnaryPredicate (std::move (p)), current_ (std::move (current)), end_ (std::move (end)) {
			advance_to_next (); // Start at a selected element.
		}

		Filter (const Filter &) = default;
		Filter (Filter &&) = default;
		Filter & operator= (const Filter & f) {
			// FIXME This is an okay-ish copy assignement operator.
			// lambda UnaryPredicate cannot be copied, but doesn't need to be copied
			// in case of func pointer, this will fail if iterators from different ranges are used.
			// (do not copy the right func pointer, do not copy end)...
			current_ = f.current_;
			return *this;
		}

		// Access
		constexpr const It & current () const { return current_; }
		constexpr const It & end () const { return end_; }
		constexpr const UnaryPredicate & predicate () const { return *this; }

		Filter & operator++ () {
			if (current_ != end_) {
				++current_;
				advance_to_next ();
			}
			return *this;
		}
		Filter operator++ (int) {
			Filter tmp (*this);
			++*this;
			return tmp;
		}
		reference operator* () { return *current_; }
		pointer operator-> () { return current_.operator-> (); }
		bool operator== (const Filter & rhs) const { return current_ == rhs.current_; }
		bool operator!= (const Filter & rhs) const { return current_ != rhs.current_; }

	private:
		void advance_to_next () {
			while (current_ != end_ && !UnaryPredicate::operator() (*current_))
				current_++;
		}

		It current_;
		const It end_;
	};
}

namespace Range {
	// Specialisation of Base to have smaller footprint
	template <typename It, typename UnaryPredicate>
	class Base<Iterator::Filter<It, UnaryPredicate>> : private UnaryPredicate {
	private:
		using FilterIt = Iterator::Filter<It, UnaryPredicate>;

	public:
		// Constructors (additional optimized constructor)
		constexpr Base (Base<It> r, UnaryPredicate p) noexcept
		    : UnaryPredicate (std::move (p)), base (std::move (r)) {}
		constexpr Base (FilterIt begin, FilterIt) noexcept
		    : UnaryPredicate (begin.predicate ()), base (begin.current (), begin.end ()) {}

		// Iterator access
		constexpr FilterIt begin () const { return FilterIt{base.begin (), base.end (), *this}; }
		constexpr FilterIt end () const { return FilterIt{base.end (), *this}; }

	private:
		Base<It> base;
	};

	template <typename It, typename UnaryPredicate>
	Base<Iterator::Filter<It, UnaryPredicate>> filter_base (Base<It> r, UnaryPredicate p) {
		return {std::move (r), std::move (p)};
	}
}

// TODO add transform/apply (on one range only)
// TODO zip ?
}
