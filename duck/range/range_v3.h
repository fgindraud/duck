#pragma once

// Range V3
// STATUS: WIP, new_syntax_convention

// __cpluplus >= 201402L

#include <duck/type_traits.h>
#include <iterator>
#include <utility>

namespace duck {
/* Import std::begin and std::end in namespace duck.
 *
 * std::begin and std::end are supposed to be used as follows:
 * > using std::begin;
 * > it = begin (obj);
 * This unqualified calls will fetch both ADL and std:: overloads of begin().
 *
 * By importing std::begin and std::end, namespace duck is now a context where unqualified calls
 * catch the same overloads as this usage pattern.
 *
 * Calls external to namespace duck are supposed to call begin with the same pattern.
 * "using std::begin" is equivalent to "using duck::begin".
 */
using std::begin;
using std::end;

/*********************************************************************************
 * Type traits.
 */

// Iterator type deduced from begin(T / T&)
template <typename T> using range_iterator_t = decltype (begin (std::declval<T &> ()));

// A Range is anything iterable, with begin and end
template <typename T, typename = void> struct is_range : std::false_type {};
template <typename T>
struct is_range<
    T, void_t<decltype (begin (std::declval<T &> ())), decltype (end (std::declval<T &> ()))>>
    : std::true_type {};

// A lvalue_range is a range type referencing an external set of value:
// T = [const] Container & : lvalue range, the container exists outside of the range.
// T = Container : rvalue range, the container is owned by the range.
// Other range classes (like combinators) should override this default impl.
// (only for the non ref type, as a any T& is a lvalue range)
template <typename T> struct is_lvalue_range : std::is_lvalue_reference<T> {};

// Typedefs
template <typename It>
using iterator_category_t = typename std::iterator_traits<It>::iterator_category;
template <typename It> using iterator_value_type_t = typename std::iterator_traits<It>::value_type;
template <typename It> using iterator_reference_t = typename std::iterator_traits<It>::reference;
template <typename It> using iterator_pointer_t = typename std::iterator_traits<It>::pointer;
template <typename It>
using iterator_difference_t = typename std::iterator_traits<It>::difference_type;

// Is iterator
template <typename T, typename = void> struct is_iterator : std::false_type {};
template <typename T> struct is_iterator<T, void_t<iterator_category_t<T>>> : std::true_type {};

// Has empty() method
template <typename T, typename = void> struct has_empty_method : std::false_type {};
template <typename T>
struct has_empty_method<T, void_t<decltype (std::declval<const T &> ().empty ())>>
    : std::true_type {};

// Has size() method
template <typename T, typename = void> struct has_size_method : std::false_type {};
template <typename T>
struct has_size_method<T, void_t<decltype (std::declval<const T &> ().size ())>> : std::true_type {
};

/*********************************************************************************
 * rvalue range begin && end.
 * Only available if the rvalue range is in fact a wrapper for a lvalue object.
 * Both version just call the lvalue versions of begin/end.
 * This is not UB as the underlying iterators point to a lvalue object.
 */
template <typename T,
          typename = enable_if_t<!std::is_reference<T>::value && is_lvalue_range<T>::value>>
range_iterator_t<T> begin (T && t) {
	return begin (t); // Calls the lvalue begin
}
template <typename T,
          typename = enable_if_t<!std::is_reference<T>::value && is_lvalue_range<T>::value>>
range_iterator_t<T> end (T && t) {
	return end (t); // Calls the lvalue end
}

/*********************************************************************************
 * ADL versions of begin / end.
 * Alternative to the "using std::begin; begin (t)" pattern, in one line.
 * Also support rvalue ranges with the same conditions as begin/end.
 */
template <typename T> auto adl_begin (T && t) -> decltype (begin (std::forward<T> (t))) {
	return begin (std::forward<T> (t));
}
template <typename T> auto adl_end (T && t) -> decltype (end (std::forward<T> (t))) {
	return end (std::forward<T> (t));
}

/*********************************************************************************
 * Free functions operating on iterable objects.
 * With optimised cases for containers.
 */

// empty
namespace internal_range {
	template <typename T> bool empty_impl (const T & t, std::true_type) { return t.empty (); }
	template <typename T> bool empty_impl (const T & t, std::false_type) {
		return begin (t) == end (t);
	}
} // namespace internal_range
template <typename T> bool empty (const T & t) {
	return internal_range::empty_impl (t, has_empty_method<T>{});
}

// size
namespace internal_range {
	template <typename T> auto size_impl (const T & t, std::true_type) -> decltype (t.size ()) {
		return t.size ();
	}
	template <typename T>
	auto size_impl (const T & t, std::false_type) -> decltype (std::distance (begin (t), end (t))) {
		return std::distance (begin (t), end (t));
	}
} // namespace internal_range
template <typename T>
auto size (const T & t) -> decltype (internal_range::size_impl (t, has_size_method<T>{})) {
	return internal_range::size_impl (t, has_size_method<T>{});
}

// front / back : with rvalues overloads which force copying the value.
namespace internal_range {
	template <typename T>
	iterator_reference_t<range_iterator_t<T>> front_impl (T & t, std::true_type) {
		return *begin (t);
	}
	template <typename T>
	iterator_value_type_t<range_iterator_t<T>> front_impl (T & t, std::false_type) {
		return std::move (*begin (t));
	}
	template <typename T>
	iterator_reference_t<range_iterator_t<T>> back_impl (T & t, std::true_type) {
		return *std::prev (end (t));
	}
	template <typename T>
	iterator_value_type_t<range_iterator_t<T>> back_impl (T & t, std::false_type) {
		return std::move (*std::prev (end (t)));
	}
} // namespace internal_range
template <typename T>
auto front (T && t) -> decltype (internal_range::front_impl (t, is_lvalue_range<T>{})) {
	return internal_range::front_impl (t, is_lvalue_range<T>{});
}
template <typename T>
auto back (T && t) -> decltype (internal_range::back_impl (t, is_lvalue_range<T>{})) {
	return internal_range::back_impl (t, is_lvalue_range<T>{});
}

/*******************************************************************************
 * iterator_pair:
 * - eager range implementation, represents a pair of iterators
 * - does not guarantee that the iterated ressource stays alive (no lifetime extension !)
 */
template <typename It> class iterator_pair {
	static_assert (is_iterator<It>::value, "iterator_pair<It>: It must be an iterator type");

public:
	iterator_pair (It begin_it, It end_it) : begin_ (begin_it), end_ (end_it) {}
	It begin () const { return begin_; }
	It end () const { return end_; }

private:
	It begin_;
	It end_;
};

// iterator_pair nevers owns the iterated object, so always lvalue range
template <typename It> struct is_lvalue_range<iterator_pair<It>> : std::true_type {};

/*******************************************************************************
 * range() function overloads
 * TODO WIP add int, add init_list
 */

template <typename T, typename = enable_if_t<is_range<T>::value>>
auto range (T && t) -> decltype (std::forward<T> (t)) {
	return std::forward<T> (t);
}

template <typename It, typename = enable_if_t<is_iterator<It>::value>>
iterator_pair<It> range (It begin_it, It end_it) {
	return {begin_it, end_it};
}

// Array situation:
// T[N] -> matched by std::begin()
// (T*, T*) -> matched by range (It, It)
// (T*, N) -> below
template <typename T, typename IntType, typename = enable_if_t<std::is_integral<IntType>::value>>
iterator_pair<T *> range (T * base, IntType size) {
	return {base, base + size};
}

/*******************************************************************************
 * Range object.
 * Wraps any range into a user friendly object where most range functions available as methods.
 * FIXME try to have the same semantics with temporaries as free functions ?
 */
template <typename R> class range_object_wrapper {
private:
	R wrapped_;

public:
	range_object_wrapper (R && r) : wrapped_ (std::forward<R> (r)) {}

	// Forced to use duck:: prefix: unqualified lookup stops at class level
	auto begin () const -> decltype (duck::adl_begin (wrapped_)) {
		return duck::adl_begin (wrapped_);
	}
	auto end () const -> decltype (duck::adl_end (wrapped_)) { return duck::adl_end (wrapped_); }
	bool empty () const { return duck::empty (wrapped_); }
	auto size () const -> decltype (duck::size (wrapped_)) { return duck::size (wrapped_); }
	auto front () const -> decltype (duck::front (wrapped_)) { return duck::front (wrapped_); }
	auto back () const -> decltype (duck::back (wrapped_)) { return duck::back (wrapped_); }
};

// Just wraps R
template <typename R> struct is_lvalue_range<range_object_wrapper<R>> : is_lvalue_range<R> {};

template <typename R> range_object_wrapper<R> range_object (R && r) {
	return {std::forward<R> (r)};
}

// TODO operator== may not benefit from ADL, or may clash...

} // namespace duck
