#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/iterator/facade.h>
#include <iostream>

class IntItImpl {
public:
	using value_type = int;
	using difference_type = int;
	using reference = const value_type &;
	using pointer = const value_type *;

protected:
	IntItImpl (int i = 0) : i_ (i) {}
	void next () { advance (1); }
	void prev () { advance (-1); }
	bool equal (IntItImpl it) const { return distance (it) == 0; }
	int distance (IntItImpl it) const { return i_ - it.i_; }
	void advance (int n) { i_ += n; }
	const int & deref () const { return i_; }

private:
	int i_{};
};

using IntIt = duck::Iterator::Facade<IntItImpl>;

TEST_CASE ("random int iterator") {
	IntIt a{0};
	IntIt b{42};
	IntIt c;
	c = b;
	CHECK (b > a);
	CHECK ((b - a) == 42);
	b -= 32;
	CHECK (b == a + 10);
	CHECK (*a == 0);
	CHECK (*b == 10);
}

// TODO more tests
