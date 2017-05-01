#pragma once

// Manipulation of sfinae types

namespace duck {
namespace Maybe {
	/* A SFINAE-type is a struct:
	 * - {} (empty) if not defined
	 * - { using Type = ...; } if defined
	 *
	 * A maybe-type is a struct {
	 *  bool value; // is the type defined
	 *  struct MaybeType {}; // a SFINAE-type
	 * }
	 *
	 * Below are:
	 * - basic true/false types
	 * - and an accessor
	 */

	template <typename T> struct DefinedMaybeType {
		// A defined maybe-type storing a simple T
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

	// Unpacking typedef (practical, and is SFINAE valid !)
	template <typename MT> using Unpack = typename MT::MaybeType::Type;

	/* Wrap a simple T as a MaybeType depending on a bool condition.
	 */
	template <typename T, bool value> struct ConditionalMaybeType : UndefinedMaybeType {};
	template <typename T> struct ConditionalMaybeType<T, true> : DefinedMaybeType<T> {};

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

	/* std::decay as a MaybeType -> MaybeType operation.
	 */
	template <typename MT> struct Decay {
	private:
		template <typename T, bool has_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> {
			using Type = typename std::decay<Unpack<T>>::type;
		};

	public:
		enum { value = MT::value };
		using MaybeType = MaybeTypeImpl<MT, MT::value>;
	};

	/* std::add_pointer as a MaybeType -> MaybeType operation.
	 */
	template <typename MT> struct AddPointer {
	private:
		template <typename T, bool has_type> struct MaybeTypeImpl {};
		template <typename T> struct MaybeTypeImpl<T, true> {
			using Type = typename std::add_pointer<Unpack<T>>::type;
		};

	public:
		enum { value = MT::value };
		using MaybeType = MaybeTypeImpl<MT, MT::value>;
	};
}
}
