#pragma once

// Additional useful type traits (and guaranteed to include <type_traits>)
// STATUS: operational, new_syntax_convention

#include <type_traits>

namespace duck {
/* Add missing type_traits from C++14 and beyond.
 * Typedef to std if possible, or reimplement.
 *
 * void_t: useful for defining type traits
 * enable_if_t: improve SFINAE readability
 */
#if __cplusplus >= 201402L
using std::add_const_t;
using std::add_lvalue_reference_t;
using std::aligned_storage_t;
using std::common_type_t;
using std::decay_t;
using std::enable_if_t;
using std::remove_cv_t;
using std::remove_reference_t;
#else
template <typename T> using add_const_t = typename std::add_const<T>::type;
template <typename T> using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;
template <std::size_t Len, std::size_t Align>
using aligned_storage_t = typename std::aligned_storage<Len, Align>::type;
template <typename... Types> using common_type_t = typename std::common_type<Types...>::type;
template <typename T> using decay_t = typename std::decay<T>::type;
template <bool B> using enable_if_t = typename std::enable_if<B>::type;
template <typename T> using remove_reference_t = typename std::remove_reference<T>::type;
template <typename T> using remove_cv_t = typename std::remove_cv<T>::type;
#endif

#if __cplusplus >= 201703L
using std::void_t;

using std::bool_constant;
using std::invoke_result_t;
#else
template <typename... Types> struct make_void { using type = void; };
template <typename... Types> using void_t = typename make_void<Types...>::type;

template <bool B> using bool_constant = std::integral_constant<bool, B>;
template <typename F, typename... ArgTypes>
using invoke_result_t = typename std::result_of<F (ArgTypes...)>::type;
#endif

// C++2a
template <typename T> using remove_cvref_t = remove_cv_t<remove_reference_t<T>>;

/* struct Self { template<typename T> Self(T&&) { ... } };
 * Self(T&&) will override the copy and move constructor of Self.
 * Enable if with does_not_match_constructor_of<T, Self>::value to prevent this.
 */
template <typename Class, typename T>
struct does_not_match_constructor_of : bool_constant<!std::is_base_of<Class, decay_t<T>>::value> {};

/* Type tags.
 * Use those from C++17, or define them.
 */
#if __cplusplus >= 201703L
using std::in_place;
using std::in_place_t;
using std::in_place_type_t;
#else
struct in_place_t {
	constexpr in_place_t () = default;
};
constexpr in_place_t in_place{};
template <typename T> struct in_place_type_t { constexpr in_place_type_t () = default; };
#endif
} // namespace Traits
