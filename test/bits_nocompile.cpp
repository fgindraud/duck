#include <duck/bits.h>
int main () {
	using B = Bits<int>;
	auto p = B::ones ();
	return 0;
}
