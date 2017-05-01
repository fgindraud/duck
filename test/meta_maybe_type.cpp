#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/meta/maybe_type.h>
#include <type_traits>

template <typename SelectedMaybeType, typename T>
using SelectedMatches = std::is_same<duck::Maybe::Unpack<SelectedMaybeType>, T>;

// SFINAE context test
template <typename T, typename = duck::Maybe::Unpack<T>> static bool test_has_type (T) {
	return true;
}
static bool test_has_type (...) {
	return false;
}

TEST_CASE ("type getter selector") {
	using namespace duck::Maybe;

	// Selection with no defined element
	using Undef_Undef_SelectedAs = SelectFirstDefined<UndefinedMaybeType, UndefinedMaybeType>;
	CHECK_FALSE (Undef_Undef_SelectedAs::value);
	using Undef_Undef_SelectedAs_UndefMaybeType =
	    std::is_base_of<UndefinedMaybeType, Undef_Undef_SelectedAs>;
	CHECK (Undef_Undef_SelectedAs_UndefMaybeType::value);

	// Selection with one defined element
	using DefinedInt_SelectedAs = SelectFirstDefined<DefinedMaybeType<int>>;
	CHECK (DefinedInt_SelectedAs::value);
	using DefinedInt_SelectedAs_Int = SelectedMatches<DefinedInt_SelectedAs, int>;
	CHECK (DefinedInt_SelectedAs_Int::value);

	// Selection with two defined element
	using DefinedInt_DefinedBool_SelectedAs =
	    SelectFirstDefined<DefinedMaybeType<int>, DefinedMaybeType<bool>>;
	CHECK (DefinedInt_DefinedBool_SelectedAs::value);
	using DefinedInt_DefinedBool_SelectedAs_Int =
	    SelectedMatches<DefinedInt_DefinedBool_SelectedAs, int>;
	CHECK (DefinedInt_DefinedBool_SelectedAs_Int::value);

	// Test SFINAE capability
	CHECK (test_has_type (DefinedMaybeType<int>{}));
	CHECK_FALSE (test_has_type (UndefinedMaybeType{}));
	CHECK_FALSE (test_has_type (Undef_Undef_SelectedAs{}));
	CHECK (test_has_type (DefinedInt_SelectedAs{}));
	CHECK (test_has_type (DefinedInt_DefinedBool_SelectedAs{}));
}
