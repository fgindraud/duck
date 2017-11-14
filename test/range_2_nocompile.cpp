#include <duck/range/combinator.h>
#include <duck/range/range.h>
#include <forward_list>
#include <vector>

struct blah {};

namespace DRC = duck::Range::Combinator;

int main () {
#if defined(TEST_CASE_1)
	// Should not catch a random type
	auto r = duck::range (blah{});
#elif defined(TEST_CASE_2)
	// Reverse should not work on a forward list
	auto r = duck::range (std::forward_list<int>{1, 2, 3, 4}) | DRC::reversed ();
#endif
	return 0;
}
