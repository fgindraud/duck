#pragma once

// Iterator traits (compile time information).

#include <iterator>
#include <type_traits>

namespace duck {
namespace Iterator {
	// Access info
	template <typename It> using GetTraits = std::iterator_traits<It>;
	template <typename It> using GetCategory = typename GetTraits<It>::iterator_category;
	template <typename It> using GetDifferenceType = typename GetTraits<It>::difference_type;
	template <typename It> using GetValueType = typename GetTraits<It>::value_type;
	template <typename It> using GetPointerType = typename GetTraits<It>::pointer;
	template <typename It> using GetReferenceType = typename GetTraits<It>::reference;

	template <typename CategoryTag, typename It>
	using HasCategory = std::is_base_of<CategoryTag, GetCategory<It>>;

	// Enable ifs
	template <typename CategoryTag, typename It, typename ReturnType = void>
	using EnableIfHasCategory =
	    typename std::enable_if<HasCategory<CategoryTag, It>::value, ReturnType>::type;

	/* -------------------------------  Iterator typedef ----------------------------- */

	// SFINAE test if member is defined
	template <typename It> class HasValueType {
	private:
		template <typename T, typename = typename T::value_type> static constexpr bool test (int) {
			return true;
		}
		template <typename T> static constexpr bool test (...) { return false; }

	public:
		enum { value = test<It> (int()) };
	};
	template <typename It> class HasDifferenceType {
	private:
		template <typename T, typename = typename T::difference_type> static constexpr bool test (int) {
			return true;
		}
		template <typename T> static constexpr bool test (...) { return false; }

	public:
		enum { value = test<It> (int()) };
	};
	template <typename It> class HasReferenceType {
	private:
		template <typename T, typename = typename T::reference> static constexpr bool test (int) {
			return true;
		}
		template <typename T> static constexpr bool test (...) { return false; }

	public:
		enum { value = test<It> (int()) };
	};
	template <typename It> class HasPointerType {
	private:
		template <typename T, typename = typename T::pointer> static constexpr bool test (int) {
			return true;
		}
		template <typename T> static constexpr bool test (...) { return false; }

	public:
		enum { value = test<It> (int()) };
	};

	/* ------------------------------  Container iterator ---------------------------- */

	// SFINAE Test if it supports std::begin()
	template <typename Container> class ContainerHasStdIterator {
	private:
		template <typename T>
		static constexpr decltype (
		    std::begin (std::declval<typename std::add_lvalue_reference<T>::type> ()), bool())
		test (int) {
			return true;
		}
		template <typename T> static constexpr bool test (...) { return false; }

	public:
		enum { value = test<Container> (int()) };
	};

	// SFINAE Test if it supports begin() (ADL one)
	template <typename Container> class ContainerHasUserIterator {
	private:
		template <typename T>
		static constexpr decltype (begin (std::declval<typename std::add_lvalue_reference<T>::type> ()),
		                           bool())
		test (int) {
			return true;
		}
		template <typename T> static constexpr bool test (...) { return false; }

	public:
		enum { value = test<Container> (int()) };
	};

	template <typename Container> class ContainerHasIterator {
	public:
		enum {
			value =
			    ContainerHasUserIterator<Container>::value || ContainerHasStdIterator<Container>::value
		};
	};

	// SFINAE Get the container iterator type
	namespace Detail {
		// Container should be a container type, possibly const qualified for const_iterator.
		template <typename Container, bool has_user_iterator, bool has_std_iterator>
		struct GetContainerIterator {};
		template <typename Container, bool has_std_iterator>
		struct GetContainerIterator<Container, true, has_std_iterator> {
			using Type =
			    decltype (begin (std::declval<typename std::add_lvalue_reference<Container>::type> ()));
		};
		template <typename Container> struct GetContainerIterator<Container, false, true> {
			using Type = decltype (
			    std::begin (std::declval<typename std::add_lvalue_reference<Container>::type> ()));
		};
	}
	template <typename Container>
	using GetContainerIteratorType =
	    typename Detail::GetContainerIterator<Container, ContainerHasUserIterator<Container>::value,
	                                          ContainerHasStdIterator<Container>::value>::Type;
}
}
