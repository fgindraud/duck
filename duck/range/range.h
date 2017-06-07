#pragma once

// Top level range object with full functionnality
// STATUS: operational, needs added functionality

#include <duck/iterator/integer.h>
#include <duck/range/base.h>
#include <duck/range/combinator.h>
#include <iterator>
#include <type_traits>
#include <utility>

namespace duck {
namespace Range {
	template <typename It> class Range : public Base<It> {
		/* Represents a matching pair of iterators.
		 *
		 * This class has constant value semantics.
		 * It can not be modified, but new ranges can be built from it.
		 * It can be passed by value.
		 *
		 * This class has the same properties has the underlying iterator type.
		 * This applies for invalidation, and which operations can be used.
		 * Some operations may work inefficiently for low capability iterators (like size ()).
		 * Beware with less than forward iterators (risk of iterating on them in size()).
		 *
		 * TODO more slices (to_end, from_start).
		 * TODO checked operations ?
		 */
	public:
		using ValueType = typename std::iterator_traits<It>::value_type;
		using ReferenceType = typename std::iterator_traits<It>::reference;
		using DifferenceType = typename std::iterator_traits<It>::difference_type;
		using IteratorCategory = typename std::iterator_traits<It>::iterator_category;

		// Basic constructors
		constexpr Range (Base<It> base) noexcept : Base<It> (std::move (base)) {}

		using Base<It>::begin;
		using Base<It>::end;

		// Input/forward iterator
		constexpr bool empty () const { return begin () == end (); }
		constexpr ReferenceType front () const { return *begin (); }
		Range pop_front () const { return Base<It>{std::next (begin ()), end ()}; }

		// Bidirectional iterator
		ReferenceType back () const { return *std::prev (end ()); }
		Range pop_back () const { return Base<It>{begin (), std::prev (end ())}; }

		// Random access iterator
		DifferenceType size () const { return std::distance (begin (), end ()); }
		ReferenceType operator[] (DifferenceType n) const { return begin ()[n]; }
		Range pop_front (DifferenceType n) const { return Base<It>{std::next (begin (), n), end ()}; }
		Range pop_back (DifferenceType n) const { return Base<It>{begin (), std::prev (end (), n)}; }

		// Interval-like API
		constexpr bool contains (It it) const { return begin () <= it && it < end (); }
		DifferenceType offset_of (It it) const { return std::distance (begin (), it); }

		// "nicer" api (python like slice ; but at(size ()) return end ())
		// TODO improve...
		It at (DifferenceType n) const {
			auto index = n < 0 ? n + size () : n;
			return std::next (begin (), index);
		}
		Range slice (DifferenceType from, DifferenceType to) const {
			return Base<It>{at (from), at (to)};
		}
		Range slice_to (DifferenceType to) const { return Base<It>{begin (), at (to)}; }
		Range slice_from (DifferenceType from) const { return Base<It>{at (from), end ()}; }

		// Build a container from this range
		template <typename Container> Container to_container () const {
			return Container (begin (), end ());
		}

		// Combinators
		Range<std::reverse_iterator<It>> reverse () const { return reverse_base (*this); }
		template <typename UnaryPredicate>
		Range<Iterator::Filter<It, UnaryPredicate>> filter (UnaryPredicate p) const {
			return filter_base (*this, std::move (p));
		}
	};

	// Factory functions

	// From iterator pair (if it is considered an iterator by STL).
	template <typename It, typename = typename std::iterator_traits<It>::iterator_category>
	Range<It> range (It begin, It end) {
		return Base<It>{std::move (begin), std::move (end)};
	}

	// From container (enabled if it supports a begin() function and is lvalue).
	namespace Detail {
		// Utility function to extract the type returned by begin(), in the right context.
		using std::begin;
		template <typename T> auto call_begin (const T & t) -> decltype (begin (t));
		template <typename T> auto call_begin (T & t) -> decltype (begin (t));
	}
	template <typename Container,
	          typename It = decltype (Detail::call_begin (std::declval<Container> ())),
	          typename = typename std::enable_if<std::is_lvalue_reference<Container>::value>::type>
	Range<It> range (Container && container) {
		using std::begin;
		using std::end;
		return range (begin (container), end (container));
	}

	// From integers.
	template <typename Int, typename = typename std::enable_if<std::is_integral<Int>::value>::type>
	Range<Iterator::Integer<Int>> range (Int from, Int to) {
		return range (Iterator::Integer<Int>{from}, Iterator::Integer<Int>{to});
	}
	template <typename Int, typename = typename std::enable_if<std::is_integral<Int>::value>::type>
	Range<Iterator::Integer<Int>> range (Int to) {
		return range (Int{0}, to);
	}

	// Other factory functions
	template <typename Container, typename SizeType = decltype (std::declval<Container> ().size ())>
	Range<Iterator::Integer<SizeType>> index_range (const Container & container) {
		return range (container.size ());
	}
}
using Range::range;
}
