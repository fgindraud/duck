#include <duck/optional.h>

int main () {
#if defined (TEST_CASE_1)
	// Const cannot be assigned, only the internal value is mutable
	const duck::Optional<int> p {42};
	p = 41;
#elif defined (TEST_CASE_2)
	// Optional<const T> cannot change its value
	duck::Optional<const int> p {42};
	*p = 41;
#endif
	return 0;
}

