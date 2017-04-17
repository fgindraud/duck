#include <duck/format/basic.h>

struct blah {};

int main () {
#if defined(TEST_CASE_1)
	// Should not catch a generic struct
	auto f = duck::format (blah{});
#elif defined(TEST_CASE_2)
	// Incomplete formatters should not be formattable
	auto f = duck::format () << "hello " << duck::placeholder << "!";
	f.write (static_cast<char *> (nullptr));
#endif
	return 0;
}
