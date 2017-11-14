#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/range.h>
#include <duck/small_vector.h>
#include <list>

template <typename SV> std::ptrdiff_t inline_storage_offset (const SV & sv) {
	// Only works if non allocated, and data returns a ptr to inline storage
	return reinterpret_cast<const char *> (sv.data ()) - reinterpret_cast<const char *> (&sv);
}

TEST_CASE ("is inline buffer offset constant") {
	// Empty vec should use inline storage
	duck::SmallVector<int, 1> vec1;
	duck::SmallVector<int, 2> vec2;
	duck::SmallVector<int, 10> vec10;
	CHECK_FALSE (vec1.is_allocated ());
	CHECK_FALSE (vec2.is_allocated ());
	CHECK_FALSE (vec10.is_allocated ());

	CHECK (inline_storage_offset (vec1) == inline_storage_offset (vec2));
	CHECK (inline_storage_offset (vec1) == inline_storage_offset (vec10));
}

TEST_CASE ("push_back, resize, clear (T=int, testing size, capacity, is_allocated, access)") {
	duck::SmallVector<int, 2> a;
	CHECK (a.size () == 0);
	CHECK (a.empty ());
	CHECK (a.capacity () == 2);
	CHECK_FALSE (a.is_allocated ());

	a.push_back (42);
	CHECK (a.size () == 1);
	CHECK_FALSE (a.empty ());
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

	a.push_back (33); // Should reallocate
	CHECK (a.size () == 3);
	CHECK (a.capacity () > 2);
	CHECK (a.is_allocated ());
	CHECK (a.front () == 42);
	CHECK (a.back () == 33);
	CHECK (a[1] == -1);
	CHECK (a.at (2) == 33);
	CHECK_THROWS_AS (a.at (3), std::out_of_range);

	a.resize (5);
	CHECK (a.size () == 5);
	CHECK (a.capacity () >= 5);
	CHECK (a.is_allocated ());
	CHECK (a.front () == 42);
	CHECK (a.back () == 0);

	auto prev_cap = a.capacity ();
	auto prev_buf = a.data ();
	a.resize (1); // Should not reduce capacity
	CHECK (a.size () == 1);
	CHECK (a.capacity () == prev_cap);
	CHECK (a.data () == prev_buf);
	CHECK (a.is_allocated ());
	CHECK (a.front () == 42);
	CHECK (a.back () == 42);

	a.clear (); // Should not reduce capacity
	CHECK (a.size () == 0);
	CHECK (a.capacity () == prev_cap);
	CHECK (a.data () == prev_buf);
}

TEST_CASE ("shrink_to_fit") {
	duck::SmallVector<int, 2> v;
	CHECK (v.capacity () == 2);
	CHECK_FALSE (v.is_allocated ());

	v.shrink_to_fit ();
	CHECK (v.capacity () == 2);
	CHECK_FALSE (v.is_allocated ());

	v.reserve (10);
	CHECK (v.capacity () == 10);
	CHECK (v.is_allocated ());

	v.shrink_to_fit ();
	CHECK (v.capacity () == 2);
	CHECK_FALSE (v.is_allocated ());

	v.reserve (10);
	v.resize (8);
	CHECK (v.capacity () == 10);
	CHECK (v.is_allocated ());
	CHECK (v.size () == 8);

	v.shrink_to_fit ();
	CHECK (v.capacity () == 8);
	CHECK (v.is_allocated ());

	v.clear ();
	duck::SmallVectorBase<int> & base = v;
	base.shrink_to_fit ();
	CHECK (v.capacity () == duck::small_vector_minimum_inline_size);
	CHECK_FALSE (v.is_allocated ());
}

TEST_CASE ("assign") {
	duck::SmallVector<int, 2> v;
	v.assign (2, 4);
	CHECK (v.size () == 2);
	CHECK (v.capacity () == 2);
	CHECK_FALSE (v.is_allocated ());
	CHECK (v[0] == 4);
	CHECK (v[1] == 4);

	v.assign ({1, 2, 3, 4});
	CHECK (v.size () == 4);
	CHECK (v.capacity () == 4);
	CHECK (v.is_allocated ());
	for (auto i : duck::range (4))
		CHECK (v[i] == i + 1);

	std::initializer_list<int> ilist = {5, 4, 3, 2, 1};
	v.assign (ilist.begin (), ilist.end ());
	CHECK (v.size () == 5);
	CHECK (v.capacity () >= 5);
	CHECK (v.is_allocated ());
	for (auto i : duck::range (5))
		CHECK (v[i] == 5 - i);

	v.assign (1, 42);
	CHECK (v.size () == 1);
	CHECK (v.capacity () >= 5);
	CHECK (v.is_allocated ());

	// Test with a non Random Access iterator
	std::list<int> l{1, 2, 3, 4};
	v.assign (l.begin (), l.end ());
	CHECK (v.size () == 4);
	CHECK (v.is_allocated ());
	for (auto i : duck::range (4))
		CHECK (v[i] == i + 1);
}

struct MoveConstrOnlyWithCount {
	MoveConstrOnlyWithCount () = default;
	MoveConstrOnlyWithCount (MoveConstrOnlyWithCount && a) : moved (a.moved + 1) {}
	int moved{0};
};

TEST_CASE ("move constructible only type") {
	duck::SmallVector<MoveConstrOnlyWithCount, 2> v;

	// Push back temporary, should move one time
	v.push_back (MoveConstrOnlyWithCount{});
	CHECK (v[0].moved == 1);

	// Emplace, should not move newly created, nor the already existing one
	v.emplace_back ();
	CHECK (v[0].moved == 1);
	CHECK (v[1].moved == 0);

	// [0] and [1] will me moved once during resize
	// [2] will be built in place
	v.resize (3);
	CHECK (v[0].moved == 2);
	CHECK (v[1].moved == 1);
	CHECK (v[2].moved == 0);
}
