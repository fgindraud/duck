#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/debug.h>
#include <duck/small_unique_ptr.h>

struct Base : public virtual duck::SmallUniquePtrMakeMovableBase<Base>, public duck::Noisy<'B'> {
	virtual ~Base () = default;
	virtual int f () const = 0;
};
struct SmallDerived : public Base,
                      public duck::SmallUniquePtrMakeMovable<SmallDerived, Base>,
                      public duck::Noisy<'d'> {
	int i_;
	SmallDerived (int i = 0) noexcept : i_ (i) {}
	int f () const noexcept override { return i_; }
};
struct BigDerived : public Base,
                    public duck::SmallUniquePtrMakeMovable<BigDerived, Base>,
                    public duck::Noisy<'D'> {
	int a_[4];
	int f () const noexcept override { return -1; }
};

TEST_CASE ("test") {
	duck::SmallUniquePtr<Base, sizeof (SmallDerived)> p;
	CHECK (p.get () == nullptr);
	CHECK (!p);

	// Reset in place
	p.reset (duck::InPlace<SmallDerived>{}, 42);
	CHECK (p);
	CHECK (p.is_inline ());
	CHECK (!p.is_allocated ());
	CHECK (p->f () == 42);

	// Emplace is an alternative
	p.emplace<BigDerived> ();
	CHECK (p);
	CHECK (!p.is_inline ());
	CHECK (p.is_allocated ());
	CHECK (p->f () == -1);

	// Construction
	duck::SmallUniquePtr<Base, sizeof (SmallDerived)> p2{duck::InPlace<SmallDerived>{}, 4};
	CHECK (p2);
	CHECK (p2.is_inline ());
	CHECK (p2->f () == 4);

	// Move construction
	duck::SmallUniquePtr<Base, sizeof (SmallDerived) / 2> p3{std::move (p2)};
	CHECK (p3);
	// p2 state is unspecified
	CHECK (p3.is_allocated ()); // Too small for inplace
	CHECK (p3->f () == 4);

	// Move assign
	p2 = std::move (p3);
	CHECK (p2);
	CHECK (p2->f () == 4);
}
