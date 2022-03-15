#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <duck/frozen_ptr.h>

struct Base {
	virtual ~Base () = default;
};
struct Derived : Base {};

TEST_CASE ("FreezableUniquePtr") {
	auto empty = duck::FreezablePtr<int>{};
	CHECK (!empty);

	auto p = duck::make_freezable<int> (42);
	CHECK (p);
	CHECK (*p == 42);
	*p = 3;
	CHECK (*p == 3);

	auto p2 = std::move (p);
	CHECK (p2);
	CHECK (!p);
	CHECK (*p2 == 3);

	auto derived = duck::make_freezable<Derived> ();
	duck::FreezablePtr<Base> base{std::move (derived)};
}

TEST_CASE ("FrozenSharedPtr") {
	auto p = duck::make_freezable<int> (44);
	auto sp = std::move (p).freeze ();
	CHECK (!p);
	CHECK (sp);
	CHECK (*sp == 44);

	auto spcpy = sp;
	CHECK (sp);
	CHECK (spcpy);
	CHECK (*sp == 44);
	CHECK (*spcpy == 44);
	CHECK (sp.get () == spcpy.get ());

	duck::FrozenPtr<Base> basep{duck::make_freezable<Derived> ().freeze ()};
}
