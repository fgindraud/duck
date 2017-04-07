#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/iterator/facade.h>
#include <iostream>

class IntIt
    : public duck::Iterator::RandomAccess<
          IntIt, duck::Iterator::MakeTraits<int, std::ptrdiff_t, const int &, const int *>> {
public:
	explicit IntIt (int i) : i_ (i) {}

private:
	friend class duck::Iterator::Access;
	void next () { ++i_; }
	bool equal (IntIt it) const { return i_ == it.i_; }
	difference_type distance (IntIt it) const { return i_ - it.i_; }
	void advance (difference_type n) { i_ += n; }
	int i_;
};

TEST_CASE ("random int iterator") {
	IntIt a{0};
	IntIt b{42};
	CHECK (b > a);
	CHECK ((b - a) == 42);
	b -= 32;
	CHECK (b == a + 10);
}

// TODO more tests
