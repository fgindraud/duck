#pragma once

// Top level range object with full functionnality

#include <duck/iterator.h>
#include <duck/range/base.h>
#include <iterator>
#include <type_traits>

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
		 * Beware with input iterators, as some read-only operations (like size()) might iterate on
		 * them.
		 * This will "invalidate all iterator copies", which can be bad and is difficult to check.
		 * It is better to use this class starting at forward iterators.
		 * Output only iterator are even more dangerous (size() might be UB).
		 *
		 * TODO more slices (to_end, from_start).
		 * TODO checked operations ?
		 */
	public:
		using ValueType = Iterator::GetValueType<It>;
		using ReferenceType = Iterator::GetReferenceType<It>;
		using DifferenceType = Iterator::GetDifferenceType<It>;
		using IteratorCategory = Iterator::GetCategory<It>;

		// Basic constructor
		constexpr Range (Base<It> base) noexcept : Base<It> (base) {}

		// TODO from template it is convertible
		// from container if it/const_it
		Range (It begin, It end) : Base<It> (begin, end) {}
		template <typename Container, typename = std::enable_if_t<
		                                  std::is_convertible<typename Container::iterator, It>::value>>
		Range (Container & container) : Range (container.begin (), container.end ()) {}
		template <typename Container, typename = std::enable_if_t<std::is_convertible<
		                                  typename Container::const_iterator, It>::value>>
		Range (const Container & container) : Range (container.begin (), container.end ()) {}

		// modify and return *this ref for chaining.
		// Input/forward iterator
		bool empty () const { return empty (*this); }
		ReferenceType front () const { return front (*this); }
		Range pop_front () const { return pop_front (*this); }

		// Bidirectional iterator
		ReferenceType back () const { return back (*this); }
		Range pop_back () const { return pop_back (*this); }

		// Random access iterator
		DifferenceType size () const { return size (*this); }
		ReferenceType operator[] (std::size_t n) const { return value_at (*this, n); }
		Range pop_front (std::size_t n) const { return pop_front (*this, n) }
		Range pop_back (DifferenceType n) const { return pop_back (*this, n); }

		// TODO additional API 
		bool contains (It it) const { return begin () <= it && it < end (); }
		DifferenceType offset_of (It it) const { return std::distance (begin (), it); }

		// "nicer" api (python like slice ; but at(size ()) return end ())
		It at (DifferenceType n) const {
			auto index = n < 0 ? n + size () : n;
			return Iterator::advance (begin (), index);
		}
		Range slice (DifferenceType from, DifferenceType to) const { return {at (from), at (to)}; }

		template <typename Container> Container to_container () const {
			return Container (begin (), end ());
		}
	};
}

// forward to Range constructor TODO

// Factory function from pair of iterators (enabled if it defines a category).
template <typename It, typename = Iterator::GetCategory<It>> auto range (It first, It last) {
	return {first, last};
}

// Factory functions for containers (enabled if Container defines the proper iterators)
template <typename Container, typename It = typename Container::iterator>
auto range (Container & container) {
	return Range<It>{container.begin (), container.end ()};
}
template <typename Container, typename It = typename Container::const_iterator>
auto range (const Container & container) {
	return Range<It>{container.begin (), container.end ()};
}
}
