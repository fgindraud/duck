#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <array>
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
	using Data = std::array<int, 10>;
	Data d_;
	BigDerived (const Data & d) : d_ (d) {}
	int f () const noexcept override {
		int acc = 0;
		for (auto i : d_)
			acc += i;
		return acc;
	}
};

TEST_CASE ("test") {
	duck::SmallUniquePtr<Base, 2 * sizeof (void *)> p;
	CHECK (p.get () == nullptr);
	CHECK (!p);
	CHECK (!p.is_allocated ());

	p.emplace<SmallDerived> (42);
	CHECK (p);
	CHECK (!p.is_allocated ());
	CHECK (p->f() == 42);

	// fails for now (expected behavior) p.emplace<BigDerived> ({1, 2, 3, 4, 5, 6, 7, 8, 9, 0});

	// TODO continue
}
