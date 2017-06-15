#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/optional.h>
#include <memory> // tests with uniq_ptr
#include <string> // to_string
#include <type_traits>

TEST_CASE ("basic operations") {
	duck::Optional<int> a;
	CHECK_FALSE (a);

	a = 42;
	CHECK (a);
	CHECK (a.value () == 42);

	a.reset ();
	CHECK_FALSE (a);

	a = 4;
	CHECK (a);
	CHECK (*a == 4);
	a = duck::nullopt;
	CHECK (!a);

	// Copy construction
	a = 54;
	duck::Optional<int> b{a};
	CHECK (b);
	CHECK (*b == 54);

	// Copy assign
	b = -1;
	a.reset ();
	CHECK (!a);
	CHECK (b);
	CHECK (*b == -1);
	a = b;
	CHECK (a);
	CHECK (*a == -1);

	// Null optional copy
	a = duck::nullopt;
	CHECK (!a);
	b = a;
	CHECK (!b);

	duck::Optional<int> c{b};
	CHECK (!c);
}

template <typename T> using IsOptionalInt = std::is_same<T, duck::Optional<int>>;

TEST_CASE ("value_or_*, map, filter") {
	const duck::Optional<int> valued{42};
	const duck::Optional<int> valued2{33};
	const duck::Optional<int> empty;

	CHECK (valued.value_or (1) == 42);
	CHECK (empty.value_or (1) == 1);

	// Detect when lambda is called for value_or_generate
	bool generate_triggered = false;
	auto generate = [&generate_triggered] {
		generate_triggered = true;
		return 21;
	};
	CHECK (valued.value_or_generate (generate) == 42);
	CHECK (!generate_triggered);
	CHECK (empty.value_or_generate (generate) == 21);
	CHECK (generate_triggered);

	// Detect when lambda is called for map
	bool map_func_triggered = false;
	auto map_fun = [&map_func_triggered](int a) {
		map_func_triggered = true;
		return -a;
	};
	auto mapped_empty = empty.map (map_fun);
	CHECK (IsOptionalInt<decltype (mapped_empty)>::value);
	CHECK (!mapped_empty);
	CHECK (!map_func_triggered);
	auto mapped_valued = valued.map (map_fun);
	CHECK (IsOptionalInt<decltype (mapped_valued)>::value);
	CHECK (mapped_valued);
	CHECK (*mapped_valued == -42);
	CHECK (map_func_triggered);

	// Chainable
	auto double_input = [](int a) { return 2 * a; };
	auto to_string = [](int a) { return std::to_string (a); };
	auto chained_empty = empty.map (double_input).map (to_string);
	CHECK (!chained_empty);
	auto chained_valued = valued.map (double_input).map (to_string);
	CHECK (chained_valued);
	CHECK (*chained_valued == "84");

	// Filter
	auto predicate = [](int a) { return a >= 40; };
	CHECK (!empty.filter (predicate));
	CHECK (!valued2.filter (predicate));
	CHECK (valued.filter (predicate));
	CHECK (valued.filter (predicate).value () == 42);
}

// Non movable nor copyable type, shoud support emplace stuff
struct OnlyConstructible {
	int a;
	OnlyConstructible (int i) : a (i) {}
	OnlyConstructible (const OnlyConstructible &) = delete;
};

TEST_CASE ("non move/copy objects") {
	duck::Optional<OnlyConstructible> a{duck::in_place, 32};
	CHECK (a);
	CHECK (a->a == 32);

	a.emplace (12);
	CHECK (a);
	CHECK (a->a == 12);
}

// Movable only type
TEST_CASE ("move only objects") {
	using UniqP = std::unique_ptr<int>;
	duck::Optional<UniqP> p;
	CHECK (!p);

	// Move value in
	p = UniqP{new int{42}};
	CHECK (p);
	CHECK (**p == 42);

	// Move construct
	duck::Optional<UniqP> p2{std::move (p)};
	CHECK (p);   // Still defined
	CHECK (!*p); // Moved from
	CHECK (p2);
	CHECK (**p2 == 42);

	// Move assign
	p = std::move (p2);
	CHECK (p2);   // Defined
	CHECK (!*p2); // Moved from
	CHECK (p);
	CHECK (**p == 42);

	// Move value out
	UniqP up = *std::move (p);
	CHECK (p);
	CHECK (!*p);
	CHECK (up);
	CHECK (*up == 42);

	// Move value in
	p = std::move (up);
	CHECK (!up);
	CHECK (*p);
	CHECK (**p == 42);

	// Check value_or
	p2.reset ();
	up = std::move (p2).value_or (UniqP{});
	CHECK (!up); // Was empty
	CHECK (p);
	CHECK (*p);
	up = std::move (p).value_or (UniqP{});
	CHECK (up);
	CHECK (*up == 42);

	// Check map (chainable)
	p = std::move (up);
	CHECK (p);
	CHECK (*p);
	CHECK (**p == 42);
	auto incr = [](UniqP && tp) {
		(*tp)++;
		return std::move (tp);
	};
	p2 = std::move (p).map (incr).map (incr);
	CHECK (p2);
	CHECK (*p2);
	CHECK (**p2 == 44);
	CHECK (p);
	CHECK (!*p);
}

TEST_CASE ("constness") {
	// Can change value of const Opt<T>
	const duck::Optional<int> const_opt{23};
	*const_opt = 42;
	CHECK (*const_opt == 42);

	// Can modify the value only by reconstructing it for Opt<const T>
	duck::Optional<const int> opt_const;
	CHECK (!opt_const);
	opt_const = 42; // Automatically uses reconstruction
	CHECK (opt_const);
	CHECK (*opt_const == 42);
	opt_const.emplace(33);
	CHECK (*opt_const == 33);
	opt_const.reset ();
	CHECK (!opt_const);
}

TEST_CASE ("operator|") {
	duck::Optional<int> empty;
	duck::Optional<int> a {42};
	duck::Optional<int> b {24};

	auto c = empty | a | b;
	CHECK (c);
	CHECK (*c == 42);

	using Or_Of_LVRef_LVRef_is_LVRef = std::is_lvalue_reference<decltype (empty | a)>;
	CHECK (Or_Of_LVRef_LVRef_is_LVRef::value);

	using Or_Of_RVRef_RVRef_is_RVRef = std::is_rvalue_reference<decltype (std::move (empty) | std::move (a))>;
	CHECK (Or_Of_RVRef_RVRef_is_RVRef::value);
}
