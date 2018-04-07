#pragma once

#include <cstddef>
#include <type_traits>

namespace duck {

using Integer = std::ptrdiff_t;

// Pointers
struct RefBase {
	void * p;
	void * get () const noexcept { return p; }
};
template <typename T> struct Ref : RefBase {
	T * get () const noexcept { return static_cast<T *> (p); }
	T & operator* () const noexcept { return *get (); }
	T * operator-> () const noexcept { return get (); }
};

// Struct<...> & related defs
template <typename... Elements> struct Struct;

template <typename T> struct IsStruct : std::false_type {};
template <typename... Elements> struct IsStruct<Struct<Elements...>> : std::true_type {};

template <typename... Elements> struct Ref<Struct<Elements...>> : RefBase {};

// TypeInfo
template <typename T> struct TypeInfo {
	static_assert (!IsStruct<T>::value, "TypeInfo must be specialised for Struct<Elements...>");

	static constexpr Integer size () noexcept { return static_cast<Integer> (sizeof (T)); }
	static constexpr Integer alignment () noexcept { return static_cast<Integer> (alignof (T)); }
};

} // namespace duck
