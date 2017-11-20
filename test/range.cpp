#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/range/range.h>
#include <forward_list>
#include <iterator>
#include <list>
#include <type_traits>
#include <vector>

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
	using size_type = typename const_iterator::difference_type;
	// Create values
	static range_type make_empty () { return range_type{}; }
	static range_type make_0_4 () { return range_type{0, 1, 2, 3, 4}; }
};
TYPE_TO_STRING (int_vector);

/* Integer list test case.
 */
struct int_list {
	using range_type = std::list<int>;
	using mutable_iterator = typename range_type::iterator;
	using const_iterator = typename range_type::const_iterator;
	using has_empty = std::true_type;
	using has_size = std::true_type;
	using size_type = typename const_iterator::difference_type;

	static range_type make_empty () { return range_type{}; }
	static range_type make_0_4 () { return range_type{0, 1, 2, 3, 4}; }
};
TYPE_TO_STRING (int_list);

/* Integer forward_list test case.
 */
struct int_forward_list {
	using range_type = std::forward_list<int>;
	using mutable_iterator = typename range_type::iterator;
	using const_iterator = typename range_type::const_iterator;
	using has_empty = std::true_type;
	using has_size = std::false_type;
	using size_type = typename const_iterator::difference_type;

	static range_type make_empty () { return range_type{}; }
	static range_type make_0_4 () { return range_type{0, 1, 2, 3, 4}; }
};
TYPE_TO_STRING (int_forward_list);

/* Integer range.
 */
struct int_range {
	using range_type = duck::iterator_pair<duck::integer_iterator<int>>;
	using mutable_iterator = duck::integer_iterator<int>;
	using const_iterator = duck::integer_iterator<int>;
	using has_empty = std::false_type;
	using has_size = std::false_type;
	using size_type = duck::iterator_difference_t<const_iterator>;

	static auto make_empty () -> decltype (duck::range (0)) { return duck::range (0); }
	static auto make_0_4 () -> decltype (duck::range (5)) { return duck::range (5); }
};
TYPE_TO_STRING (int_range);

/* Basic int range, which uses adl_ to call begin and end.
 * Can be considered a lvalue_range, but not flagged as one.
 */
namespace test_adl {
struct dummy_int_range {
	int end_;
};
struct dummy_int_iterator {
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = int;
	using reference = int;
	using difference_type = int;
	using pointer = void;

	int i;

	int operator* () const { return i; }
	dummy_int_iterator & operator++ () { return ++i, *this; }
	dummy_int_iterator & operator-- () { return --i, *this; }
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

/* Raw array as iterator_pair
 */
int raw_array[5] = {0, 1, 2, 3, 4};
struct pointer_pair {
	using range_type = duck::iterator_pair<int *>;
	using mutable_iterator = int *;
	using const_iterator = int *;
	using has_empty = std::false_type;
	using has_size = std::false_type;
	using size_type = duck::iterator_difference_t<int *>;

	static auto make_empty () -> decltype (duck::range (raw_array, raw_array)) {
		return duck::range (raw_array, raw_array);
	}
	static auto make_0_4 () -> decltype (duck::range (raw_array, raw_array + 5)) {
		return duck::range (raw_array, raw_array + 5);
	}
};
TYPE_TO_STRING (pointer_pair);

/* initializer list.
 * maps to an int_vector...
 */
struct int_init_list : int_vector {
	static auto make_empty () -> decltype (duck::range (std::initializer_list<int>{})) {
		return duck::range (std::initializer_list<int>{});
	}
	static auto make_0_4 () -> decltype (duck::range ({0, 1, 2, 3, 4})) {
		return duck::range ({0, 1, 2, 3, 4});
	}
};
TYPE_TO_STRING (int_init_list);

/* wrapper to const&, &, &&
 */
std::vector<int> empty_int_vector{};
std::vector<int> non_empty_int_vector{0, 1, 2, 3, 4};
const auto & empty_int_vector_const_ref = empty_int_vector;
const auto & non_empty_int_vector_const_ref = non_empty_int_vector;
auto & empty_int_vector_ref = empty_int_vector;
auto & non_empty_int_vector_ref = non_empty_int_vector;

struct const_lvalue_wrapper {
	using range_type = duck::range_object_wrapper<const std::vector<int> &>;
	using const_iterator = typename std::vector<int>::const_iterator;
	using mutable_iterator = typename std::vector<int>::const_iterator;
	using has_empty = std::true_type;
	using has_size = std::true_type;
	using size_type = typename const_iterator::difference_type;

	static auto make_empty () -> decltype (duck::range_object (empty_int_vector_const_ref)) {
		return duck::range_object (empty_int_vector_const_ref);
	}
	static auto make_0_4 () -> decltype (duck::range_object (non_empty_int_vector_const_ref)) {
		return duck::range_object (non_empty_int_vector_const_ref);
	}
};
TYPE_TO_STRING (const_lvalue_wrapper);
struct lvalue_wrapper {
	using range_type = duck::range_object_wrapper<std::vector<int> &>;
	using const_iterator = typename std::vector<int>::iterator;
	using mutable_iterator = typename std::vector<int>::iterator;
	using has_empty = std::true_type;
	using has_size = std::true_type;
	using size_type = typename const_iterator::difference_type;

	static auto make_empty () -> decltype (duck::range_object (empty_int_vector_ref)) {
		return duck::range_object (empty_int_vector_ref);
	}
	static auto make_0_4 () -> decltype (duck::range_object (non_empty_int_vector_ref)) {
		return duck::range_object (non_empty_int_vector_ref);
	}
};
TYPE_TO_STRING (lvalue_wrapper);
struct rvalue_wrapper {
	using range_type = duck::range_object_wrapper<std::vector<int>>;
	using const_iterator = typename std::vector<int>::const_iterator;
	using mutable_iterator = typename std::vector<int>::const_iterator;
	using has_empty = std::true_type;
	using has_size = std::true_type;
	using size_type = typename const_iterator::difference_type;

	static auto make_empty () -> decltype (duck::range_object (int_vector::make_empty ())) {
		return duck::range_object (int_vector::make_empty ());
	}
	static auto make_0_4 () -> decltype (duck::range_object (int_vector::make_0_4 ())) {
		return duck::range_object (int_vector::make_0_4 ());
	}
};
TYPE_TO_STRING (rvalue_wrapper);

// List of tested type cases
using all_range_types = doctest::Types<int_vector, int_list, int_forward_list, int_range,
                                       adl_dummy_int_range, int_init_list, const_lvalue_wrapper,
                                       lvalue_wrapper, rvalue_wrapper, pointer_pair>;
using bidir_range_types =
    doctest::Types<int_vector, int_list, int_range, adl_dummy_int_range, int_init_list,
                   const_lvalue_wrapper, lvalue_wrapper, rvalue_wrapper, pointer_pair>;

/* Template test cases.
 * Test properties like returned types, and properties on iterators.
 * Tested for const T&, T&, with empty or non empty T.
 */
TEST_CASE_TEMPLATE ("typedefs", C, all_range_types) {
	auto empty = C::make_empty ();
	static_assert (std::is_same<decltype (empty), typename C::range_type>::value,
	               "empty C is of unexpected type");
	auto r_0_4 = C::make_0_4 ();
	static_assert (std::is_same<decltype (r_0_4), typename C::range_type>::value,
	               "0_4 C is of unexpected type");

	using RT = decltype (empty);
	static_assert (duck::is_range<RT>::value, "0_4 C is not a range");
	static_assert (duck::is_range<const RT &>::value, "0_4 const C& is not a range");
	static_assert (duck::is_range<RT &>::value, "0_4 C& is not a range");

	static_assert (duck::has_empty_method<RT>::value == C::has_empty::value,
	               "has_empty_method trait failed");
	static_assert (duck::has_size_method<RT>::value == C::has_size::value,
	               "has_size_method trait failed");
}

TEST_CASE_TEMPLATE ("const C& - begin/end/empty/size/front/nth/contains", C, all_range_types) {
	{
		auto empty_range = C::make_empty ();
		const auto & empty_ref = empty_range;
		auto b = duck::adl_begin (empty_ref);
		auto e = duck::adl_end (empty_ref);
		static_assert (std::is_same<decltype (b), typename C::const_iterator>::value,
		               "begin (const C&) is not C::const_iterator");
		CHECK (b == e);
		CHECK (duck::empty (empty_ref));
		auto s = duck::size (empty_ref);
		static_assert (std::is_same<decltype (s), typename C::size_type>::value,
		               "size (const C&) is not C::size_type");
		CHECK (s == 0);
		CHECK_FALSE (duck::contains (empty_ref, b));
		CHECK_FALSE (duck::contains (empty_ref, e));
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
		static_assert (std::is_same<decltype (duck::nth (ref, 2)),
		                            duck::iterator_reference_t<typename C::const_iterator>>::value,
		               "nth(const C&) is not C::const_iterator::reference");
		CHECK (duck::nth (ref, 2) == 2);
		CHECK (duck::contains (ref, b));
		CHECK (duck::contains (ref, std::next (b)));
		CHECK_FALSE (duck::contains (ref, e));
	}
}
TEST_CASE_TEMPLATE ("C& - begin/end/empty/size/front/nth/contains", C, all_range_types) {
	{
		auto empty_range = C::make_empty ();
		auto & empty_ref = empty_range;
		auto b = duck::adl_begin (empty_ref);
		auto e = duck::adl_end (empty_ref);
		static_assert (std::is_same<decltype (b), typename C::mutable_iterator>::value,
		               "begin (C&) is not C::mutable_iterator");
		CHECK (b == e);
		CHECK (duck::empty (empty_ref));
		auto s = duck::size (empty_ref);
		static_assert (std::is_same<decltype (s), typename C::size_type>::value,
		               "size (C&) is not C::size_type");
		CHECK (s == 0);
		CHECK_FALSE (duck::contains (empty_ref, b));
		CHECK_FALSE (duck::contains (empty_ref, e));
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
		static_assert (std::is_same<decltype (duck::nth (ref, 2)),
		                            duck::iterator_reference_t<typename C::mutable_iterator>>::value,
		               "nth(C&) is not C::mutable_iterator::reference");
		CHECK (duck::nth (ref, 2) == 2);
		CHECK (duck::contains (ref, b));
		CHECK (duck::contains (ref, std::next (b)));
		CHECK_FALSE (duck::contains (ref, e));
	}
}
TEST_CASE_TEMPLATE ("C&& - begin/end/empty/size/front/nth", C, all_range_types) {
	{
		auto b = duck::adl_begin (C::make_empty ());
		auto e = duck::adl_end (C::make_empty ()); // Cannot compare, may not be the same object
		static_assert (std::is_same<decltype (b), typename C::const_iterator>::value,
		               "begin (C&&) is not C::const_iterator");
		static_assert (std::is_same<decltype (e), typename C::const_iterator>::value,
		               "end (C&&) is not C::const_iterator");
		CHECK (duck::empty (C::make_empty ()));
		auto s = duck::size (C::make_empty ());
		static_assert (std::is_same<decltype (s), typename C::size_type>::value,
		               "size (C&&) is not C::size_type");
		CHECK (s == 0);
	}
	{
		CHECK_FALSE (duck::empty (C::make_0_4 ()));
		CHECK (duck::size (C::make_0_4 ()) == 5);
		static_assert (std::is_same<decltype (duck::front (C::make_0_4 ())),
		                            duck::iterator_reference_t<typename C::const_iterator>>::value,
		               "front(C&&) is not C::const_iterator::reference");
		CHECK (duck::front (C::make_0_4 ()) == 0);
		static_assert (std::is_same<decltype (duck::nth (C::make_0_4 (), 2)),
		                            duck::iterator_reference_t<typename C::const_iterator>>::value,
		               "nth(C&&) is not C::const_iterator::reference");
		CHECK (duck::nth (C::make_0_4 (), 2) == 2);
	}
}

TEST_CASE_TEMPLATE ("const C& - back", C, bidir_range_types) {
	auto non_empty_range = C::make_0_4 ();
	const auto & ref = non_empty_range;
	static_assert (std::is_same<decltype (duck::back (ref)),
	                            duck::iterator_reference_t<typename C::const_iterator>>::value,
	               "back(const C&) is not C::const_iterator::reference");
	CHECK (duck::back (ref) == 4);
}
TEST_CASE_TEMPLATE ("C& - back", C, bidir_range_types) {
	auto non_empty_range = C::make_0_4 ();
	auto & ref = non_empty_range;
	static_assert (std::is_same<decltype (duck::back (ref)),
	                            duck::iterator_reference_t<typename C::mutable_iterator>>::value,
	               "back(C&) is not C::mutable_iterator::reference");
	CHECK (duck::back (ref) == 4);
}
TEST_CASE_TEMPLATE ("C&& - back", C, bidir_range_types) {
	static_assert (std::is_same<decltype (duck::back (C::make_0_4 ())),
	                            duck::iterator_reference_t<typename C::const_iterator>>::value,
	               "back(C&&) is not C::const_iterator::reference");
	CHECK (duck::back (C::make_0_4 ()) == 4);
}

TEST_CASE ("integer iterator") {
	auto it = duck::integer_iterator<int>{42};
	CHECK (*it == 42);
	CHECK (it == it);
	auto it2 = it - 2;
	CHECK (it - it2 == 2);
	CHECK (it2 < it);
	it++;
	CHECK (*it == 43);
}
