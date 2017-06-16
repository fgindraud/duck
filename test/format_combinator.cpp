#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <algorithm>
#include <duck/format/basic.h>
#include <duck/format/combinator.h>
#include <vector>

TEST_CASE ("truncated output iterator") {
	std::vector<int>::size_type sz = 40;
	std::vector<int> va (sz, 42);
	std::vector<int> vb (sz, 3);

	CHECK (std::all_of (va.begin (), va.end (), [](int a) { return a == 42; }));

	auto tit = duck::Iterator::TruncatedOutput<typename std::vector<int>::iterator>{va.begin (), 20};
	auto stopped = std::copy (vb.begin (), vb.end (), tit);
	CHECK (stopped.base () == va.begin () + 20);
	CHECK (std::all_of (va.begin (), va.begin () + 20, [](int a) { return a == 3; }));
	CHECK (std::all_of (va.begin () + 20, va.end (), [](int a) { return a == 42; }));
}

TEST_CASE ("ref") {
	const auto f = duck::format ('-');
	auto a = duck::Format::ref (f);
	CHECK (a.size () == f.size ());
	CHECK (a.to_string () == f.to_string ());
	CHECK (&a.formatter () == &f);
}

TEST_CASE ("repeated") {
	auto f = duck::Format::repeated (duck::format ('#'), 5);
	CHECK (f.to_string () == "#####");
}

TEST_CASE ("truncated") {
	auto hw = duck::format ("Hello world !");
	auto f = duck::Format::truncated (hw, 5);
	CHECK (f.size () == 5);
	CHECK (f.to_string () == "Hello");

	auto f2 = duck::Format::truncated (hw, 25);
	CHECK (f2.size () == hw.size ());
	CHECK (f2.to_string () == hw.to_string ());
}
