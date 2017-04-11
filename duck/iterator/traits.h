#pragma once

// Iterator traits (compile time information).

#include <duck/type_traits.h>
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
	template <typename It> class HasValueType {
	private:
		template <typename T>
		static auto test (T) -> decltype (std::declval<typename T::value_type> (), std::true_type{});
		static auto test (...) -> decltype (std::false_type{});

		template <typename T, bool has_value_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> { using Type = typename T::value_type; };

	public:
		enum { value = decltype (test (std::declval<It> ()))::value };
		using MaybeType = MaybeTypeImpl<It, value>;
	};
	template <typename It> class HasDifferenceType {
	private:
		template <typename T>
		static auto test (T)
		    -> decltype (std::declval<typename T::difference_type> (), std::true_type{});
		static auto test (...) -> decltype (std::false_type{});

		template <typename T, bool has_difference_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> {
			using Type = typename T::difference_type;
		};

	public:
		enum { value = decltype (test (std::declval<It> ()))::value };
		using MaybeType = MaybeTypeImpl<It, value>;
	};
	template <typename It> class HasReferenceType {
	private:
		template <typename T>
		static auto test (T) -> decltype (std::declval<typename T::reference> (), std::true_type{});
		static auto test (...) -> decltype (std::false_type{});

		template <typename T, bool has_reference_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> { using Type = typename T::reference; };

	public:
		enum { value = decltype (test (std::declval<It> ()))::value };
		using MaybeType = MaybeTypeImpl<It, value>;
	};
	template <typename It> class HasPointerType {
	private:
		template <typename T>
		static auto test (T) -> decltype (std::declval<typename T::pointer> (), std::true_type{});
		static auto test (...) -> decltype (std::false_type{});

		template <typename T, bool has_pointer_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> { using Type = typename T::pointer; };

	public:
		enum { value = decltype (test (std::declval<It> ()))::value };
		using MaybeType = MaybeTypeImpl<It, value>;
	};

	// Add SFINAE capable typedefs (use std::iterator_traits).
	template <typename It>
	using GetValueType = typename HasValueType<std::iterator_traits<It>>::MaybeType::Type;
	template <typename It>
	using GetDifferenceType = typename HasDifferenceType<std::iterator_traits<It>>::MaybeType::Type;
	template <typename It>
	using GetReferenceType = typename HasReferenceType<std::iterator_traits<It>>::MaybeType::Type;
	template <typename It>
	using GetPointerType = typename HasPointerType<std::iterator_traits<It>>::MaybeType::Type;

	/* ------------------------------  Container iterator ---------------------------- */

	// Maybe-type testing if std::begin works on Container (returns iterator type)
	template <typename Container> class ContainerHasStdIterator {
	private:
		template <typename T>
		static auto test (T)
		    -> decltype (std::begin (std::declval<typename std::add_lvalue_reference<T>::type> ()),
		                 std::true_type{});
		static auto test (...) -> decltype (std::false_type{});

		template <typename T, bool has_std_it_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> {
			using Type =
			    decltype (std::begin (std::declval<typename std::add_lvalue_reference<T>::type> ()));
		};

	public:
		enum { value = decltype (test (std::declval<Container> ()))::value };
		using MaybeType = MaybeTypeImpl<Container, value>;
	};

	// Maybe-type testing if a user begin() works on Container (returns iterator type)
	template <typename Container> class ContainerHasUserIterator {
	private:
		template <typename T>
		static auto test (T)
		    -> decltype (begin (std::declval<typename std::add_lvalue_reference<T>::type> ()),
		                 std::true_type{});
		static auto test (...) -> decltype (std::false_type{});

		template <typename T, bool has_std_it_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> {
			using Type = decltype (begin (std::declval<typename std::add_lvalue_reference<T>::type> ()));
		};

	public:
		enum { value = decltype (test (std::declval<Container> ()))::value };
		using MaybeType = MaybeTypeImpl<Container, value>;
	};

	// Maybe-type testing if a user begin() works on Container (returns iterator type)
	template <typename Container>
	using ContainerHasIterator = Traits::SelectFirstDefined<ContainerHasStdIterator<Container>,
	                                                        ContainerHasUserIterator<Container>>;
	// SFINAE-type for iterator type
	template <typename Container>
	using GetContainerIteratorType = typename ContainerHasIterator<Container>::MaybeType::Type;
}
}
