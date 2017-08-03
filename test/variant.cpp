#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/variant.h>
#include <iostream>
#include <string>

struct blah {};
inline std::ostream & operator<< (std::ostream & os, const blah &) {
	return os << "blah!";
}

struct PrintVisitor {
	template <typename T> void operator() (const T & t) const { std::cout << t << '\n'; }
};

TEST_CASE ("test") {
	using Var = duck::Variant::Static<bool, int, blah, std::string>;
	CHECK (Var::index_for_type<bool> () == 0);
	CHECK (Var::index_for_type<int> () == 1);
	CHECK (Var::index_for_type<blah> () == 2);

	Var a{blah{}};
	Var b{32};
	Var c{duck::InPlace<std::string>{}, "hello"};
	b.visit (PrintVisitor{});
	c.visit (PrintVisitor{});
}

TEST_CASE ("dynamic") {
	// WIP
	using MyVariant = duck::Variant::Dynamic<sizeof (long), alignof (long)>;
	MyVariant z{42};
}
