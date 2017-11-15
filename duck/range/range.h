#pragma once

// Range V2
// STATUS: WIP

#ifndef HAS_CPP14
#define HAS_CPP14 (__cpluplus >= 201402L)
#endif

#include <duck/type_traits.h>
#include <initializer_list> // init list overload
#include <iterator>
#include <string> // char_range
#include <utility>
#include <vector> // init list overload

#if HAS_CPP14
#include <algorithm> // operator==
#endif

namespace duck {
namespace Range {
	/* Ranges base header file.
	 * Provide a common interface for iterables.
	 *
	 * A range represents something which can be iterated upon.
	 * The range is constant : after creation, they cannot be modified.
	 * The iterated objects can be modified however, if supported by the objects
	 * (const_iterator VS iterator).
	 * Some range may be assignable, but this is not guaranteed to be true.
	 *
	 * Ranges are created by calling range() on supported arguments.
	 * range() will select the right range class to use.
	 * This file provides most basic overloads of range().
	 *
	 * Most represent a reference to an iterable (range(vec)).
	 * Some store the object itself (range of a temporary object).
	 * The object in this case becomes constant and cannot be retrieved.
	 * To retrieve the object, store the temporary then call range on it (reference).
	 */

	/**********************************************************************************
	 * Provides typedefs for Range types (must be specialised).
	 * Member typedefs:
	 * - Iterator: iterator type
	 * - SizeType: type returned by size() method
	 */
	template <typename RangeType> struct RangeTraits;

	/* Type trait to test if this is a range type.
	 * Impl: tests if RangeTraits is defined.
	 */
	template <typename T, typename = void> struct IsRange : std::false_type {};
	template <typename T>
	struct IsRange<T, VoidT<typename RangeTraits<T>::Iterator>> : std::true_type {};

	/*********************************************************************************
	 * Other useful type traits.
	 */

	// Is an iterator: std::iterator_traits<It> is SFINAE compatible
	template <typename It, typename = void> struct IsIterator : std::false_type {};
	template <typename It>
	struct IsIterator<It, VoidT<typename std::iterator_traits<It>::iterator_category>>
	    : std::true_type {};

	namespace UnifiedCall {
		// namespace representing the "using std::sth; sth(object);" pattern.
		using std::begin;
		using std::end;
		template <typename T> auto call_begin (T && t) -> decltype (begin (std::forward<T> (t)));
		template <typename T> auto call_end (T && t) -> decltype (end (std::forward<T> (t)));
	} // namespace UnifiedCall

	// Supports both T& and T.
	template <typename T>
	using IteratorTypeOf = decltype (UnifiedCall::call_begin (std::declval<T &> ()));

	// Is iterable: if we can call begin & end on the object
	template <typename T, typename = void> struct IsIterable : std::false_type {};
	template <typename T>
	struct IsIterable<T, VoidT<decltype (UnifiedCall::call_begin (std::declval<T &> ())),
	                           decltype (UnifiedCall::call_end (std::declval<T &> ()))>>
	    : std::true_type {};
	template <typename T>
	using IsBaseTypeIterable = IsIterable<typename std::remove_reference<T>::type>;

	// Is container: has empty(), size() methods and is Iterable.
	template <typename T, typename = void> struct IsContainer : std::false_type {};
	template <typename T>
	struct IsContainer<T, VoidT<decltype (std::declval<T &> ().size ()), typename T::size_type>>
	    : IsIterable<T> {};
	template <typename T>
	using IsBaseTypeContainer = IsContainer<typename std::remove_reference<T>::type>;

	/**********************************************************************************
	 * Common base interface for range types.
	 * Intended to be used in a CRTP pattern extending a derived range class.
	 * Derived classes can override these functions if they have a better implementation.
	 *
	 * Ranges must inherit from Base.
	 * Ranges can be taken by reference through it (similar to eigen strategy).
	 * Note that all methods should be called through derived().method ().
	 * Base inherit from RangeTraits of type to have useful typedefs.
	 */
	template <typename Derived> class Base : public RangeTraits<Derived> {
	public:
		using typename RangeTraits<Derived>::Iterator;
		using typename RangeTraits<Derived>::SizeType;

		using ReferenceType = typename std::iterator_traits<Iterator>::reference;
		using DifferenceType = typename std::iterator_traits<Iterator>::difference_type;

		const Derived & derived () const { return static_cast<const Derived &> (*this); }

		bool empty () const { return derived ().begin () == derived ().end (); }
		SizeType size () const { return std::distance (derived ().begin (), derived ().end ()); }

		// Accesses (UB if empty or out of range ; available if Iterator supports it)
		ReferenceType front () const { return *derived ().begin (); }
		ReferenceType back () const { return *std::prev (derived ().end ()); }
		ReferenceType operator[] (DifferenceType n) const { return derived ().begin ()[n]; }

		// TODO provide a std::distance(it, first, last)
		bool contains (Iterator it) const;
		DifferenceType offset_of (Iterator it) const;

		// Python like referencing: -1 is last, etc.
		Iterator at (DifferenceType n) const {
			auto index = n < 0 ? n + derived ().size () : n;
			return std::next (derived ().begin (), index);
		}

		// Build a container from this range
		template <typename Container> Container to_container () const {
			return Container{derived ().begin (), derived ().end ()};
		}
	};

	// range() overload: forwards ranges as is
	template <typename R, typename = EnableIfV<IsRange<R>>>
	auto range (R && r) -> decltype (std::forward<R> (r)) {
		return std::forward<R> (r);
	}

	/**********************************************************************************
	 * IteratorPair: most basic range, stores a pair of iterator.
	 */
	template <typename It> class IteratorPair;

	template <typename It> struct RangeTraits<IteratorPair<It>> {
		using Iterator = It;
		using SizeType = typename std::iterator_traits<It>::difference_type;
	};

	template <typename It> class IteratorPair : public Base<IteratorPair<It>> {
		static_assert (IsIterator<It>::value, "IteratorPair<It>: It must be a valid iterator type");

	public:
		IteratorPair (It begin_it, It end_it) : begin_ (begin_it), end_ (end_it) {}

		It begin () const { return begin_; }
		It end () const { return end_; }

	private:
		It begin_;
		It end_;
	};

	// range() overload
	template <typename It, typename = EnableIfV<IsIterator<It>>>
	IteratorPair<It> range (It begin_it, It end_it) {
		return {begin_it, end_it};
	}

	// Array situation:
	// T[N] -> matched by Iterable<T>
	// (T*, T*) -> matched by IteratorPair<It>
	// (T*, N) -> below
	template <typename T, typename IntType, typename = EnableIfV<std::is_integral<IntType>>>
	IteratorPair<T *> range (T * base, IntType size) {
		return {base, base + size};
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
		using reference = value_type; // Force copying the int

		IntegerIterator () noexcept = default;
		IntegerIterator (Int n) noexcept : n_ (n) {}

		// Input / output
		IntegerIterator & operator++ () noexcept { return ++n_, *this; }
		reference operator* () const noexcept { return n_; }
		pointer operator-> () const noexcept { return &n_; }
		bool operator== (const IntegerIterator & o) const noexcept { return n_ == o.n_; }
		bool operator!= (const IntegerIterator & o) const noexcept { return n_ != o.n_; }

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
		IntegerIterator operator+ (difference_type n) const noexcept {
			return IntegerIterator (n_ + n);
		}
		friend IntegerIterator operator+ (difference_type n, const IntegerIterator & it) noexcept {
			return it + n;
		}
		IntegerIterator & operator-= (difference_type n) noexcept { return n_ -= n, *this; }
		IntegerIterator operator- (difference_type n) const noexcept {
			return IntegerIterator (n_ - n);
		}
		difference_type operator- (const IntegerIterator & o) const noexcept { return n_ - o.n_; }
		reference operator[] (difference_type n) const noexcept { return n_ + n; }
		bool operator< (const IntegerIterator & o) const noexcept { return n_ < o.n_; }
		bool operator> (const IntegerIterator & o) const noexcept { return n_ > o.n_; }
		bool operator<= (const IntegerIterator & o) const noexcept { return n_ <= o.n_; }
		bool operator>= (const IntegerIterator & o) const noexcept { return n_ >= o.n_; }

	private:
		Int n_{};
	};

	// range() overloads
	template <typename Int, typename = EnableIfV<std::is_integral<Int>>>
	IteratorPair<IntegerIterator<Int>> range (Int from, Int to) {
		return {IntegerIterator<Int>{from}, IntegerIterator<Int>{to}};
	}
	template <typename Int, typename = EnableIfV<std::is_integral<Int>>>
	IteratorPair<IntegerIterator<Int>> range (Int to) {
		return range (Int{0}, to);
	}

	/**********************************************************************************
	 * Iterable reference & value.
	 * Reference: const and mutable lvalue-references (T = const I& / I&).
	 * Value: store temporary object to guarantee its lifetime (T = I).
	 * This class is just a lazy iterator pair.
	 */
	template <typename T> class Iterable;

	// Utils to get base (decayed) stored type:
	// T = const I& / I& -> const I / I
	// T = I -> const I (a stored temporary is considered constant)
	template <typename T>
	using IterableBaseType = typename std::conditional<std::is_reference<T>::value,
	                                                   typename std::remove_reference<T>::type,
	                                                   typename std::add_const<T>::type>::type;

	// Same traits as the iterator pair.
	// std::remove_reference<T> gives us const I or I, to select I::const_iterator or I::iterator
	template <typename T> struct RangeTraits<Iterable<T>> {
		using Iterator = IteratorTypeOf<IterableBaseType<T>>;
		using SizeType = typename std::iterator_traits<Iterator>::difference_type;
	};

	template <typename T> class Iterable : public Base<Iterable<T>> {
		static_assert (IsBaseTypeIterable<T>::value, "Iterable<T>: requires that T is iterable");
		static_assert (std::is_lvalue_reference<T>::value || !std::is_reference<T>::value,
		               "Iterable<T>: T must be 'const I&', 'I&', or 'I'");

	public:
		using typename Base<Iterable<T>>::Iterator;

		// T = const I& / I& : T&& -> const I& / I &, just copy reference (reference collapsing)
		// T = I : T&& -> I&&, construct I by move
		Iterable (T && t) : iterable_ (std::forward<T> (t)) {}

		// T = const I& / I& : returns I::const_iterator / I::iterator
		// T = I : returns const_iterator, object is constant
		Iterator begin () const {
			using std::begin;
			return begin (iterable_);
		}
		Iterator end () const {
			using std::end;
			return end (iterable_);
		}

	private:
		T iterable_; // Reference for const I& / I&, object for I.
	};

	// range() overload
	// Matchings: const I& -> Iterable<const I&> ; I& -> Iterable<I&> ; I&& -> Iterable<I>.
	template <typename T,
	          typename = EnableIf<IsBaseTypeIterable<T>::value && !IsBaseTypeContainer<T>::value>>
	Iterable<T> range (T && iterable) {
		return {std::forward<T> (iterable)};
	}

	/**********************************************************************************
	 * Container reference & value.
	 * Similar to iterable, but specialised for STL compatible containers.
	 * Provide optimized functions using the container directly (empty, size).
	 */
	template <typename C> class Container;

	// Traits
	template <typename C> struct RangeTraits<Container<C>> {
		using Iterator = IteratorTypeOf<IterableBaseType<C>>;
		using SizeType = typename IterableBaseType<C>::size_type;
	};

	template <typename C> class Container : public Base<Container<C>> {
		static_assert (std::is_lvalue_reference<C>::value || !std::is_reference<C>::value,
		               "Container<C>: C must be 'const I&', 'I&', or 'I'");
		static_assert (IsBaseTypeContainer<C>::value, "Container<C>: C must be a container");

	public:
		using typename Base<Container<C>>::Iterator;
		using typename Base<Container<C>>::SizeType;

		Container (C && c) : container_ (std::forward<C> (c)) {}

		Iterator begin () const {
			using std::begin;
			return begin (container_);
		}
		Iterator end () const {
			using std::end;
			return end (container_);
		}
		SizeType size () const { return container_.size (); }

	private:
		C container_;
	};

	// range () overloads
	template <typename C, typename = EnableIfV<IsBaseTypeContainer<C>>>
	Container<C> range (C && container) {
		return {std::forward<C> (container)};
	}
	template <typename T> Container<std::vector<T>> range (std::initializer_list<T> ilist) {
		// Range overload for initializer_list<T>.
		// initializer_list are somewhat equivalent to reference to const temporary.
		// Thus they cannot be stored (lifetime will not be extended enough).
		// So we have to build a container with its data (vector will do fine...).
		return {ilist};
	}

	/**********************************************************************************
	 * String range.
	 * range() only cares about storage : "hello" is considered as char[6] "hello\0".
	 * char_range () removes the null terminator
	 * FIXME fragile
	 */
	template <std::size_t N, typename = EnableIf<(N > 0)>>
	IteratorPair<const char *> char_range (const char (&str)[N]) {
		return {&str[0], &str[N - 1]};
	}
	inline IteratorPair<const char *> char_range (const char * str) {
		return {str, str + std::char_traits<char>::length (str)};
	}

	/**********************************************************************************
	 * Utils.
	 */

	// operator== returns true if values are equal and range have same length
	template <typename R1, typename R2, typename = EnableIf<IsRange<R1>::value && IsRange<R2>::value>>
	bool operator== (const R1 & r1, const R2 & r2) {
#if HAS_CPP14
		return std::equal (r1.begin (), r1.end (), r2.begin (), r2.end ());
#else
		typename R1::Iterator r1_it = r1.begin ();
		typename R1::Iterator r1_end = r1.end ();
		typename R2::Iterator r2_it = r2.begin ();
		typename R2::Iterator r2_end = r2.end ();
		for (; r1_it != r1_end && r2_it != r2_end; ++r1_it, ++r2_it)
			if (!(*r1_it == *r2_it))
				return false;
		return r1_it == r1_end && r2_it == r2_end;
#endif
	}
} // namespace Range

// Pull range() functions in namespace duck
using Range::char_range;
using Range::range;
} // namespace duck
