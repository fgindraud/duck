#pragma once

// Range V3
// STATUS: WIP

#ifndef HAS_CPP14
#define HAS_CPP14 (__cpluplus >= 201402L)
#endif

#include <duck/type_traits.h>
#include <iterator>
#include <utility>

namespace duck {
/*********************************************************************************
 * Type traits.
 */
namespace InternalADLCall {
	// namespace representing the "using std::sth; sth(object);" pattern.
	using std::begin;
	using std::end;
	template <typename T> auto call_begin (T && t) -> decltype (begin (std::forward<T> (t)));
	template <typename T> auto call_end (T && t) -> decltype (end (std::forward<T> (t)));
} // namespace InternalADLCall

// Iterator type deduced from begin(T / T&)
template <typename T>
using DeducedIteratorType = decltype (InternalADLCall::call_begin (std::declval<T &> ()));

// Is iterable: if we can call begin & end on the object
template <typename T, typename = void> struct IsIterable : std::false_type {};
template <typename T>
struct IsIterable<T, VoidT<decltype (InternalADLCall::call_begin (std::declval<T &> ())),
                           decltype (InternalADLCall::call_end (std::declval<T &> ()))>>
    : std::true_type {};

/*********************************************************************************
 * Free functions operating on iterable objects.
 * With optimised cases for containers.
 */

// begin / end
template <typename T> auto begin (T & t) -> DeducedIteratorType<T &> {
	using std::begin;
	return begin (t);
}
template <typename T> auto end (T & t) -> DeducedIteratorType<T &> {
	using std::end;
	return end (t);
}

// empty
namespace Internal {
	// Has empty() method
	template <typename T, typename = void> struct HasEmptyMethod : std::false_type {};
	template <typename T>
	struct HasEmptyMethod<T, VoidT<decltype (std::declval<const T &> ().empty ())>> : std::true_type {
	};

	template <typename T> bool empty_impl (const T & t, std::true_type) { return t.empty (); }
	template <typename T> bool empty_impl (const T & t, std::false_type) {
		return begin (t) == end (t);
	}
} // namespace Internal
template <typename T> bool empty (const T & t) {
	return Internal::empty_impl (t, Internal::HasEmptyMethod<T>{});
}

// size
namespace Internal {
	// Has size() method
	template <typename T, typename = void> struct HasSizeMethod : std::false_type {};
	template <typename T>
	struct HasSizeMethod<T, VoidT<decltype (std::declval<const T &> ().size ())>> : std::true_type {};

	template <typename T> auto size_impl (const T & t, std::true_type) -> decltype (t.size ()) {
		return t.size ();
	}
	template <typename T>
	auto size_impl (const T & t, std::false_type) -> decltype (std::distance (begin (t), end (t))) {
		return std::distance (begin (t), end (t));
	}
} // namespace Internal
template <typename T>
auto size (const T & t) -> decltype (Internal::size_impl (t, Internal::HasSizeMethod<T>{})) {
	return Internal::size_impl (t, Internal::HasSizeMethod<T>{});
}

} // namespace duck
