#pragma once

// Range V3
// STATUS: WIP, new_syntax_convention

#include <duck/type_traits.h>
#include <iterator>
#include <utility>

namespace duck {
/* Range
 * TODO overall doc
 * TODO adapt combinators
 *
 * rvalue note:
 * Range based on lvalues (references to containers) are mostly safe.
 * Range based on temporaries (rvalues) store the temporary objects internally.
 * Thus, to guarantee the lifetime of the objects, the range must be stored.
 * Some functions are still valid, as long as the expression builds a new object as result:
 * > auto r = vec<int>{...}; f(begin(r)); // correct
 * > auto it = begin (vec<int>{...}); // 'it' is dangling
 * > auto & v = duck::front (vec<int>{...}); // 'int& v' is dangling
 * > auto v = duck::front (vec<int>{...}); // correct
 */

/* Import std::begin and std::end in namespace duck.
 *
 * std::begin and std::end are supposed to be used as follows:
 * > using std::begin;
 * > it = begin (obj);
 * This unqualified calls will fetch both ADL and std:: overloads of begin().
 *
 * By importing std::begin and std::end, namespace duck is now a context where unqualified calls
 * catch the same overloads as this usage pattern.
 *
 * Calls external to namespace duck are supposed to call begin with the same pattern.
 * "using std::begin" is equivalent to "using duck::begin".
 *
 * std::begin/end also catch rvlues by casting them to const&.
 */
using std::begin;
using std::end;

/*********************************************************************************
 * Type traits.
 */

// Iterator type deduced from begin(T / T&)
template <typename T> using range_iterator_t = decltype (begin (std::declval<T> ()));

// A Range is anything iterable, with begin and end
template <typename T, typename = void> struct is_range : std::false_type {};
template <typename T>
struct is_range<T,
                void_t<decltype (begin (std::declval<T> ())), decltype (end (std::declval<T> ()))>>
    : std::true_type {};

// Typedefs
template <typename It>
using iterator_category_t = typename std::iterator_traits<It>::iterator_category;
template <typename It> using iterator_value_type_t = typename std::iterator_traits<It>::value_type;
template <typename It> using iterator_reference_t = typename std::iterator_traits<It>::reference;
template <typename It> using iterator_pointer_t = typename std::iterator_traits<It>::pointer;
template <typename It>
using iterator_difference_t = typename std::iterator_traits<It>::difference_type;

// Is iterator
template <typename T, typename = void> struct is_iterator : std::false_type {};
template <typename T> struct is_iterator<T, void_t<iterator_category_t<T>>> : std::true_type {};

// Has empty() method
template <typename T, typename = void> struct has_empty_method : std::false_type {};
template <typename T>
struct has_empty_method<T, void_t<decltype (std::declval<T> ().empty ())>> : std::true_type {};

// Has size() method and size_type typedef
template <typename T, typename = void> struct has_size_method : std::false_type {};
template <typename T>
struct has_size_method<T, void_t<decltype (std::declval<T> ().size ())>> : std::true_type {};

/*********************************************************************************
 * ADL versions of begin / end.
 * Alternative to the "using std::begin; begin (t)" pattern, in one line.
 * Also support rvalue ranges with the same conditions as begin/end.
 */
template <typename T> range_iterator_t<T> adl_begin (T && t) {
	return begin (std::forward<T> (t));
}
template <typename T> range_iterator_t<T> adl_end (T && t) {
	return end (std::forward<T> (t));
}

/*********************************************************************************
 * Free functions operating on iterable objects.
 * With optimised cases for containers.
 */

// empty
namespace internal_range {
	template <typename T> bool empty_impl (const T & t, std::true_type) { return t.empty (); }
	template <typename T> bool empty_impl (const T & t, std::false_type) {
		return begin (t) == end (t);
	}
} // namespace internal_range
template <typename T> bool empty (const T & t) {
	return internal_range::empty_impl (t, has_empty_method<const T &>{});
}

// size
namespace internal_range {
	template <typename T>
	iterator_difference_t<range_iterator_t<const T &>> size_impl (const T & t, std::true_type) {
		return static_cast<iterator_difference_t<range_iterator_t<const T &>>> (t.size ());
	}
	template <typename T>
	iterator_difference_t<range_iterator_t<const T &>> size_impl (const T & t, std::false_type) {
		return std::distance (begin (t), end (t));
	}
} // namespace internal_range
template <typename T> iterator_difference_t<range_iterator_t<const T &>> size (const T & t) {
	return internal_range::size_impl (t, has_size_method<T>{});
}

// front / back
template <typename T> iterator_reference_t<range_iterator_t<T>> front (T && t) {
	return *begin (std::forward<T> (t));
}
template <typename T> iterator_reference_t<range_iterator_t<T>> back (T && t) {
	return *std::prev (end (std::forward<T> (t)));
}
template <typename T>
iterator_reference_t<range_iterator_t<T>> nth (T && t,
                                               iterator_difference_t<range_iterator_t<T>> n) {
	return *std::next (begin (std::forward<T> (t)), n);
}

// to_container
template <typename Container, typename T> Container to_container (const T & t) {
	return Container{begin (t), end (t)};
}

// contains
namespace internal_range {
	template <typename It>
	bool contains_impl (It begin_it, It end_it, It it, std::random_access_iterator_tag) {
		return begin_it <= it && it < end_it;
	}
	template <typename It>
	bool contains_impl (It begin_it, It end_it, It it, std::input_iterator_tag) {
		for (; begin_it != end_it; ++begin_it)
			if (begin_it == it)
				return true;
		return false;
	}
} // namespace internal_range
template <typename T> bool contains (const T & t, range_iterator_t<const T &> it) {
	return internal_range::contains_impl (begin (t), end (t), it,
	                                      iterator_category_t<range_iterator_t<const T &>>{});
}

/*******************************************************************************
 * iterator_pair:
 * - eager range implementation, represents a pair of iterators
 * - does not guarantee that the iterated ressource stays alive (no lifetime extension !)
 */
template <typename It> class iterator_pair {
	static_assert (is_iterator<It>::value, "iterator_pair<It>: It must be an iterator type");

public:
	iterator_pair (It begin_it, It end_it) : begin_ (begin_it), end_ (end_it) {}
	It begin () const { return begin_; }
	It end () const { return end_; }

private:
	It begin_;
	It end_;
};

/**********************************************************************************
 * Integer iterator for [i, j[ ranges
 */
template <typename Int> class integer_iterator {
	static_assert (std::is_integral<Int>::value,
	               "integer_iterator<Int>: Int must be an integer type");

public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = Int;
	using difference_type = std::ptrdiff_t;
	using pointer = const value_type *;
	using reference = value_type; // Force copying the int

	integer_iterator () noexcept = default;
	integer_iterator (Int n) noexcept : n_ (n) {}

	// Input / output
	integer_iterator & operator++ () noexcept { return ++n_, *this; }
	reference operator* () const noexcept { return n_; }
	pointer operator-> () const noexcept { return &n_; }
	bool operator== (const integer_iterator & o) const noexcept { return n_ == o.n_; }
	bool operator!= (const integer_iterator & o) const noexcept { return n_ != o.n_; }

	// Forward
	integer_iterator operator++ (int) noexcept {
		integer_iterator tmp (*this);
		++*this;
		return tmp;
	}

	// Bidir
	integer_iterator & operator-- () noexcept { return --n_, *this; }
	integer_iterator operator-- (int) noexcept {
		integer_iterator tmp (*this);
		--*this;
		return tmp;
	}

	// Random access
	integer_iterator & operator+= (difference_type n) noexcept { return n_ += n, *this; }
	integer_iterator operator+ (difference_type n) const noexcept {
		return integer_iterator (n_ + n);
	}
	friend integer_iterator operator+ (difference_type n, const integer_iterator & it) noexcept {
		return it + n;
	}
	integer_iterator & operator-= (difference_type n) noexcept { return n_ -= n, *this; }
	integer_iterator operator- (difference_type n) const noexcept {
		return integer_iterator (n_ - n);
	}
	difference_type operator- (const integer_iterator & o) const noexcept { return n_ - o.n_; }
	reference operator[] (difference_type n) const noexcept { return n_ + n; }
	bool operator< (const integer_iterator & o) const noexcept { return n_ < o.n_; }
	bool operator> (const integer_iterator & o) const noexcept { return n_ > o.n_; }
	bool operator<= (const integer_iterator & o) const noexcept { return n_ <= o.n_; }
	bool operator>= (const integer_iterator & o) const noexcept { return n_ >= o.n_; }

private:
	Int n_{};
};

/*******************************************************************************
 * range() function overloads
 */

template <typename T, typename = enable_if_t<is_range<T>::value>>
auto range (T && t) -> decltype (std::forward<T> (t)) {
	return std::forward<T> (t);
}

template <typename It, typename = enable_if_t<is_iterator<It>::value>>
iterator_pair<It> range (It begin_it, It end_it) {
	return {begin_it, end_it};
}

template <typename Int, typename = enable_if_t<std::is_integral<Int>::value>>
iterator_pair<integer_iterator<Int>> range (Int from, Int to) {
	return {integer_iterator<Int>{from}, integer_iterator<Int>{to}};
}
template <typename Int, typename = enable_if_t<std::is_integral<Int>::value>>
iterator_pair<integer_iterator<Int>> range (Int to) {
	return range (Int{}, to);
}

// Array situation:
// T[N] -> matched by std::begin()
// (T*, T*) -> matched by range (It, It)
// (T*, N) -> below
template <typename T, typename Int, typename = enable_if_t<std::is_integral<Int>::value>>
iterator_pair<T *> range (T * base, Int size) {
	return {base, base + size};
}

// ilist lifetime is never extended beyond the return, so we must build a temporary vector...
template <typename T> std::vector<T> range (std::initializer_list<T> ilist) {
	return std::vector<T>{ilist};
}

/*******************************************************************************
 * Range object.
 * Wraps any range into a user friendly object where most range functions available as methods.
 * FIXME try to have the same semantics with temporaries as free functions ?
 * TODO range_object taking variadic params, calls range() for ease of use
 */
template <typename R> class range_object_wrapper {
private:
	R wrapped_;

public:
	using const_iterator = range_iterator_t<const R &>;

	range_object_wrapper (R && r) : wrapped_ (std::forward<R> (r)) {}

	// Forced to use duck:: prefix: unqualified lookup stops at class level
	const_iterator begin () const { return duck::adl_begin (wrapped_); }
	const_iterator end () const { return duck::adl_end (wrapped_); }
	bool empty () const { return duck::empty (wrapped_); }
	iterator_difference_t<const_iterator> size () const { return duck::size (wrapped_); }
	iterator_reference_t<const_iterator> front () const { return duck::front (wrapped_); }
	iterator_reference_t<const_iterator> back () const { return duck::back (wrapped_); }
	iterator_reference_t<const_iterator> operator[] (iterator_difference_t<const_iterator> n) const {
		return duck::nth (wrapped_, n);
	}

	template <typename Container> Container to_container () const {
		return duck::to_container<Container> (wrapped_);
	}
	bool contains (const_iterator it) const { return duck::contains (wrapped_, it); }
};
template <typename R> range_object_wrapper<R> range_object (R && r) {
	return {std::forward<R> (r)};
}

// TODO operator== may not benefit from ADL, or may clash...
} // namespace duck
