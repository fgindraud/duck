#pragma once

// Manipulation of types

namespace duck {
namespace Traits {
	/* A SFINAE-type is a struct:
	 * - {} (empty) if not defined
	 * - { using Type = ...; } if defined
	 *
	 * A maybe-type is a struct {
	 *  bool value; // is the type defined
	 *  struct MaybeType {}; // a SFINAE-type
	 * }
	 */
	template <typename T> struct DefinedMaybeType {
		// A defined maybe-type storing T
		enum { value = true };
		struct MaybeType {
			using Type = T;
		};
	};
	struct UndefinedMaybeType {
		// An undefined maybe-type
		enum { value = false };
		struct MaybeType {};
	};

	/* SelectFirstDefined<MaybeTypeA, MaybeTypeB> is a maybe-type with:
	 * - value == true if at least one of the maybe-types was defined
	 * - MaybeType == MaybeTypeA::MaybeType if MaybeTypeA::value
	 * - or MaybeType == MaybeTypeB::MaybeType if MaybeTypeB::value
	 * - or UndefinedMaybeType if no ::value was true
	 *
	 * Adding DefinedMaybeType<T> acts as a default case.
	 */
	template <typename... MaybeTypes> struct SelectFirstDefined;

	// Default
	template <> struct SelectFirstDefined<> : UndefinedMaybeType {};

	// Test first recursion
	namespace Detail {
		// Test next default case
		template <bool mt_is_defined, typename MT, typename... MaybeTypes>
		struct SelectFirstDefinedTestFirst : SelectFirstDefined<MaybeTypes...> {};
		// MT is defined, just use it
		template <typename MT, typename... MaybeTypes>
		struct SelectFirstDefinedTestFirst<true, MT, MaybeTypes...> : MT {};
	}
	template <typename MT, typename... MaybeTypes>
	struct SelectFirstDefined<MT, MaybeTypes...>
	    : Detail::SelectFirstDefinedTestFirst<MT::value, MT, MaybeTypes...> {};
}
}
