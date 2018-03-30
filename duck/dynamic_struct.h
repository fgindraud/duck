#pragma once

#include <cstddef>
#include <type_traits>

namespace duck {

using Integer = std::ptrdiff_t;

template <typename... Elements> struct Struct;

template <typename T> struct IsStruct : std::false_type {};
template <typename... Elements> struct IsStruct<Struct<Elements...>> : std::true_type {};

template <typename T> struct TypeInfo {
	static_assert (!IsStruct<T>::value, "TypeInfo must be specialised for Struct<Elements...>");

	static constexpr Integer size () noexcept { return static_cast<Integer> (sizeof (T)); }
	static constexpr Integer alignment () noexcept { return static_cast<Integer> (alignof (T)); }
};

} // namespace duck
