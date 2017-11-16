#pragma once

// Range V3
// STATUS: WIP, new_syntax_convention

// __cpluplus >= 201402L

#include <duck/type_traits.h>
#include <iterator>
#include <utility>

namespace duck {
/*********************************************************************************
 * Type traits.
 */
namespace internal_range {
	namespace adl_call {
		// namespace representing the "using std::sth; sth(object);" pattern.
		using std::begin;
		using std::end;
		template <typename T> auto call_begin (T && t) -> decltype (begin (std::forward<T> (t)));
		template <typename T> auto call_end (T && t) -> decltype (end (std::forward<T> (t)));
	} // namespace adl_call
} // namespace internal_range

// Iterator type deduced from begin(T / T&)
template <typename T>
using range_iterator_t = decltype (internal_range::adl_call::call_begin (std::declval<T &> ()));

// A Range is anything iterable, with begin and end
template <typename T, typename = void> struct is_range : std::false_type {};
template <typename T>
struct is_range<T, void_t<decltype (internal_range::adl_call::call_begin (std::declval<T &> ())),
                          decltype (internal_range::adl_call::call_end (std::declval<T &> ()))>>
    : std::true_type {};

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
 * Free functions operating on iterable objects.
 * With optimised cases for containers.
 */

// begin / end
template <typename T> auto begin (T & t) -> range_iterator_t<T &> {
	using std::begin;
	return begin (t);
}
template <typename T> auto end (T & t) -> range_iterator_t<T &> {
	using std::end;
	return end (t);
}

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

// front / back
template <typename T> auto front (T & t) -> decltype (*begin (t)) {
	return *begin (t);
}
template <typename T> auto back (T & t) -> decltype (*std::prev (end (t))) {
	return *std::prev (end (t));
}

// TODO operator== may not benefit from ADL, or may clash...

} // namespace duck
