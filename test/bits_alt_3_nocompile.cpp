#include <duck/bits.h>
struct A {
	int a;
};
int main () {
#if defined(TEST_CASE_1)
	using B = Bits<int>;
#elif defined(TEST_CASE_2)
	using B = Bits<char>;
#elif defined(TEST_CASE_3)
	using B = Bits<A>;
#endif
	auto p = B::ones ();
	return 0;
}
