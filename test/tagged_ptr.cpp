#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/tagged_ptr.h>

TEST_CASE ("test") {
	using MyPtr = duck::TaggedPtr<int, 2>;

	int a;
	MyPtr p = &a;
	CHECK (p.get_ptr () == &a);
	CHECK_FALSE (p.get_bit<0> ());
	CHECK_FALSE (p.get_bit<1> ());
	CHECK_FALSE (p.get_bit (0));
	CHECK_FALSE (p.get_bit (1));

	p.set_bit<0> (false);
	CHECK (p.get_ptr () == &a);
	CHECK_FALSE (p.get_bit<0> ());
	CHECK_FALSE (p.get_bit<1> ());

	p.set_bit<0> (true);
	CHECK (p.get_ptr () == &a);
	CHECK (p.get_bit<0> ());
	CHECK_FALSE (p.get_bit<1> ());

	p.set_bit<1> (false);
	CHECK (p.get_ptr () == &a);
	CHECK (p.get_bit<0> ());
	CHECK_FALSE (p.get_bit<1> ());

	p.set_bit<1> (true);
	CHECK (p.get_ptr () == &a);
	CHECK (p.get_bit<0> ());
	CHECK (p.get_bit<1> ());

	int b;
	p = &b;
	CHECK (p.get_ptr () == &b);
	CHECK (p.get_bit<0> ());
	CHECK (p.get_bit<1> ());

	p.set_bit<0> (false);
	CHECK (p.get_ptr () == &b);
	CHECK_FALSE (p.get_bit<0> ());
	CHECK (p.get_bit<1> ());
}
