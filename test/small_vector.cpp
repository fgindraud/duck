#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/small_vector.h>

void notice_this (duck::SmallVectorBase<int> & v) {
	(void) v;
}

template <typename SV> std::ptrdiff_t inline_storage_offset (const SV & sv) {
	return reinterpret_cast<const char *> (sv.inline_storage_ptr ()) -
	       reinterpret_cast<const char *> (&sv);
}

TEST_CASE ("is inline buffer offset constant") {
	duck::SmallVector<int, 1> vec1;
	duck::SmallVector<int, 2> vec2;
	duck::SmallVector<int, 10> vec10;
	CHECK (inline_storage_offset (vec1) == inline_storage_offset (vec2));
	CHECK (inline_storage_offset (vec1) == inline_storage_offset (vec10));
}

TEST_CASE ("growing and resizing with trivial type (int)") {
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

struct MoveConstrOnlyWithCount {
	MoveConstrOnlyWithCount () = default;
	MoveConstrOnlyWithCount (MoveConstrOnlyWithCount && a) : moved (a.moved + 1) {}
	int moved{0};
};

TEST_CASE ("move constructible only type") {
	duck::SmallVector<MoveConstrOnlyWithCount, 2> v;

	// Push back temporary, should move one time
	v.push_back (MoveConstrOnlyWithCount{});
	CHECK (v.front ().moved == 1);

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
