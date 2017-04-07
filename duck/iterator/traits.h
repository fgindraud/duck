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
}
}
