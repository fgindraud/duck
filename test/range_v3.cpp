#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/range_v3.h>
#include <forward_list>
#include <iterator>
#include <list>
#include <type_traits>
#include <vector>

/* Basic sanity test for overloading.
 */
namespace {
template <typename T> std::true_type is_lvalue_ref (T &);
template <typename T, typename = duck::enable_if_t<!std::is_reference<T>::value>>
std::false_type is_lvalue_ref (T &&);
int i = 0;
static_assert (decltype (is_lvalue_ref (i))::value, "");
static_assert (decltype (is_lvalue_ref (static_cast<const int &> (i)))::value, "");
static_assert (decltype (is_lvalue_ref (static_cast<int &> (i)))::value, "");
static_assert (!decltype (is_lvalue_ref (42))::value, "");
} // namespace

/* Define multiple type of containers to test with range.
 * Containers and expected properties are encoded as a struct.
 * -> how to build instances
 * -> expected types
 */

/* Integer vector test case.
 */
struct int_vector {
	// Expected types
	using range_type = std::vector<int>;
	using mutable_iterator = typename range_type::iterator;
	using const_iterator = typename range_type::const_iterator;
	using has_empty = std::true_type;
	using has_size = std::true_type;
	using size_type = typename range_type::size_type;

	// Create instances
	static range_type make_empty () { return range_type{}; }
	static range_type make_0_4 () { return range_type{0, 1, 2, 3, 4}; }
};
TYPE_TO_STRING (int_vector);

/* Integer list test case.
 */
struct int_list {
	// Expected types
	using range_type = std::list<int>;
	using mutable_iterator = typename range_type::iterator;
	using const_iterator = typename range_type::const_iterator;
	using has_empty = std::true_type;
	using has_size = std::true_type;
	using size_type = typename range_type::size_type;

	// Create instances
	static range_type make_empty () { return range_type{}; }
	static range_type make_0_4 () { return range_type{0, 1, 2, 3, 4}; }
};
TYPE_TO_STRING (int_list);

/* Integer forward_list test case.
 */
struct int_forward_list {
	// Expected types
	using range_type = std::forward_list<int>;
	using mutable_iterator = typename range_type::iterator;
	using const_iterator = typename range_type::const_iterator;
	using has_empty = std::true_type;
	using has_size = std::false_type;
	using size_type = typename range_type::difference_type;

	// Create instances
	static range_type make_empty () { return range_type{}; }
	static range_type make_0_4 () { return range_type{0, 1, 2, 3, 4}; }
};
TYPE_TO_STRING (int_forward_list);

/* Basic int range, which uses adl_ to call begin and end.
 */
namespace test_adl {
struct dummy_int_range {
	int end_;
};
struct dummy_int_iterator {
	using iterator_category = std::forward_iterator_tag;
	using value_type = int;
	using reference = int;
	using difference_type = int;
	using pointer = void;

	int i;

	int operator* () const { return i; }
	dummy_int_iterator & operator++ () { return ++i, *this; }
	bool operator== (dummy_int_iterator o) const { return i == o.i; }
	bool operator!= (dummy_int_iterator o) const { return i != o.i; }
};
inline dummy_int_iterator begin (dummy_int_range) {
	return {0};
}
inline dummy_int_iterator end (dummy_int_range r) {
	return {r.end_};
}
} // namespace test_adl
struct adl_dummy_int_range {
	using range_type = test_adl::dummy_int_range;
	using const_iterator = test_adl::dummy_int_iterator;
	using mutable_iterator = test_adl::dummy_int_iterator;
	using has_empty = std::false_type;
	using has_size = std::false_type;
	using size_type = typename test_adl::dummy_int_iterator::difference_type;

	static range_type make_empty () { return {0}; }
	static range_type make_0_4 () { return {5}; }
};
TYPE_TO_STRING (adl_dummy_int_range);

// List of tested type cases
using AllRangeTypes = doctest::Types<int_vector, int_list, int_forward_list, adl_dummy_int_range>;
using BidirRangeTypes = doctest::Types<int_vector, int_list, adl_dummy_int_range>;

/* Template test cases.
 * Test properties like returned types, and properties on iterators.
 * Tested for const T&, T&, with empty or non empty T.
 */
TEST_CASE_TEMPLATE ("typedefs", C, AllRangeTypes) {
	auto empty = C::make_empty ();
	static_assert (std::is_same<decltype (empty), typename C::range_type>::value,
	               "empty C is of unexpected type");
	auto r_0_4 = C::make_0_4 ();
	static_assert (std::is_same<decltype (r_0_4), typename C::range_type>::value,
	               "0_4 C is of unexpected type");
	static_assert (duck::is_range<decltype (r_0_4)>::value, "0_4 C is not a range");
	static_assert (duck::has_empty_method<decltype (empty)>::value == C::has_empty::value,
	               "has_empty_method trait failed");
	static_assert (duck::has_size_method<decltype (empty)>::value == C::has_size::value,
	               "has_size_method trait failed");
}

TEST_CASE_TEMPLATE ("const C& - begin/end/empty/size/front", C, AllRangeTypes) {
	{
		auto empty_range = C::make_empty ();
		const auto & empty_ref = empty_range;
		auto b = duck::adl_begin (empty_ref);
		auto e = duck::adl_end (empty_ref);
		static_assert (std::is_same<decltype (b), typename C::const_iterator>::value,
		               "begin (const C &) is not C::const_iterator");
		CHECK (b == e);
		CHECK (duck::empty (empty_ref));
		auto s = duck::size (empty_ref);
		static_assert (std::is_same<decltype (s), typename C::size_type>::value,
		               "size (const C &) is not C::size_type");
		CHECK (s == 0);
	}
	{
		auto non_empty_range = C::make_0_4 ();
		const auto & ref = non_empty_range;
		auto b = duck::adl_begin (ref);
		auto e = duck::adl_end (ref);
		CHECK (b != e);
		CHECK_FALSE (duck::empty (ref));
		CHECK (duck::size (ref) == 5);
		static_assert (std::is_same<decltype (duck::front (ref)),
		                            duck::iterator_reference_t<typename C::const_iterator>>::value,
		               "front(const C&) is not C::const_iterator::reference");
		CHECK (duck::front (ref) == 0);
	}
}
TEST_CASE_TEMPLATE ("C& - begin/end/empty/size/front", C, AllRangeTypes) {
	{
		auto empty_range = C::make_empty ();
		auto & empty_ref = empty_range;
		auto b = duck::adl_begin (empty_ref);
		auto e = duck::adl_end (empty_ref);
		static_assert (std::is_same<decltype (b), typename C::mutable_iterator>::value,
		               "begin (C &) is not C::mutable_iterator");
		CHECK (b == e);
		CHECK (duck::empty (empty_ref));
		auto s = duck::size (empty_ref);
		static_assert (std::is_same<decltype (s), typename C::size_type>::value,
		               "size (C &) is not C::size_type");
		CHECK (s == 0);
	}
	{
		auto non_empty_range = C::make_0_4 ();
		auto & ref = non_empty_range;
		auto b = duck::adl_begin (ref);
		auto e = duck::adl_end (ref);
		CHECK (b != e);
		CHECK_FALSE (duck::empty (ref));
		CHECK (duck::size (ref) == 5);
		static_assert (std::is_same<decltype (duck::front (ref)),
		                            duck::iterator_reference_t<typename C::mutable_iterator>>::value,
		               "front(C&) is not C::mutable_iterator::reference");
		CHECK (duck::front (ref) == 0);
	}
}
TEST_CASE_TEMPLATE ("C&& - empty/size/front", C, AllRangeTypes) {
	{
		CHECK (duck::empty (C::make_empty ()));
		auto s = duck::size (C::make_empty ());
		static_assert (std::is_same<decltype (s), typename C::size_type>::value,
		               "size (C &&) is not C::size_type");
		CHECK (s == 0);
	}
	{
		CHECK_FALSE (duck::empty (C::make_0_4 ()));
		CHECK (duck::size (C::make_0_4 ()) == 5);
		static_assert (std::is_same<decltype (duck::front (C::make_0_4 ())),
		                            duck::iterator_value_type_t<typename C::const_iterator>>::value,
		               "front(C&) is not C::mutable_iterator::reference");
		CHECK (duck::front (C::make_0_4 ()) == 0);
	}
}

	// TODO back, in separate test case for bidirs only

#if 0
TEST_CASE ("integer iterator") {
	auto it = duck::Range::IntegerIterator<int>{42};
	CHECK (*it == 42);
	CHECK (it == it);
	auto it2 = it - 2;
	CHECK (it - it2 == 2);
	CHECK (it2 < it);
	it++;
	CHECK (*it == 43);
}

TEST_CASE ("integer range & range basic primitives") {
	auto r = duck::range (4, 10);
	CHECK (*r.begin () == 4);
	CHECK (r.front () == 4);
	CHECK (*r.end () == 10); // Not UB as IntegerIterator is friendly :)
	CHECK (r.back () == 9);
	CHECK (r[2] == 6);
	CHECK_FALSE (r.empty ());
	CHECK (r.size () == 6);
	CHECK (*r.at (1) == 5);
	CHECK (*r.at (-3) == 7);

	auto r2 = duck::range (0);
	CHECK (r2.empty ());
	CHECK (r2.size () == 0);

	CHECK (r2 == r2);
	CHECK_FALSE (r == r2);
	CHECK (r == r);
}

TEST_CASE ("container ref range") {
	std::vector<int> vec{0, 1, 2, 3, 4};
	auto vec_r = duck::range (vec);
	CHECK_FALSE (vec_r.empty ());
	CHECK (vec_r.size () == 5);
	CHECK (vec_r.begin () == vec.begin ());
	CHECK (vec_r.end () == vec.end ());
	*vec_r.begin () = 42;
	CHECK (vec[0] == 42);
	CHECK (std::equal (vec.begin (), vec.end (), vec_r.begin ()));
	CHECK (vec_r.to_container<std::vector<int>> () == vec);
}

TEST_CASE ("container value range") {
	auto r = duck::range ({1, 2, 3, 4});
	CHECK_FALSE (r.empty ());
	CHECK (r.size () == 4);
	CHECK (*r.begin () == 1);
}

TEST_CASE ("strings & arrays") {
	auto & literal = "hello world";

	auto s = std::string{literal};

	// String literal is just an array for range
	auto r_lit = duck::range (literal);
	CHECK (r_lit.size () == s.size () + 1); // Includes '\0'

	// Use char_range to consider it as null_terminated
	auto cr_lit = duck::char_range (literal);
	CHECK (cr_lit.size () == s.size ());
}
#endif
