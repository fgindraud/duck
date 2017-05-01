#pragma once

// Iterator maybe type traits
// FIXME only used in facade, may need cleaning

#include <duck/meta/maybe_type.h>
#include <iterator>
#include <type_traits>

namespace duck {
namespace Iterator {
	/* -------------------------------  Iterator typedef ----------------------------- */

	// Maybe types testing if some typedefs exists.
	// [!] Do not apply std::iterator_traits beforehand (will not work on raw T*).
	template <typename It> class MaybeValueType {
	private:
		template <typename T>
		static auto test (T) -> decltype (std::declval<typename T::value_type> (), std::true_type{});
		static auto test (...) -> std::false_type;

		template <typename T, bool has_value_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> { using Type = typename T::value_type; };

	public:
		enum { value = decltype (test (std::declval<It> ()))::value };
		using MaybeType = MaybeTypeImpl<It, value>;
	};
	template <typename It> class MaybeDifferenceType {
	private:
		template <typename T>
		static auto test (T)
		    -> decltype (std::declval<typename T::difference_type> (), std::true_type{});
		static auto test (...) -> std::false_type;

		template <typename T, bool has_difference_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> {
			using Type = typename T::difference_type;
		};

	public:
		enum { value = decltype (test (std::declval<It> ()))::value };
		using MaybeType = MaybeTypeImpl<It, value>;
	};
	template <typename It> class MaybeReferenceType {
	private:
		template <typename T>
		static auto test (T) -> decltype (std::declval<typename T::reference> (), std::true_type{});
		static auto test (...) -> std::false_type;

		template <typename T, bool has_reference_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> { using Type = typename T::reference; };

	public:
		enum { value = decltype (test (std::declval<It> ()))::value };
		using MaybeType = MaybeTypeImpl<It, value>;
	};
	template <typename It> class MaybePointerType {
	private:
		template <typename T>
		static auto test (T) -> decltype (std::declval<typename T::pointer> (), std::true_type{});
		static auto test (...) -> std::false_type;

		template <typename T, bool has_pointer_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> { using Type = typename T::pointer; };

	public:
		enum { value = decltype (test (std::declval<It> ()))::value };
		using MaybeType = MaybeTypeImpl<It, value>;
	};
}
}
