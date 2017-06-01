#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/small_unique_ptr.h>

struct Base {
	virtual ~Base () = default;
	virtual int f () const = 0;
};
struct SmallDerived : public Base {
	int i_;
	SmallDerived (int i = 0) noexcept : i_ (i) {}
	int f () const noexcept override { return i_; }
};
struct BigDerived : public Base {
	int a_[4];
	int f () const noexcept override {
		return -1;
	}
};

TEST_CASE ("test") {
	duck::SmallUniquePtr<Base, sizeof (SmallDerived)> p;
	CHECK (p.get () == nullptr);
	CHECK (!p);

	p.emplace<SmallDerived> (42);
	CHECK (p);
	CHECK (p.is_inline ());
	CHECK (!p.is_allocated ());
	CHECK (p->f () == 42);

	p.emplace<BigDerived> ();
	CHECK (p);
	CHECK (!p.is_inline ());
	CHECK (p.is_allocated ());
	CHECK (p->f () == -1);

	// TODO continue
}
