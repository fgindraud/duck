#include <duck/old_range/range.h>
#include <vector>

struct blah {};

int main () {
#if defined(TEST_CASE_1)
	// Should not catch a random type
	auto r = duck::range (blah {});
#elif defined(TEST_CASE_2)
	// Should not catch a rvalue container
	auto r = duck::range (std::vector<int> {1, 2, 3, 4});
#endif
	return 0;
}
