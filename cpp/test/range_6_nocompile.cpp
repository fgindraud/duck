#include <forward_list>
#include <vector>

#include <duck/range/combinator.h>
#include <duck/range/range.h>

struct blah {};

int main () {
#if defined(TEST_CASE_1)
	// Should not catch a random type
	auto r = duck::range (blah{});
#elif defined(TEST_CASE_2)
	// Reverse should not work on a forward list
	auto r = std::forward_list<int>{1, 2, 3, 4} | duck::reverse ();
#elif defined(TEST_CASE_3)
	// Combinator should fail if not used with a range
	auto r = blah{} | duck::reverse ();
#elif defined(TEST_CASE_4)
	// Predicate not callable on values
	auto r = duck::range (42) | duck::filter ([](blah) { return true; });
#elif defined(TEST_CASE_5)
	// Predicate not returning a bool
	auto r = duck::range (42) | duck::filter ([](int) { return blah{}; });
#elif defined(TEST_CASE_6)
	// Function not callable on values
	auto r = duck::range (42) | duck::map ([](blah) { return 42; });
#endif
	return 0;
}
