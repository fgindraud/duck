#pragma once

// Iterator traits (compile time information).
// FIXME only used in facade, may need cleaning

#include <duck/meta/maybe_type.h>
#include <iterator>
#include <type_traits>

namespace duck {
namespace Iterator {
	// Access info
	template <typename It> using GetTraits = std::iterator_traits<It>;
	template <typename It> using GetCategory = typename GetTraits<It>::iterator_category;
	template <typename CategoryTag, typename It>
	using HasCategory = std::is_base_of<CategoryTag, GetCategory<It>>;

	// Enable ifs
	template <typename CategoryTag, typename It, typename ReturnType = void>
	using EnableIfHasCategory =
	    typename std::enable_if<HasCategory<CategoryTag, It>::value, ReturnType>::type;

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

	// Add SFINAE capable typedefs (use std::iterator_traits).
	template <typename It>
	using GetValueType = Maybe::Unpack<MaybeValueType<std::iterator_traits<It>>>;
	template <typename It>
	using GetDifferenceType = Maybe::Unpack<MaybeDifferenceType<std::iterator_traits<It>>>;
	template <typename It>
	using GetReferenceType = Maybe::Unpack<MaybeReferenceType<std::iterator_traits<It>>>;
	template <typename It>
	using GetPointerType = Maybe::Unpack<MaybePointerType<std::iterator_traits<It>>>;

	/* ------------------------------  Container iterator ---------------------------- */

	// SFINAE-type for iterator type
	namespace Detail {
		using std::begin;
		template <typename T> auto call_begin (T && t) -> decltype (begin (std::forward<T> (t)));
	}
	template <typename Container>
	using GetContainerIteratorType = decltype (Detail::call_begin (std::declval<Container> ()));
}
}
