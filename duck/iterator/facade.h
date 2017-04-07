#pragma once

// Iterator builder classes

// TODO Partially finished
// Lacking element access
// Other problem, traits definition...

#include <duck/iterator/traits.h>
#include <iterator>
#include <type_traits>
#include <utility>

namespace duck {
namespace Iterator {

	namespace Detail {
		template <typename TypeA, typename TypeB, typename ReturnType = void>
		using EnableIfDifferent =
		    typename std::enable_if<!std::is_same<TypeA, TypeB>::value, ReturnType>::type;
	}

	class Access {
		/* Iterators with private/protected impl methods must declare this class as friend.
		 * Calls to impl methods go through this class to bypass access restrictions.
		 * In addition, defines prototypes for the methods.
		 */
	public:
		// Increments/decrements the iterator
		template <typename It> static void next (It & it) { it.next (); }
		template <typename It> static void prev (It & it) { it.prev (); }

		// Test for equality with something
		template <typename It, typename Other> static bool equal (const It & it, const Other & other) {
			return it.equal (other);
		}

		// Shift the iterator, and distance
		template <typename It> static void advance (It & it, GetDifferenceType<It> n) {
			it.advance (n);
		}
		template <typename It> static GetDifferenceType<It> distance (const It & from, const It & to) {
			return from.distance (to);
		}
	};

	template <typename ValueType, typename DifferenceType = std::ptrdiff_t,
	          typename ReferenceType = ValueType &, typename Pointer = ValueType *>
	class MakeTraits {
	public:
		using value_type = ValueType;
		using difference_type = DifferenceType;
		using reference = ReferenceType;
		using pointer = Pointer;
	};

	// Value access categories (input / output)

	template <typename DerivedIt> class Input { public: };

	// Traversal categories

	template <typename DerivedIt, typename Traits> class SinglePass {
		/* Single pass traversal category.
		 * Cannot be copied, only moved.
		 * Represents an iterator with a non trivial state behind it.
		 *
		 * Requires: next, equal
		 */
	public:
		using value_type = typename Traits::value_type;
		using difference_type = typename Traits::difference_type;
		using reference = typename Traits::reference;
		using pointer = typename Traits::pointer;

		// Movable, non copyable
		SinglePass () = default;
		SinglePass (const SinglePass &) = delete;
		SinglePass & operator= (const SinglePass &) = delete;
		SinglePass (SinglePass &&) = default;
		SinglePass & operator= (SinglePass &&) = default;

		// In place increment
		DerivedIt & operator++ () {
			auto & self = static_cast<DerivedIt &> (*this);
			Access::next (self);
			return self;
		}

		// Equality test
		template <typename Other> bool operator== (const Other & other) const {
			return Access::equal (static_cast<const DerivedIt &> (*this), other);
		}
		template <typename Other> bool operator!= (const Other & other) const {
			return !(*this == other);
		}

		// Symmetric ops
		template <typename Other, typename = Detail::EnableIfDifferent<Other, DerivedIt>>
		friend bool operator== (const Other & other, const DerivedIt & it) {
			return it == other;
		}
		template <typename Other, typename = Detail::EnableIfDifferent<Other, DerivedIt>>
		friend bool operator!= (const Other & other, const DerivedIt & it) {
			return it != other;
		}
	};

	template <typename DerivedIt, typename Traits>
	class Forward : public SinglePass<DerivedIt, Traits> {
		// Forward iterator from STL (minus access info).
	public:
		using value_type = typename Traits::value_type;
		using difference_type = typename Traits::difference_type;
		using reference = typename Traits::reference;
		using pointer = typename Traits::pointer;
		using iterator_category = std::forward_iterator_tag;

		// Enable copy (use default constr from SinglePass, ok because sizeof(SinglePass)==0).
		Forward () = default;
		Forward (const Forward &) : SinglePass<DerivedIt, Traits> () {}
		Forward & operator= (const Forward &) { return *this; }
		Forward (Forward &&) = default;
		Forward & operator= (Forward &&) = default;

		// Post increment
		DerivedIt operator++ (int) {
			DerivedIt tmp (static_cast<const DerivedIt &> (*this));
			++*this;
			return tmp;
		}
	};

	template <typename DerivedIt, typename Traits>
	class Bidirectional : public Forward<DerivedIt, Traits> {
		/* Bidirectional iterator from STL (minus access info).
		 *
		 * Requires: prev
		 */
	public:
		using value_type = typename Traits::value_type;
		using difference_type = typename Traits::difference_type;
		using reference = typename Traits::reference;
		using pointer = typename Traits::pointer;
		using iterator_category = std::bidirectional_iterator_tag;

		// Constructors defaulted

		// Decrements
		DerivedIt & operator-- () {
			auto & self = static_cast<DerivedIt &> (*this);
			Access::prev (self);
			return self;
		}
		DerivedIt operator-- (int) {
			DerivedIt tmp (static_cast<const DerivedIt &> (*this));
			--*this;
			return tmp;
		}
	};

	template <typename DerivedIt, typename Traits>
	class RandomAccess : public Bidirectional<DerivedIt, Traits> {
		/* Random access iterator from STL (minus access info).
		 *
		 * Requires: advance, distance
		 */
	public:
		using value_type = typename Traits::value_type;
		using difference_type = typename Traits::difference_type;
		using reference = typename Traits::reference;
		using pointer = typename Traits::pointer;
		using iterator_category = std::random_access_iterator_tag;

		// Constructors defaulted

		DerivedIt & operator+= (difference_type n) {
			auto & self = static_cast<DerivedIt &> (*this);
			Access::advance (self, n);
			return self;
		}
		DerivedIt operator+ (difference_type n) const {
			DerivedIt tmp (static_cast<const DerivedIt &> (*this));
			tmp += n;
			return tmp;
		}
		friend DerivedIt operator+ (difference_type n, const DerivedIt & it) { return it + n; }

		DerivedIt & operator-= (difference_type n) { return *this += (-n); }
		DerivedIt operator- (difference_type n) const { return *this + (-n); }

		template <typename Other, typename = Detail::EnableIfDifferent<Other, difference_type>>
		difference_type operator- (const Other & other) const {
			auto & self = static_cast<const DerivedIt &> (*this);
			return Access::distance (self, other);
		}

		// operator [] TODO

		template <typename Other> bool operator< (const Other & other) const {
			return (*this - other) < 0;
		}
		template <typename Other> bool operator> (const Other & other) const {
			return (*this - other) > 0;
		}
		template <typename Other> bool operator<= (const Other & other) const {
			return (*this - other) <= 0;
		}
		template <typename Other> bool operator>= (const Other & other) const {
			return (*this - other) >= 0;
		}

		template <typename Other, typename = Detail::EnableIfDifferent<Other, DerivedIt>>
		friend bool operator< (const Other & other, const DerivedIt & it) {
			return it > other;
		}
		template <typename Other, typename = Detail::EnableIfDifferent<Other, DerivedIt>>
		friend bool operator> (const Other & other, const DerivedIt & it) {
			return it < other;
		}
		template <typename Other, typename = Detail::EnableIfDifferent<Other, DerivedIt>>
		friend bool operator<= (const Other & other, const DerivedIt & it) {
			return it >= other;
		}
		template <typename Other, typename = Detail::EnableIfDifferent<Other, DerivedIt>>
		friend bool operator>= (const Other & other, const DerivedIt & it) {
			return it <= other;
		}
	};
}
}
