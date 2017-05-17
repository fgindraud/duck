#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/small_vector.h>

void notice_this (duck::SmallVectorBase<int> & v) {
	(void) v;
}

#include <iostream>

TEST_CASE ("test") {
	duck::SmallVector<int, 2> a;
	CHECK (a.size () == 0);
	CHECK (a.capacity () == 2);
	CHECK_FALSE (a.is_allocated ());

	a.push_back (42);
	CHECK (a.size () == 1);
	CHECK (a.capacity () == 2);
	CHECK_FALSE (a.is_allocated ());
	CHECK (a.front () == 42);
	CHECK (a.back () == 42);
	CHECK (*a.begin () == 42);
	CHECK (*(a.end () - 1) == 42);

	a.push_back (-1);
	CHECK (a.size () == 2);
	CHECK (a.capacity () == 2);
	CHECK_FALSE (a.is_allocated ());
	CHECK (a.front () == 42);
	CHECK (a.back () == -1);

	a.push_back (33);
	CHECK (a.size () == 3);
	CHECK (a.capacity () > 2);
	CHECK (a.is_allocated ());
	CHECK (a.front () == 42);
	CHECK (a.back () == 33);
	CHECK (a[1] == -1);
}
