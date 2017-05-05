#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/utility.h>

TEST_CASE ("max") {
	CHECK (duck::max (1) == 1);
	CHECK (duck::max (1, 2) == 2);
	CHECK (duck::max (1, 2, 3) == 3);
	CHECK (duck::max (1, 2, 3, 4) == 4);
	CHECK (duck::max (4, 3, 2, 1) == 4);
	CHECK (duck::max (1, 4, 2, 4) == 4);
	CHECK (duck::max (4, 4) == 4);
}

TEST_CASE ("min") {
	CHECK (duck::min (1) == 1);
	CHECK (duck::min (1, 2) == 1);
	CHECK (duck::min (1, 2, 3) == 1);
	CHECK (duck::min (1, 2, 3, 4) == 1);
	CHECK (duck::min (4, 3, 2, 1) == 1);
	CHECK (duck::min (1, 4, 2, 4) == 1);
	CHECK (duck::min (4, 4) == 4);
}
