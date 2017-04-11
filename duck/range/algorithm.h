#pragma once

// Overloads of <algorithm> functions to accept range arguments instead of iterator pairs.

#include <algorithm>
#include <duck/range/base.h>

namespace duck {

// non modifying sequence operations

template <typename InputIt, typename UnaryPredicate>
auto all_of (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::all_of (r.begin (), r.end (), p);
}
template <typename InputIt, typename UnaryPredicate>
auto none_of (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::none_of (r.begin (), r.end (), p);
}
template <typename InputIt, typename UnaryPredicate>
auto any_of (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::any_of (r.begin (), r.end (), p);
}

template <typename InputIt, typename UnaryFunction>
void for_each (Range::Base<InputIt> r, UnaryFunction f) {
	std::for_each (r.begin (), r.end (), f);
}

template <typename InputIt, typename T> auto count (Range::Base<InputIt> r, const T & value) {
	return std::count (r.begin (), r.end (), value);
}
template <typename InputIt, typename UnaryPredicate>
auto count_if (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::count_if (r.begin (), r.end (), p);
}

template <typename InputIt1, typename InputIt2>
auto mismatch (Range::Base<InputIt1> r, InputIt2 it) {
	return std::mismatch (r.begin (), r.end (), it);
}
template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
auto mismatch (Range::Base<InputIt1> r, InputIt2 it, BinaryPredicate p) {
	return std::mismatch (r.begin (), r.end (), it, p);
}
template <typename InputIt1, typename InputIt2>
auto mismatch (Range::Base<InputIt1> r, Range::Base<InputIt2> r2) {
	return std::mismatch (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
auto mismatch (Range::Base<InputIt1> r, Range::Base<InputIt2> r2, BinaryPredicate p) {
	return std::mismatch (r.begin (), r.end (), r2.begin (), r2.end (), p);
}

template <typename InputIt1, typename InputIt2> auto equal (Range::Base<InputIt1> r, InputIt2 it) {
	return std::equal (r.begin (), r.end (), it);
}
template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
auto equal (Range::Base<InputIt1> r, InputIt2 it, BinaryPredicate p) {
	return std::equal (r.begin (), r.end (), it, p);
}
template <typename InputIt1, typename InputIt2>
auto equal (Range::Base<InputIt1> r, Range::Base<InputIt2> r2) {
	return std::equal (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename InputIt1, typename InputIt2, typename BinaryPredicate>
auto equal (Range::Base<InputIt1> r, Range::Base<InputIt2> r2, BinaryPredicate p) {
	return std::equal (r.begin (), r.end (), r2.begin (), r2.end (), p);
}

template <typename InputIt, typename T> auto find (Range::Base<InputIt> r, const T & value) {
	return std::find (r.begin (), r.end (), value);
}
template <typename InputIt, typename UnaryPredicate>
auto find_if (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::find_if (r.begin (), r.end (), p);
}
template <typename InputIt, typename UnaryPredicate>
auto find_if_not (Range::Base<InputIt> r, UnaryPredicate p) {
	return std::find_if_not (r.begin (), r.end (), p);
}

template <typename ForwardIt1, typename ForwardIt2>
auto find_end (Range::Base<ForwardIt1> r, Range::Base<ForwardIt2> r2) {
	return std::find_end (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename ForwardIt1, typename ForwardIt2, typename BinaryPredicate>
auto find_end (Range::Base<ForwardIt1> r, Range::Base<ForwardIt2> r2, BinaryPredicate p) {
	return std::find_end (r.begin (), r.end (), r2.begin (), r2.end (), p);
}

template <typename InputIt1, typename ForwardIt2>
auto find_first_of (Range::Base<InputIt1> r, Range::Base<ForwardIt2> r2) {
	return std::find_first_of (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename InputIt1, typename ForwardIt2, typename BinaryPredicate>
auto find_first_of (Range::Base<InputIt1> r, Range::Base<ForwardIt2> r2, BinaryPredicate p) {
	return std::find_first_of (r.begin (), r.end (), r2.begin (), r2.end (), p);
}

template <typename ForwardIt> auto adjacent_find (Range::Base<ForwardIt> r) {
	return std::adjacent_find (r.begin (), r.end ());
}
template <typename ForwardIt, typename BinaryPredicate>
auto adjacent_find (Range::Base<ForwardIt> r, BinaryPredicate p) {
	return std::adjacent_find (r.begin (), r.end (), p);
}

template <typename ForwardIt1, typename ForwardIt2>
auto search (Range::Base<ForwardIt1> r, Range::Base<ForwardIt2> r2) {
	return std::search (r.begin (), r.end (), r2.begin (), r2.end ());
}
template <typename ForwardIt1, typename ForwardIt2, typename BinaryPredicate>
auto search (Range::Base<ForwardIt1> r, Range::Base<ForwardIt2> r2, BinaryPredicate p) {
	return std::search (r.begin (), r.end (), r2.begin (), r2.end (), p);
}

template <typename ForwardIt, typename Size, typename T>
auto search_n (Range::Base<ForwardIt> r, Size count, const T & value) {
	return std::search_n (r.begin (), r.end (), count, value);
}
template <typename ForwardIt, typename Size, typename T, typename BinaryPredicate>
auto search_n (Range::Base<ForwardIt> r, Size count, const T & value, BinaryPredicate p) {
	return std::search_n (r.begin (), r.end (), count, value, p);
}

// TODO rest of algorithm
}
