#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/algorithm.h>
#include <duck/range/range.h>
#include <vector>

using duck::range;

//TODO do more than that

TEST_CASE ("filled vector") {
	std::vector<int> v1234 = {1, 2, 3, 4};
	CHECK (duck::count (range (v1234), 2) == 1);
	CHECK (duck::count (range (v1234).pop_front (2), 2) == 0);
}
