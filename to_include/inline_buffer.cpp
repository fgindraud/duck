#include <cpp-utils/inline_buffer.h>
struct A {
	int blah;
	InlineBuffer<int> b;
};
struct B {
	int blah;
	InlineBuffer<int> b;
	int oops;
};

int main () {
	return 0;
}
