#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <vector>

#include <duck/range/algorithm.h>

using duck::range;

auto empty = duck::range (0);
auto all0 = std::vector<int>{0, 0, 0, 0};
auto all1 = std::vector<int>{1, 1, 1, 1};
auto ladder0_4 = duck::range (0, 4);

auto few0 = std::vector<int>{0, 0};

auto is_zero = [](int i) { return i == 0; };
auto is_equal = [](int l, int r) { return l == r; };

TEST_CASE ("all / any / none") {
	CHECK (duck::all_of (empty, is_zero));
	CHECK (duck::all_of (all0, is_zero));
	CHECK_FALSE (duck::all_of (all1, is_zero));
	CHECK_FALSE (duck::all_of (ladder0_4, is_zero));

	CHECK_FALSE (duck::any_of (empty, is_zero));
	CHECK (duck::any_of (all0, is_zero));
	CHECK_FALSE (duck::any_of (all1, is_zero));
	CHECK (duck::any_of (ladder0_4, is_zero));

	CHECK (duck::none_of (empty, is_zero));
	CHECK_FALSE (duck::none_of (all0, is_zero));
	CHECK (duck::none_of (all1, is_zero));
	CHECK_FALSE (duck::none_of (ladder0_4, is_zero));
}

TEST_CASE ("count") {
	CHECK (duck::count (empty, 0) == 0);
	CHECK (duck::count (all0, 0) == duck::size (all0));
	CHECK (duck::count (all1, 0) == 0);
	CHECK (duck::count (ladder0_4, 0) == 1);

	CHECK (duck::count_if (empty, is_zero) == 0);
	CHECK (duck::count_if (all0, is_zero) == duck::size (all0));
	CHECK (duck::count_if (all1, is_zero) == 0);
	CHECK (duck::count_if (ladder0_4, is_zero) == 1);
}

// TODO mismatch

TEST_CASE ("equal") {
	CHECK (duck::equal (empty, duck::begin (empty)));
	CHECK (duck::equal (empty, duck::begin (all0)));
	CHECK (duck::equal (all0, duck::begin (all0)));
	CHECK_FALSE (duck::equal (all0, duck::begin (ladder0_4)));

	CHECK (duck::equal (empty, duck::begin (empty), is_equal));
	CHECK (duck::equal (empty, duck::begin (all0), is_equal));
	CHECK (duck::equal (all0, duck::begin (all0), is_equal));
	CHECK_FALSE (duck::equal (all0, duck::begin (ladder0_4), is_equal));

#if HAS_CPP14
	CHECK (duck::equal (empty, empty));
	CHECK_FALSE (duck::equal (empty, all0));
	CHECK (duck::equal (all0, all0));
	CHECK_FALSE (duck::equal (all0, ladder0_4));

	CHECK (duck::equal (empty, empty, is_equal));
	CHECK_FALSE (duck::equal (empty, all0, is_equal));
	CHECK (duck::equal (all0, all0, is_equal));
	CHECK_FALSE (duck::equal (all0, ladder0_4, is_equal));
#endif
}

TEST_CASE ("find") {
	CHECK (duck::find (empty, 0) == duck::end (empty));
	CHECK (duck::find (all0, 0) == duck::begin (all0));
	CHECK (duck::find (all1, 0) == duck::end (all1));
	CHECK (duck::find (ladder0_4, 0) == duck::begin (ladder0_4));

	CHECK (duck::find_if (empty, is_zero) == duck::end (empty));
	CHECK (duck::find_if (all0, is_zero) == duck::begin (all0));
	CHECK (duck::find_if (all1, is_zero) == duck::end (all1));
	CHECK (duck::find_if (ladder0_4, is_zero) == duck::begin (ladder0_4));

	CHECK (duck::find_if_not (empty, is_zero) == duck::end (empty));
	CHECK (duck::find_if_not (all0, is_zero) == duck::end (all0));
	CHECK (duck::find_if_not (all1, is_zero) == duck::begin (all1));
	CHECK (duck::find_if_not (ladder0_4, is_zero) == duck::begin (ladder0_4) + 1);
}

TEST_CASE ("find end/first_of") {
	CHECK (duck::find_first_of (empty, few0) == duck::end (empty));
	CHECK (duck::find_first_of (all0, few0) == duck::begin (all0));
	CHECK (duck::find_first_of (all1, few0) == duck::end (all1));

	CHECK (duck::find_first_of (empty, few0, is_equal) == duck::end (empty));
	CHECK (duck::find_first_of (all0, few0, is_equal) == duck::begin (all0));
	CHECK (duck::find_first_of (all1, few0, is_equal) == duck::end (all1));

	CHECK (duck::find_end (empty, few0) == duck::end (empty));
	CHECK (duck::find_end (all0, few0) ==
	       duck::begin (all0) + (duck::size (all0) - duck::size (few0)));
	CHECK (duck::find_end (all1, few0) == duck::end (all1));

	CHECK (duck::find_end (empty, few0, is_equal) == duck::end (empty));
	CHECK (duck::find_end (all0, few0, is_equal) ==
	       duck::begin (all0) + (duck::size (all0) - duck::size (few0)));
	CHECK (duck::find_end (all1, few0, is_equal) == duck::end (all1));
}

// TODO adjacent_find

// TODO search
