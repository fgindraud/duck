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
namespace Internal {
	namespace ADLCall {
		// namespace representing the "using std::sth; sth(object);" pattern.
		using std::begin;
		using std::end;
		template <typename T> auto call_begin (T && t) -> decltype (begin (std::forward<T> (t)));
		template <typename T> auto call_end (T && t) -> decltype (end (std::forward<T> (t)));
	} // namespace ADLCall

	// Iterator type deduced from begin(T / T&)
	template <typename T>
	using DeducedIteratorType = decltype (ADLCall::call_begin (std::declval<T &> ()));

	// Has empty() method
	template <typename T, typename = void> struct HasEmptyMethod : std::false_type {};
	template <typename T>
	struct HasEmptyMethod<T, void_t<decltype (std::declval<const T &> ().empty ())>>
	    : std::true_type {};

	// Has size() method
	template <typename T, typename = void> struct HasSizeMethod : std::false_type {};
	template <typename T>
	struct HasSizeMethod<T, void_t<decltype (std::declval<const T &> ().size ())>> : std::true_type {
	};

	// Iterator has at least category
	template <typename RefCategory, typename TestedCategory>
	using HasCategory = std::is_base_of<RefCategory, TestedCategory>;

} // namespace Internal

// A Range is anything iterable, with begin and end
template <typename T, typename = void> struct IsRange : std::false_type {};
template <typename T>
struct IsRange<T, void_t<decltype (Internal::ADLCall::call_begin (std::declval<T &> ())),
                         decltype (Internal::ADLCall::call_end (std::declval<T &> ()))>>
    : std::true_type {};

// Range iterator type
template <typename T> using RangeIterator = Internal::DeducedIteratorType<T>;

/*********************************************************************************
 * Free functions operating on iterable objects.
 * With optimised cases for containers.
 */

// begin / end
template <typename T> auto begin (T & t) -> RangeIterator<T &> {
	using std::begin;
	return begin (t);
}
template <typename T> auto end (T & t) -> RangeIterator<T &> {
	using std::end;
	return end (t);
}

// empty
namespace Internal {
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

// front / back
template <typename T> auto front (T & t) -> decltype (*begin (t)) {
	return *begin (t);
}
template <typename T> auto back (T & t) -> decltype (*std::prev (end (t))) {
	return *std::prev (end (t));
}

// TODO namespacing ?
// TODO operator== may not benefit from ADL, or may clash...

} // namespace duck
