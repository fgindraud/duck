#pragma once

// Overloads of <algorithm> functions to accept range arguments instead of iterator pairs.
// STATUS: WIP (missing part of <algorithm>), NSC

#include <algorithm>
#include <duck/range/range.h>

namespace duck {

// non modifying sequence operations

template <typename R, typename UnaryPredicate, typename = enable_if_t<is_range<const R>::value>>
bool all_of (const R & r, UnaryPredicate p) {
	return std::all_of (begin (r), end (r), p);
}
template <typename R, typename UnaryPredicate, typename = enable_if_t<is_range<const R>::value>>
bool none_of (const R & r, UnaryPredicate p) {
	return std::none_of (begin (r), end (r), p);
}
template <typename R, typename UnaryPredicate, typename = enable_if_t<is_range<const R>::value>>
bool any_of (const R & r, UnaryPredicate p) {
	return std::any_of (begin (r), end (r), p);
}

template <typename R, typename UnaryFunction, typename = enable_if_t<is_range<const R>::value>>
void for_each (const R & r, UnaryFunction f) {
	std::for_each (begin (r), end (r), f);
}

template <typename R, typename T, typename = enable_if_t<is_range<const R>::value>>
iterator_difference_t<range_iterator_t<const R>> count (const R & r, const T & value) {
	return std::count (begin (r), end (r), value);
}
template <typename R, typename UnaryPredicate, typename = enable_if_t<is_range<const R>::value>>
iterator_difference_t<range_iterator_t<const R>> count_if (const R & r, UnaryPredicate p) {
	return std::count_if (begin (r), end (r), p);
}

template <typename R, typename InputIt, typename = enable_if_t<is_range<const R>::value>>
std::pair<range_iterator_t<const R>, InputIt> mismatch (const R & r, InputIt it) {
	return std::mismatch (begin (r), end (r), it);
}
template <typename R, typename InputIt, typename BinaryPredicate,
          typename = enable_if_t<is_range<const R>::value>>
std::pair<range_iterator_t<const R>, InputIt> mismatch (const R & r, InputIt it,
                                                        BinaryPredicate p) {
	return std::mismatch (begin (r), end (r), it, p);
}
#if __cpluplus >= 201402L
template <typename R1, typename R2,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
std::pair<range_iterator_t<const R1>, range_iterator_t<const R2>> mismatch (const R1 & r,
                                                                            const R2 & r2) {
	return std::mismatch (begin (r), end (r), begin (r2), end (r2));
}
template <typename R1, typename R2, typename BinaryPredicate,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
std::pair<range_iterator_t<const R1>, range_iterator_t<const R2>>
mismatch (const R1 & r, const R2 & r2, BinaryPredicate p) {
	return std::mismatch (begin (r), end (r), begin (r2), end (r2), p);
}
#endif

template <typename R, typename InputIt, typename = enable_if_t<is_range<const R>::value>>
bool equal (const R & r, InputIt it) {
	return std::equal (begin (r), end (r), it);
}
template <typename R, typename InputIt, typename BinaryPredicate,
          typename = enable_if_t<is_range<const R>::value>>
bool equal (const R & r, InputIt it, BinaryPredicate p) {
	return std::equal (begin (r), end (r), it, p);
}
#if __cpluplus >= 201402L
template <typename R1, typename R2,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
bool equal (const R1 & r, const R2 & r2) {
	return std::equal (begin (r), end (r), begin (r2), end (r2));
}
template <typename R1, typename R2, typename BinaryPredicate,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
bool equal (const R1 & r, const R2 & r2, BinaryPredicate p) {
	return std::equal (begin (r), end (r), begin (r2), end (r2), p);
}
#endif

template <typename R, typename T, typename = enable_if_t<is_range<const R>::value>>
range_iterator_t<const R> find (const R & r, const T & value) {
	return std::find (begin (r), end (r), value);
}
template <typename R, typename UnaryPredicate, typename = enable_if_t<is_range<const R>::value>>
range_iterator_t<const R> find_if (const R & r, UnaryPredicate p) {
	return std::find_if (begin (r), end (r), p);
}
template <typename R, typename UnaryPredicate, typename = enable_if_t<is_range<const R>::value>>
range_iterator_t<const R> find_if_not (const R & r, UnaryPredicate p) {
	return std::find_if_not (begin (r), end (r), p);
}

template <typename R1, typename R2,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
range_iterator_t<const R1> find_end (const R1 & r, const R2 & r2) {
	return std::find_end (begin (r), end (r), begin (r2), end (r2));
}
template <typename R1, typename R2, typename BinaryPredicate,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
range_iterator_t<const R1> find_end (const R1 & r, const R2 & r2, BinaryPredicate p) {
	return std::find_end (begin (r), end (r), begin (r2), end (r2), p);
}

template <typename R1, typename R2,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
range_iterator_t<const R1> find_first_of (const R1 & r, const R2 & r2) {
	return std::find_first_of (begin (r), end (r), begin (r2), end (r2));
}
template <typename R1, typename R2, typename BinaryPredicate,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
range_iterator_t<const R1> find_first_of (const R1 & r, const R2 & r2, BinaryPredicate p) {
	return std::find_first_of (begin (r), end (r), begin (r2), end (r2), p);
}

template <typename R, typename = enable_if_t<is_range<const R>::value>>
range_iterator_t<const R> adjacent_find (const R & r) {
	return std::adjacent_find (begin (r), end (r));
}
template <typename R, typename BinaryPredicate, typename = enable_if_t<is_range<const R>::value>>
range_iterator_t<const R> adjacent_find (const R & r, BinaryPredicate p) {
	return std::adjacent_find (begin (r), end (r), p);
}

template <typename R1, typename R2,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
range_iterator_t<const R1> search (const R1 & r, const R2 & r2) {
	return std::search (begin (r), end (r), begin (r2), end (r2));
}
template <typename R1, typename R2, typename BinaryPredicate,
          typename = enable_if_t<is_range<const R1>::value && is_range<const R2>::value>>
range_iterator_t<const R1> search (const R1 & r, const R2 & r2, BinaryPredicate p) {
	return std::search (begin (r), end (r), begin (r2), end (r2), p);
}

template <typename R, typename Size, typename T, typename = enable_if_t<is_range<const R>::value>>
range_iterator_t<const R> search_n (const R & r, Size count, const T & value) {
	return std::search_n (begin (r), end (r), count, value);
}
template <typename R, typename Size, typename T, typename BinaryPredicate,
          typename = enable_if_t<is_range<const R>::value>>
range_iterator_t<const R> search_n (const R & r, Size count, const T & value, BinaryPredicate p) {
	return std::search_n (begin (r), end (r), count, value, p);
}

// TODO rest of algorithm
} // namespace duck
