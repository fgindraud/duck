#pragma once

// Additional useful type traits (and guaranteed to inclue <type_traits>)

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
}
}