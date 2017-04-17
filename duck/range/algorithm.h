#pragma once

// Overloads of <algorithm> functions to accept range arguments instead of iterator pairs.

#include <algorithm>
#include <duck/range/base.h>

#define HAS_CPP14 (__cpluplus >= 201402L)

namespace duck {

// non modifying sequence operations

template <typename InputIt, typename UnaryPredicate>
bool all_of (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::all_of (r.begin (), r.end (), p);
}
template <typename InputIt, typename UnaryPredicate>
bool none_of (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::none_of (r.begin (), r.end (), p);
}
template <typename InputIt, typename UnaryPredicate>
bool any_of (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::any_of (r.begin (), r.end (), p);
}

template <typename InputIt, typename UnaryFunction>
void for_each (Range::Base<InputIt> r, UnaryFunction f) {
	std::for_each (r.begin (), r.end (), f);
}

template <typename InputIt, typename T>
typename std::iterator_traits<InputIt>::difference_type count (Range::Base<InputIt> r,
                                                               const T & value) {
	return std::count (r.begin (), r.end (), value);
}
template <typename InputIt, typename UnaryPredicate>
typename std::iterator_traits<InputIt>::difference_type count_if (Range::Base<InputIt> r,
                                                                  UnaryPredicate p) {
	return std::count_if (r.begin (), r.end (), p);
}

template <typename InputIt1, typename InputIt2>
std::pair<InputIt1, InputIt2> mismatch (Range::Base<InputIt1> r, InputIt2 it) {
	return std::mismatch (r.begin (), r.end (), it);
}
template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
std::pair<InputIt1, InputIt2> mismatch (Range::Base<InputIt1> r, InputIt2 it, BinaryPredicate p) {
	return std::mismatch (r.begin (), r.end (), it, p);
}
#if HAS_CPP14
template <typename InputIt1, typename InputIt2>
std::pair<InputIt1, InputIt2> mismatch (Range::Base<InputIt1> r, Range::Base<InputIt2> r2) {
	return std::mismatch (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
std::pair<InputIt1, InputIt2> mismatch (Range::Base<InputIt1> r, Range::Base<InputIt2> r2,
                                        BinaryPredicate p) {
	return std::mismatch (r.begin (), r.end (), r2.begin (), r2.end (), p);
}
#endif

template <typename InputIt1, typename InputIt2> bool equal (Range::Base<InputIt1> r, InputIt2 it) {
	return std::equal (r.begin (), r.end (), it);
}
template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
bool equal (Range::Base<InputIt1> r, InputIt2 it, BinaryPredicate p) {
	return std::equal (r.begin (), r.end (), it, p);
}
#if HAS_CPP14
template <typename InputIt1, typename InputIt2>
bool equal (Range::Base<InputIt1> r, Range::Base<InputIt2> r2) {
	return std::equal (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
bool equal (Range::Base<InputIt1> r, Range::Base<InputIt2> r2, BinaryPredicate p) {
	return std::equal (r.begin (), r.end (), r2.begin (), r2.end (), p);
}
#endif

template <typename InputIt, typename T> InputIt find (Range::Base<InputIt> r, const T & value) {
	return std::find (r.begin (), r.end (), value);
}
template <typename InputIt, typename UnaryPredicate>
InputIt find_if (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::find_if (r.begin (), r.end (), p);
}
template <typename InputIt, typename UnaryPredicate>
InputIt find_if_not (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::find_if_not (r.begin (), r.end (), p);
}

template <typename ForwardIt1, typename ForwardIt2>
ForwardIt1 find_end (Range::Base<ForwardIt1> r, Range::Base<ForwardIt2> r2) {
	return std::find_end (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename ForwardIt1, typename ForwardIt2, typename BinaryPredicate>
ForwardIt1 find_end (Range::Base<ForwardIt1> r, Range::Base<ForwardIt2> r2, BinaryPredicate p) {
	return std::find_end (r.begin (), r.end (), r2.begin (), r2.end (), p);
}

template <typename InputIt1, typename ForwardIt2>
InputIt1 find_first_of (Range::Base<InputIt1> r, Range::Base<ForwardIt2> r2) {
	return std::find_first_of (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename InputIt1, typename ForwardIt2, typename BinaryPredicate>
InputIt1 find_first_of (Range::Base<InputIt1> r, Range::Base<ForwardIt2> r2, BinaryPredicate p) {
	return std::find_first_of (r.begin (), r.end (), r2.begin (), r2.end (), p);
}

template <typename ForwardIt> ForwardIt adjacent_find (Range::Base<ForwardIt> r) {
	return std::adjacent_find (r.begin (), r.end ());
}
template <typename ForwardIt, typename BinaryPredicate>
ForwardIt adjacent_find (Range::Base<ForwardIt> r, BinaryPredicate p) {
	return std::adjacent_find (r.begin (), r.end (), p);
}

template <typename ForwardIt1, typename ForwardIt2>
ForwardIt1 search (Range::Base<ForwardIt1> r, Range::Base<ForwardIt2> r2) {
	return std::search (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename ForwardIt1, typename ForwardIt2, typename BinaryPredicate>
ForwardIt1 search (Range::Base<ForwardIt1> r, Range::Base<ForwardIt2> r2, BinaryPredicate p) {
	return std::search (r.begin (), r.end (), r2.begin (), r2.end (), p);
}

template <typename ForwardIt, typename Size, typename T>
ForwardIt search_n (Range::Base<ForwardIt> r, Size count, const T & value) {
	return std::search_n (r.begin (), r.end (), count, value);
}
template <typename ForwardIt, typename Size, typename T, typename BinaryPredicate>
ForwardIt search_n (Range::Base<ForwardIt> r, Size count, const T & value, BinaryPredicate p) {
	return std::search_n (r.begin (), r.end (), count, value, p);
}

// TODO rest of algorithm
}
