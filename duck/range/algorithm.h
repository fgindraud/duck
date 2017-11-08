#pragma once

// Overloads of <algorithm> functions to accept range arguments instead of iterator pairs.

#include <algorithm>
#include <duck/range/range.h>

#ifndef HAS_CPP14
#define HAS_CPP14 (__cpluplus >= 201402L)
#endif

namespace duck {

// non modifying sequence operations

template <typename Derived, typename UnaryPredicate>
bool all_of (const Range::Base<Derived> & r, UnaryPredicate p) {
	return std::all_of (r.derived ().begin (), r.derived ().end (), p);
}
template <typename Derived, typename UnaryPredicate>
bool none_of (const Range::Base<Derived> & r, UnaryPredicate p) {
	return std::none_of (r.derived ().begin (), r.derived ().end (), p);
}
template <typename Derived, typename UnaryPredicate>
bool any_of (const Range::Base<Derived> & r, UnaryPredicate p) {
	return std::any_of (r.derived ().begin (), r.derived ().end (), p);
}

template <typename Derived, typename UnaryFunction>
void for_each (const Range::Base<Derived> & r, UnaryFunction f) {
	std::for_each (r.derived ().begin (), r.derived ().end (), f);
}

template <typename Derived, typename T>
typename Range::RangeIteratorTraits<Derived>::difference_type count (const Range::Base<Derived> & r,
                                                                     const T & value) {
	return std::count (r.derived ().begin (), r.derived ().end (), value);
}
template <typename Derived, typename UnaryPredicate>
typename Range::RangeIteratorTraits<Derived>::difference_type
count_if (const Range::Base<Derived> & r, UnaryPredicate p) {
	return std::count_if (r.derived ().begin (), r.derived ().end (), p);
}

template <typename Derived, typename InputIt>
std::pair<Range::RangeIterator<Derived>, InputIt> mismatch (const Range::Base<Derived> & r,
                                                            InputIt it) {
	return std::mismatch (r.derived ().begin (), r.derived ().end (), it);
}
template <typename Derived, typename InputIt, typename BinaryPredicate>
std::pair<Range::RangeIterator<Derived>, InputIt> mismatch (const Range::Base<Derived> & r,
                                                            InputIt it, BinaryPredicate p) {
	return std::mismatch (r.derived ().begin (), r.derived ().end (), it, p);
}
#if HAS_CPP14
template <typename Derived1, typename Derived2>
std::pair<Range::RangeIterator<Derived1>, Range::RangeIterator<Derived2>>
mismatch (const Range::Base<Derived1> & r, const Range::Base<Derived2> & r2) {
	return std::mismatch (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                      r2.derived ().end ());
}
template <typename Derived1, typename Derived2, typename BinaryPredicate>
std::pair<Range::RangeIterator<Derived1>, Range::RangeIterator<Derived2>>
mismatch (const Range::Base<Derived1> & r, const Range::Base<Derived2> & r2, BinaryPredicate p) {
	return std::mismatch (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                      r2.derived ().end (), p);
}
#endif

template <typename Derived, typename InputIt>
bool equal (const Range::Base<Derived> & r, InputIt it) {
	return std::equal (r.derived ().begin (), r.derived ().end (), it);
}
template <typename Derived, typename InputIt, typename BinaryPredicate>
bool equal (const Range::Base<Derived> & r, InputIt it, BinaryPredicate p) {
	return std::equal (r.derived ().begin (), r.derived ().end (), it, p);
}
#if HAS_CPP14
template <typename Derived1, typename Derived2>
bool equal (const Range::Base<Derived1> & r, const Range::Base<Derived2> & r2) {
	return std::equal (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                   r2.derived ().end ());
}
template <typename Derived1, typename Derived2, typename BinaryPredicate>
bool equal (const Range::Base<Derived1> & r, const Range::Base<Derived2> & r2, BinaryPredicate p) {
	return std::equal (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                   r2.derived ().end (), p);
}
#endif

template <typename Derived, typename T>
Range::RangeIterator<Derived> find (const Range::Base<Derived> & r, const T & value) {
	return std::find (r.derived ().begin (), r.derived ().end (), value);
}
template <typename Derived, typename UnaryPredicate>
Range::RangeIterator<Derived> find_if (const Range::Base<Derived> & r, UnaryPredicate p) {
	return std::find_if (r.derived ().begin (), r.derived ().end (), p);
}
template <typename Derived, typename UnaryPredicate>
Range::RangeIterator<Derived> find_if_not (const Range::Base<Derived> & r, UnaryPredicate p) {
	return std::find_if_not (r.derived ().begin (), r.derived ().end (), p);
}

template <typename Derived1, typename Derived2>
Range::RangeIterator<Derived1> find_end (const Range::Base<Derived1> & r,
                                         const Range::Base<Derived2> & r2) {
	return std::find_end (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                      r2.derived ().end ());
}
template <typename Derived1, typename Derived2, typename BinaryPredicate>
Range::RangeIterator<Derived1> find_end (const Range::Base<Derived1> & r,
                                         const Range::Base<Derived2> & r2, BinaryPredicate p) {
	return std::find_end (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                      r2.derived ().end (), p);
}

template <typename Derived1, typename Derived2>
Range::RangeIterator<Derived1> find_first_of (const Range::Base<Derived1> & r,
                                              const Range::Base<Derived2> & r2) {
	return std::find_first_of (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                           r2.derived ().end ());
}
template <typename Derived1, typename Derived2, typename BinaryPredicate>
Range::RangeIterator<Derived1> find_first_of (const Range::Base<Derived1> & r,
                                              const Range::Base<Derived2> & r2, BinaryPredicate p) {
	return std::find_first_of (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                           r2.derived ().end (), p);
}

template <typename Derived>
Range::RangeIterator<Derived> adjacent_find (const Range::Base<Derived> & r) {
	return std::adjacent_find (r.derived ().begin (), r.derived ().end ());
}
template <typename Derived, typename BinaryPredicate>
Range::RangeIterator<Derived> adjacent_find (const Range::Base<Derived> & r, BinaryPredicate p) {
	return std::adjacent_find (r.derived ().begin (), r.derived ().end (), p);
}

template <typename Derived1, typename Derived2>
Range::RangeIterator<Derived1> search (const Range::Base<Derived1> & r,
                                       const Range::Base<Derived2> & r2) {
	return std::search (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                    r2.derived ().end ());
}
template <typename Derived1, typename Derived2, typename BinaryPredicate>
Range::RangeIterator<Derived1> search (const Range::Base<Derived1> & r,
                                       const Range::Base<Derived2> & r2, BinaryPredicate p) {
	return std::search (r.derived ().begin (), r.derived ().end (), r2.derived ().begin (),
	                    r2.derived ().end (), p);
}

template <typename Derived, typename Size, typename T>
Range::RangeIterator<Derived> search_n (const Range::Base<Derived> & r, Size count,
                                        const T & value) {
	return std::search_n (r.derived ().begin (), r.derived ().end (), count, value);
}
template <typename Derived, typename Size, typename T, typename BinaryPredicate>
Range::RangeIterator<Derived> search_n (const Range::Base<Derived> & r, Size count, const T & value,
                                        BinaryPredicate p) {
	return std::search_n (r.derived ().begin (), r.derived ().end (), count, value, p);
}

// TODO rest of algorithm
} // namespace duck
