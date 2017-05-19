#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/optional.h>

struct blah {};

TEST_CASE ("test") {
	duck::Optional<int> a;
	CHECK_FALSE (a);
}
