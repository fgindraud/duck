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

// Typedefs
template <typename It>
using iterator_category_t = typename std::iterator_traits<It>::iterator_category;
template <typename It> using iterator_value_type_t = typename std::iterator_traits<It>::value_type;
template <typename It> using iterator_reference_t = typename std::iterator_traits<It>::reference;
template <typename It> using iterator_pointer_t = typename std::iterator_traits<It>::pointer;
template <typename It> using iterator_difference_t = typename std::iterator_traits<It>::difference;

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
 * ADL versions of begin / end.
 * Alternative to the "using std::begin; begin (t)" pattern, in one line.
 */
template <typename T> auto adl_begin (T & t) -> range_iterator_t<T> {
	return begin (t);
}
template <typename T> auto adl_end (T & t) -> range_iterator_t<T> {
	return end (t);
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
template <typename T> auto front (T & t) -> iterator_reference_t<range_iterator_t<T>> {
	return *begin (t);
}
template <typename T, typename = enable_if_t<!std::is_reference<T>::value>>
auto front (const T && t) -> iterator_value_type_t<range_iterator_t<const T>> {
	return *begin (t);
}

template <typename T> auto back (T & t) -> iterator_reference_t<range_iterator_t<T>> {
	return *std::prev (end (t));
}
template <typename T, typename = enable_if_t<!std::is_reference<T>::value>>
auto back (const T && t) -> iterator_value_type_t<range_iterator_t<const T>> {
	return *std::prev (end (t));
}

// TODO operator== may not benefit from ADL, or may clash...

} // namespace duck
