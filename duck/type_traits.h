#pragma once

// Additional useful type traits (and guaranteed to include <type_traits>)

#include <type_traits>

namespace duck {
namespace Traits {
	/* struct Self {
	 *   template<typename T> Self(T&&) { ... }
	 * };
	 * Self(T&&) will override the copy and move constructor of Self.
	 * Enable if with NonSelf<T, Self>::value to prevent this.
	 */
	template <typename T, typename Self> struct NonSelf {
		enum { value = !std::is_base_of<Self, typename std::decay<T>::type>::value };
	};
} // namespace Traits

/* Missing std::void_t in c++11.
 * Useful for quickly defining type traits.
 * Using impl from cpp-reference.
 */
template <typename... Types> struct MakeVoid { using Type = void; };
template <typename... Types> using VoidT = typename MakeVoid<Types...>::Type;

/* Enable if typedef.
 * Missing std::enable_if_t typedef in c++11.
 */
template <bool condition> using EnableIf = typename std::enable_if<condition>::type;
template <typename ConditionType> using EnableIfV = EnableIf<ConditionType::value>;

/* Missing shortening typedefs
 */
template <typename T> using DecayT = typename std::decay<T>::type;
template <typename T>
using RemoveCvRefT = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

// Common type tags
template <typename T> struct InPlace { constexpr InPlace () = default; };
constexpr InPlace<void> in_place{};
} // namespace duck
