#pragma once

// Range V3 combinators
// STATUS: WIP

#include <algorithm>
#include <duck/range/range.h>
#include <limits>

namespace duck {
/********************************************************************************
 * Type traits.
 */
template <typename It, typename Category>
using is_iterator_of_category = std::is_base_of<Category, iterator_category_t<It>>;

// Is callable
template <typename F, typename Arg, typename = void> struct is_unary_invocable : std::false_type {};
template <typename F, typename Arg>
struct is_unary_invocable<F, Arg, void_t<decltype (std::declval<F> () (std::declval<Arg> ()))>>
    : std::true_type {};

// Is predicate
template <typename Predicate, typename Arg, typename = void>
struct is_unary_predicate : std::false_type {};
template <typename Predicate, typename Arg>
struct is_unary_predicate<
    Predicate, Arg,
    void_t<decltype (static_cast<bool> (std::declval<Predicate> () (std::declval<Arg> ())))>>
    : std::true_type {};

/********************************************************************************
 * Slicing. TODO
 * Just pack lambdas that transform iterators ?
 */

/********************************************************************************
 * Reverse order.
 */
template <typename R> class reverse_range {
	static_assert (is_range<R>::value, "reverse_range<R>: R must be a range");
	static_assert (
	    is_iterator_of_category<range_iterator_t<R>, std::bidirectional_iterator_tag>::value,
	    "reverse_range<R>: R must be at least bidirectional_iterator");

public:
	using iterator = std::reverse_iterator<range_iterator_t<R>>;

	reverse_range (R && r) : inner_ (std::forward<R> (r)) {}

	iterator begin () const { return iterator{duck::adl_end (inner_)}; }
	iterator end () const { return iterator{duck::adl_begin (inner_)}; }
	iterator_difference_t<iterator> size () const { return duck::size (inner_); }

private:
	R inner_;
};

template <typename R> reverse_range<R> reverse (R && r) {
	return {std::forward<R> (r)};
}

struct reverse_range_tag {};
inline reverse_range_tag reverse () {
	return {};
}
template <typename R>
auto operator| (R && r, reverse_range_tag) -> decltype (reverse (std::forward<R> (r))) {
	return reverse (std::forward<R> (r));
}

/********************************************************************************
 * Indexed range.
 * Returned value_type has an index field, and a value() member.
 * end() iterator index is UB.
 */
template <typename It, typename Int> class indexed_iterator {
	static_assert (is_iterator<It>::value, "indexed_iterator<It, Int>: It must be an iterator type");
	static_assert (std::is_integral<Int>::value, "indexed_iterator<It, Int>: Int must be integral");

public:
	using iterator_category = iterator_category_t<It>;
	struct value_type {
		It it;
		Int index;
		value_type () = default;
		value_type (It it_arg, Int index_arg) : it (it_arg), index (index_arg) {}
		iterator_reference_t<It> value () const { return *it; }
	};
	using difference_type = iterator_difference_t<It>;
	using pointer = const value_type *;
	using reference = const value_type &;

	indexed_iterator () = default;
	indexed_iterator (It it, Int index) : d_ (it, index) {}

	It base () const { return d_.it; }

	// Input / output
	indexed_iterator & operator++ () { return ++d_.it, ++d_.index, *this; }
	reference operator* () const { return d_; }
	pointer operator-> () const { return &d_; }
	bool operator== (const indexed_iterator & o) const { return d_.it == o.d_.it; }
	bool operator!= (const indexed_iterator & o) const { return d_.it != o.d_.it; }

	// Forward
	indexed_iterator operator++ (int) {
		indexed_iterator tmp (*this);
		++*this;
		return tmp;
	}

	// Bidir
	indexed_iterator & operator-- () { return --d_.it, --d_.index, *this; }
	indexed_iterator operator-- (int) {
		indexed_iterator tmp (*this);
		--*this;
		return tmp;
	}

	// Random access
	indexed_iterator & operator+= (difference_type n) { return d_.it += n, d_.index += n, *this; }
	indexed_iterator operator+ (difference_type n) const {
		return indexed_iterator (d_.it + n, d_.index + n);
	}
	friend indexed_iterator operator+ (difference_type n, const indexed_iterator & it) {
		return it + n;
	}
	indexed_iterator & operator-= (difference_type n) { return d_.it -= n, d_.index -= n, *this; }
	indexed_iterator operator- (difference_type n) const {
		return indexed_iterator (d_.it - n, d_.index - n);
	}
	difference_type operator- (const indexed_iterator & o) const { return d_.it - o.d_.it; }
	value_type operator[] (difference_type n) const { return *(*this + n); }
	bool operator< (const indexed_iterator & o) const { return d_.it < o.d_.it; }
	bool operator> (const indexed_iterator & o) const { return d_.it > o.d_.it; }
	bool operator<= (const indexed_iterator & o) const { return d_.it <= o.d_.it; }
	bool operator>= (const indexed_iterator & o) const { return d_.it >= o.d_.it; }

private:
	value_type d_{};
};

template <typename R, typename Int> class indexed_range {
	static_assert (is_range<R>::value, "indexed_range<R, Int>: R must be a range");
	static_assert (std::is_integral<Int>::value,
	               "indexed_range<R, Int>: Int must be an integral type");

public:
	using iterator = indexed_iterator<range_iterator_t<R>, Int>;

	indexed_range (R && r) : inner_ (std::forward<R> (r)) {}

	iterator begin () const { return {duck::adl_begin (inner_), 0}; }
	iterator end () const { return {duck::adl_end (inner_), std::numeric_limits<Int>::max ()}; }
	iterator_difference_t<iterator> size () const { return duck::size (inner_); }

private:
	R inner_;
};

template <typename Int, typename R> indexed_range<R, Int> index (R && r) {
	return {std::forward<R> (r)};
}

template <typename Int> struct indexed_range_tag {};
template <typename Int = int> indexed_range_tag<Int> indexed () {
	return {};
}
template <typename Int, typename R>
auto operator| (R && r, indexed_range_tag<Int>) -> decltype (index<Int> (std::forward<R> (r))) {
	return index<Int> (std::forward<R> (r));
}

/********************************************************************************
 * Filter with a predicate.
 */
template <typename R, typename Predicate> class filtered_range {
	static_assert (is_range<R>::value, "filtered_range<R, Predicate>: R must be a range");
	static_assert (is_unary_predicate<Predicate, iterator_reference_t<range_iterator_t<R>>>::value,
	               "filtered_range<R, Predicate>: Predicate must be callable on R values");

public:
	using inner_iterator = range_iterator_t<R>;
	class iterator {
	public:
		// At most this is a bidirectional_iterator
		using iterator_category =
		    common_type_t<std::bidirectional_iterator_tag, iterator_category_t<inner_iterator>>;
		using value_type = iterator_value_type_t<inner_iterator>;
		using difference_type = iterator_difference_t<inner_iterator>;
		using pointer = iterator_pointer_t<inner_iterator>;
		using reference = iterator_reference_t<inner_iterator>;

		iterator () = default;
		iterator (inner_iterator it, const filtered_range & range) : it_ (it), range_ (&range) {}

		inner_iterator base () const { return it_; }

		// Input / output
		iterator & operator++ () { return it_ = range_->next_after (it_), *this; }
		reference operator* () const { return *it_; }
		pointer operator-> () const { return it_.operator-> (); }
		bool operator== (const iterator & o) const { return it_ == o.it_; }
		bool operator!= (const iterator & o) const { return it_ != o.it_; }

		// Forward
		iterator operator++ (int) {
			iterator tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir
		iterator & operator-- () { return it_ = range_->previous_before (it_), *this; }
		iterator operator-- (int) {
			iterator tmp (*this);
			--*this;
			return tmp;
		}

	private:
		inner_iterator it_{};
		const filtered_range * range_{nullptr};
	};

	filtered_range (R && r, const Predicate & predicate)
	    : inner_ (std::forward<R> (r)), predicate_ (predicate) {}
	filtered_range (R && r, Predicate && predicate)
	    : inner_ (std::forward<R> (r)), predicate_ (std::move (predicate)) {}

	iterator begin () const { return {next (duck::adl_begin (inner_)), *this}; }
	iterator end () const { return {duck::adl_end (inner_), *this}; }

private:
	inner_iterator next (inner_iterator from) const {
		return std::find_if (from, duck::adl_end (inner_), predicate_);
	}
	inner_iterator next_after (inner_iterator from) const {
		if (from == duck::adl_end (inner_))
			return from;
		else
			return next (++from);
	}
	inner_iterator previous_before (inner_iterator from) const {
		auto b = duck::adl_begin (inner_);
		if (from == b) {
			return from;
		} else {
			// Try to find a match before from
			// If this fails, from was first, return it
			using RevIt = std::reverse_iterator<inner_iterator>;
			auto rev_end = RevIt{b};
			auto rev_next = std::find_if (RevIt{--from}, rev_end, predicate_);
			if (rev_next != rev_end)
				return rev_next.base ();
			else
				return from;
		}
	}

	R inner_;
	Predicate predicate_;
};

template <typename R, typename Predicate>
filtered_range<R, remove_cvref_t<Predicate>> filter (R && r, Predicate && predicate) {
	return {std::forward<R> (r), std::forward<Predicate> (predicate)};
}

template <typename Predicate> struct filtered_range_tag {
	filtered_range_tag (const Predicate & p) : predicate (p) {}
	filtered_range_tag (Predicate && p) : predicate (std::move (p)) {}
	Predicate predicate;
};
template <typename Predicate>
filtered_range_tag<remove_cvref_t<Predicate>> filter (Predicate && predicate) {
	return {std::forward<Predicate> (predicate)};
}
template <typename R, typename Predicate>
auto operator| (R && r, filtered_range_tag<Predicate> tag)
    -> decltype (filter (std::forward<R> (r), std::move (tag.predicate))) {
	return filter (std::forward<R> (r), std::move (tag.predicate));
}

/********************************************************************************
 * Processed range.
 * Apply function f to each element.
 */
template <typename R, typename Function> class mapped_range {
	static_assert (is_range<R>::value, "mapped_range<R, Function>: R must be a range");
	static_assert (is_unary_invocable<Function, iterator_reference_t<range_iterator_t<R>>>::value,
	               "mapped_range<R, Function>: Function must be callable on R values");

public:
	class iterator {
	public:
		using inner_iterator = range_iterator_t<R>;
		using iterator_category = iterator_category_t<inner_iterator>;
		using value_type = invoke_result_t<Function, iterator_reference_t<inner_iterator>>;
		using difference_type = iterator_difference_t<inner_iterator>;
		using pointer = void;
		using reference = value_type; // No way to take references on function_ result

		iterator () = default;
		iterator (inner_iterator it, const mapped_range & range) : it_ (it), range_ (&range) {}

		inner_iterator base () const { return it_; }

		// Input / output
		iterator & operator++ () { return ++it_, *this; }
		reference operator* () const { return range_->function_ (*it_); }
		// No operator->
		bool operator== (const iterator & o) const { return it_ == o.it_; }
		bool operator!= (const iterator & o) const { return it_ != o.it_; }

		// Forward
		iterator operator++ (int) {
			iterator tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir
		iterator & operator-- () { return --it_, *this; }
		iterator operator-- (int) {
			iterator tmp (*this);
			--*this;
			return tmp;
		}

		// Random access
		iterator & operator+= (difference_type n) { return it_ += n, *this; }
		iterator operator+ (difference_type n) const { return iterator (it_ + n, range_); }
		friend iterator operator+ (difference_type n, const iterator & it) { return it + n; }
		iterator & operator-= (difference_type n) { return it_ -= n, *this; }
		iterator operator- (difference_type n) const { return iterator (it_ - n, range_); }
		difference_type operator- (const iterator & o) const { return it_ - o.it_; }
		reference operator[] (difference_type n) const { return *(*this + n); }
		bool operator< (const iterator & o) const { return it_ < o.it_; }
		bool operator> (const iterator & o) const { return it_ > o.it_; }
		bool operator<= (const iterator & o) const { return it_ <= o.it_; }
		bool operator>= (const iterator & o) const { return it_ >= o.it_; }

	private:
		inner_iterator it_{};
		const mapped_range * range_{nullptr};
	};

	mapped_range (R && r, const Function & function)
	    : inner_ (std::forward<R> (r)), function_ (function) {}
	mapped_range (R && r, Function && function)
	    : inner_ (std::forward<R> (r)), function_ (std::move (function)) {}

	iterator begin () const { return {duck::adl_begin (inner_), *this}; }
	iterator end () const { return {duck::adl_end (inner_), *this}; }
	iterator_difference_t<iterator> size () const { return duck::size (inner_); }

private:
	R inner_;
	Function function_;
};

template <typename R, typename Function>
mapped_range<R, remove_cvref_t<Function>> map (R && r, Function && function) {
	return {std::forward<R> (r), std::forward<Function> (function)};
}

template <typename Function> struct mapped_range_tag {
	mapped_range_tag (const Function & f) : function (f) {}
	mapped_range_tag (Function && f) : function (std::move (f)) {}
	Function function;
};
template <typename Function> mapped_range_tag<remove_cvref_t<Function>> map (Function && function) {
	return {std::forward<Function> (function)};
}
template <typename R, typename Function>
auto operator| (R && r, mapped_range_tag<Function> tag)
    -> decltype (map (std::forward<R> (r), std::move (tag.function))) {
	return map (std::forward<R> (r), std::move (tag.function));
}
} // namespace duck
