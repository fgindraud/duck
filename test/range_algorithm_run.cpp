#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range_algorithm.h>
#include <vector>

using duck::range;
using namespace duck::wrapper;

TEST_CASE ("filled vector") {
	std::vector<int> v1234 = {1, 2, 3, 4};
	CHECK (count (range (v1234), 2) == 1);
	CHECK (count (range (v1234).pop_front (2), 2) == 0);
}
