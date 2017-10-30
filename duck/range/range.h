#pragma once

// Range V2
// STATUS: WIP

#include <duck/type_traits.h>
#include <iterator>
#include <utility>

namespace duck {
namespace Range {
	namespace Detail {
		// Getters
		template <typename It>
		using IteratorCategory = typename std::iterator_traits<It>::iterator_category;
		template <typename It>
		using IteratorDifferenceType = typename std::iterator_traits<It>::difference_type;

		// Get iterator type through call begin
		namespace Detail {
			using std::begin;
			template <typename T> auto call_begin (T && t) -> decltype (begin (std::forward<T> (t)));
		} // namespace Detail
		template <typename T> using IteratorTypeOf = decltype (Detail::call_begin (std::declval<T> ()));

		// Enable ifs TODO use void_t
		template <typename It> using EnableIfIsIterator = IteratorCategory<It>;
		template <typename Int>
		using EnableIfIsInteger = typename std::enable_if<std::is_integral<Int>::value>::type;
		template <typename T> using EnableIfHasBegin = IteratorTypeOf<T>;
	} // namespace Detail

	/**********************************************************************************
	 * Provides typedefs for Range types (must be specialised).
	 * Member typedefs:
	 * - Iterator: iterator type
	 * - SizeType: type returned by size() method
	 */
	template <typename RangeType> struct RangeTraits;

	/* Common base interface for range types.
	 * Intended to be used in a CRTP pattern extending a derived range class.
	 * Derived classes can override these functions if they have a better implementation.
	 */
	template <typename Derived> class Base {
	private:
		constexpr const Derived & derived () const { return static_cast<const Derived &> (*this); }

	public:
		constexpr bool empty () const { return derived ().begin () == derived ().end (); }

		constexpr typename RangeTraits<Derived>::SizeType size () const {
			return std::distance (derived ().begin (), derived ().end ());
		}

		// TODO add interface of old Range which does not generate ranges
	};

	/**********************************************************************************
	 * IteratorPair: most basic range, stores a pair of iterator.
	 */
	template <typename It> class IteratorPair;

	template <typename It> struct RangeTraits<IteratorPair<It>> {
		using Iterator = It;
		using SizeType = Detail::IteratorDifferenceType<It>;
	};

	template <typename It> class IteratorPair : public Base<IteratorPair<It>> {
	private:
		It begin_;
		It end_;

	public:
		constexpr IteratorPair (It begin_it, It end_it)
		    : begin_ (std::move (begin_it)), end_ (std::move (end_it)) {}

		constexpr It begin () const { return begin_; }
		constexpr It end () const { return end_; }
	};

	template <typename It, typename = Detail::EnableIfIsIterator<It>>
	constexpr IteratorPair<It> range (It begin_it, It end_it) {
		return {std::move (begin_it), std::move (end_it)};
	}

	/**********************************************************************************
	 * Integer range: iterates on [i, j[
	 */
	template <typename Int> class IntegerIterator {
		static_assert (std::is_integral<Int>::value,
		               "IntegerIterator<Int>: Int must be an integer type");

	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = Int;
		using difference_type = std::ptrdiff_t;
		using pointer = const value_type *;
		using reference = value_type; // Not standard compliant

		constexpr IntegerIterator () noexcept = default;
		constexpr IntegerIterator (Int n) noexcept : n_ (n) {}

		// Input / output
		IntegerIterator & operator++ () noexcept { return ++n_, *this; }
		constexpr reference operator* () const noexcept { return n_; }
		constexpr pointer operator-> () const noexcept { return &n_; }
		constexpr bool operator== (const IntegerIterator & o) const noexcept { return n_ == o.n_; }
		constexpr bool operator!= (const IntegerIterator & o) const noexcept { return n_ != o.n_; }

		// Forward
		IntegerIterator operator++ (int) noexcept {
			IntegerIterator tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir
		IntegerIterator & operator-- () noexcept { return --n_, *this; }
		IntegerIterator operator-- (int) noexcept {
			IntegerIterator tmp (*this);
			--*this;
			return tmp;
		}

		// Random access
		IntegerIterator & operator+= (difference_type n) noexcept { return n_ += n, *this; }
		constexpr IntegerIterator operator+ (difference_type n) const noexcept {
			return IntegerIterator (n_ + n);
		}
		friend constexpr IntegerIterator operator+ (difference_type n,
		                                            const IntegerIterator & it) noexcept {
			return it + n;
		}
		IntegerIterator & operator-= (difference_type n) noexcept { return n_ -= n, *this; }
		constexpr IntegerIterator operator- (difference_type n) const noexcept {
			return IntegerIterator (n_ - n);
		}
		constexpr difference_type operator- (const IntegerIterator & o) const noexcept {
			return n_ - o.n_;
		}
		constexpr reference operator[] (difference_type n) const noexcept { return n_ + n; }
		constexpr bool operator< (const IntegerIterator & o) const noexcept { return n_ < o.n_; }
		constexpr bool operator> (const IntegerIterator & o) const noexcept { return n_ > o.n_; }
		constexpr bool operator<= (const IntegerIterator & o) const noexcept { return n_ <= o.n_; }
		constexpr bool operator>= (const IntegerIterator & o) const noexcept { return n_ >= o.n_; }

	private:
		Int n_{};
	};

	template <typename Int, typename = Detail::EnableIfIsInteger<Int>>
	constexpr IteratorPair<IntegerIterator<Int>> range (Int from, Int to) {
		return {IntegerIterator<Int>{from}, IntegerIterator<Int>{to}};
	}
	template <typename Int, typename = Detail::EnableIfIsInteger<Int>>
	constexpr IteratorPair<IntegerIterator<Int>> range (Int to) {
		return range (Int{0}, to);
	}

	/**********************************************************************************
	 * Container reference.
	 * Can store a const reference (Container = const T) or mutable reference (Container = T).
	 */
	template <typename Container> class ContainerRef;

	template <typename Container> struct RangeTraits<ContainerRef<Container>> {
		using Iterator = Detail::IteratorTypeOf<Container>;
		using SizeType = typename Container::size_type;
	};

	template <typename Container> class ContainerRef : public Base<ContainerRef<Container>> {
	private:
		Container & container_;

	public:
		using Traits = RangeTraits<ContainerRef<Container>>;

		constexpr ContainerRef (Container & ref) : container_ (ref) {}

		constexpr typename Traits::Iterator begin () const {
			using std::begin;
			return begin (container_);
		}
		constexpr typename Traits::Iterator end () const {
			using std::end;
			return end (container_);
		}

		Container & container () const noexcept { return container_; }

		constexpr bool empty () const { return container_.empty (); }
		constexpr typename Traits::SizeType size () const { return container_.size (); }
	};

	template <typename Container, typename = Detail::EnableIfHasBegin<Container>>
	constexpr ContainerRef<Container> range (Container & container) {
		return {container};
	}

		// TODO add ContainerValue
		// TODO add special case to just propagate ranges in range()
		// TODO use Base<T> to check if it is a range ?
		// TODO add a simple combinator for test !
		// -> Defining f(const T&), f(T&) and f(T&&) dispatches calls as intended.

#if 0
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

		// Input/forward iterator
		constexpr ReferenceType front () const { return *begin (); }
		Range pop_front () const { return Base<It>{std::next (begin ()), end ()}; }

		// Bidirectional iterator
		ReferenceType back () const { return *std::prev (end ()); }
		Range pop_back () const { return Base<It>{begin (), std::prev (end ())}; }

		// Random access iterator
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
#endif
} // namespace Range

// Pull main range() function in namespace duck::
using Range::range;
} // namespace duck
