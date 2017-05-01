#pragma once

// Iterator traits

#include <iterator>
#include <type_traits>

namespace duck {
namespace Iterator {
	// Access info
	template <typename It> using GetCategory = typename std::iterator_traits<It>::iterator_category;
	template <typename It> using GetValueType = typename std::iterator_traits<It>::value_type;
	template <typename It>
	using GetDifferenceType = typename std::iterator_traits<It>::difference_type;
	template <typename It> using GetReferenceType = typename std::iterator_traits<It>::reference;
	template <typename It> using GetPointerType = typename std::iterator_traits<It>::pointer;

	// Type manipulators
	template <typename CategoryTag, typename It>
	using HasCategory = std::is_base_of<CategoryTag, GetCategory<It>>;

	template <typename CategoryA, typename CategoryB>
	using CommonCategory = typename std::common_type<CategoryA, CategoryB>::type;

	template <typename It, typename Category>
	using RestrictToCategory = CommonCategory<GetCategory<It>, Category>;
}
}
