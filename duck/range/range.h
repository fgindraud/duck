#pragma once

// Top level range object with full functionnality

#include <duck/iterator/integer.h>
#include <duck/iterator/traits.h>
#include <duck/range/base.h>

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
		 *
		 * Less than forward iterators are not supported properly FIXME.
		 *
		 * TODO more slices (to_end, from_start).
		 * TODO checked operations ?
		 */
	public:
		using ValueType = Iterator::GetValueType<It>;
		using ReferenceType = Iterator::GetReferenceType<It>;
		using DifferenceType = Iterator::GetDifferenceType<It>;
		using IteratorCategory = Iterator::GetCategory<It>;

		// Basic constructors
		constexpr Range (Base<It> base) noexcept : Base<It> (std::move (base)) {}

		// TODO may add constructors from container like things, to enable auto conversion to range.

		using Base<It>::begin;
		using Base<It>::end;

		// Input/forward iterator
		bool empty () const { return begin () == end (); }
		ReferenceType front () const { return *begin (); }
		Range pop_front () const { return Base<It>{std::next (begin (), end ())}; }

		// Bidirectional iterator
		ReferenceType back () const { return *std::prev (end ()); }
		Range pop_back () const { return Base<It>{begin (), std::prev (end ())}; }

		// Random access iterator
		DifferenceType size () const { return std::distance (begin (), end ()); }
		ReferenceType operator[] (DifferenceType n) const { return begin ()[n]; }
		Range pop_front (DifferenceType n) const { return Base<It>{std::next (begin (), n), end ()}; }
		Range pop_back (DifferenceType n) const { return Base<It>{begin (), std::prev (end (), n)}; }

		// TODO additional API
		bool contains (It it) const { return begin () <= it && it < end (); }
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

		template <typename Container> Container to_container () const {
			return Container (begin (), end ());
		}
	};
}
// Factory functions

// From iterator pair (if it is considered an iterator by STL).
template <typename It, typename = Iterator::GetCategory<It>>
Range::Range<It> range (It begin, It end) {
	return Range::Base<It>{std::move (begin), std::move (end)};
}

// From container (enabled if it supports a begin() function).
template <typename Container, typename It = Iterator::GetContainerIteratorType<Container &>>
Range::Range<It> range (Container & container) {
	using std::begin;
	using std::end;
	return range (begin (container), end (container));
}
template <typename Container, typename It = Iterator::GetContainerIteratorType<const Container &>>
Range::Range<It> range (const Container & container) {
	using std::begin;
	using std::end;
	return range (begin (container), end (container));
}

// From integers.
template <typename Int, typename = typename std::enable_if<std::is_integral<Int>::value>::type>
Range::Range<Iterator::Integer<Int>> range (Int from, Int to) {
	return range (Iterator::Integer<Int>{from}, Iterator::Integer<Int>{to});
}
template <typename Int, typename = typename std::enable_if<std::is_integral<Int>::value>::type>
Range::Range<Iterator::Integer<Int>> range (Int to) {
	return range (Int{0}, to);
}

// Other factory functions
template <typename Container, typename SizeType = decltype (std::declval<Container> ().size ())>
Range::Range<Iterator::Integer<SizeType>> index_range (const Container & container) {
	return range (container.size ());
}
}
