#pragma once

// In place optional type
// STATUS: mature

#include <cassert>
#include <duck/type_traits.h>
#include <utility>

namespace duck {

// Type tag and global value for "No value"
struct NullOpt {
	constexpr NullOpt () = default;
};
constexpr NullOpt nullopt{};

template <typename T> class Optional {
	/* Optional<T> is similar to the c++17 std::optional<T>.
	 * It conditionally stores in place a T value.
	 *
	 * It behaves like a pointer to data, also for constness:
	 * A const Optional cannot change its state ; however the stored value is mutable.
	 * An Optional<const T> can change its state but not the stored value after creation.
	 *
	 * Moves are considered to change state, so move overloads require a non const && *this.
	 * A moved-from Optional still contains a value (which is moved-from).
	 *
	 * Types that do not support copy/move assignments use destruction+copy/move construction instead.
	 * Non copyable and non movable object should only use emplace().
	 */
	static_assert (!std::is_reference<T>::value, "Optional<T> does not support references");

public:
	using value_type = T;

	constexpr Optional () : has_value_ (false) {}
	constexpr Optional (NullOpt) noexcept : Optional () {}
	Optional (bool) = delete;
	Optional (const Optional & other) : Optional () {
		if (other)
			create (*other);
	}
	Optional (Optional && other) noexcept : Optional () {
		if (other)
			create (*std::move (other));
	}
	Optional (const T & t) : Optional () { create (t); }
	Optional (T && t) : Optional () { create (std::move (t)); }
	template <typename... Args> Optional (InPlace<void>, Args &&... args) : Optional () {
		create (std::forward<Args> (args)...);
	}
	template <typename U, typename... Args>
	Optional (InPlace<void>, std::initializer_list<U> ilist, Args &&... args) : Optional () {
		create (std::move (ilist), std::forward<Args> (args)...);
	}

	~Optional () { reset (); }

	// Assignment
	Optional & operator= (NullOpt) noexcept {
		reset ();
		return *this;
	}
	Optional & operator= (bool) = delete;
	Optional & operator= (const Optional & other) {
		if (has_value () && other)
			replace_value_with (*other);
		else if (!has_value () && other)
			create (*other);
		else if (has_value () && !other)
			destroy ();
		return *this;
	}
	Optional & operator= (Optional && other) noexcept {
		if (has_value () && other)
			replace_value_with (std::move (*other));
		else if (!has_value () && other)
			create (std::move (*other));
		else if (has_value () && !other)
			destroy ();
		return *this;
	}
	Optional & operator= (const T & t) {
		if (has_value ())
			replace_value_with (t);
		else
			create (t);
		return *this;
	}
	Optional & operator= (T && t) noexcept {
		if (has_value ())
			replace_value_with (std::move (t));
		else
			create (std::move (t));
		return *this;
	}

	// Unchecked access
	T & value () const & noexcept {
		assert (has_value ());
		return *value_ptr ();
	}
	T && value () && noexcept {
		assert (has_value ());
		return std::move (*value_ptr ());
	}

	// Operator unchecked access
	T * operator-> () const noexcept { return value_ptr (); }
	T & operator* () const & noexcept { return value (); }
	T && operator* () && noexcept { return std::move (*this).value (); }

	// Status test
	constexpr bool has_value () const noexcept { return has_value_; }
	constexpr explicit operator bool () const noexcept { return has_value (); }

	// Access with default
	template <typename U> constexpr T value_or (U && default_value) const & {
		return has_value () ? value () : static_cast<T> (std::forward<U> (default_value));
	}
	template <typename U> T value_or (U && default_value) && {
		return has_value () ? std::move (*this).value ()
		                    : static_cast<T> (std::forward<U> (default_value));
	}

	// Access with generated default
	template <typename Callable> T value_or_generate (Callable && callable) const & {
		return has_value () ? value () : std::forward<Callable> (callable) ();
	}
	template <typename Callable> T value_or_generate (Callable && callable) && {
		return has_value () ? std::move (*this).value () : std::forward<Callable> (callable) ();
	}

	// Map : Optional<T> -> Optional<U> with f : T -> U
	template <typename Callable,
	          typename ReturnType = typename std::result_of<Callable (const T &)>::type>
	Optional<ReturnType> map (Callable && callable) const & {
		if (has_value ())
			return std::forward<Callable> (callable) (value ());
		else
			return {};
	}
	template <typename Callable, typename ReturnType = typename std::result_of<Callable (T &&)>::type>
	Optional<ReturnType> map (Callable && callable) && {
		if (has_value ())
			return std::forward<Callable> (callable) (std::move (*this).value ());
		else
			return {};
	}

	// Filter : Optional<T> -> Optional<T>, propagate arg if p(arg), or return NullOpt
	template <typename Predicate> Optional filter (Predicate && predicate) const & {
		if (has_value () && std::forward<Predicate> (predicate) (value ()))
			return *this;
		else
			return {};
	}
	template <typename Predicate> Optional filter (Predicate && predicate) && {
		if (has_value () && std::forward<Predicate> (predicate) (value ()))
			return std::move (*this);
		else
			return {};
	}

	// Modifiers
	void reset () noexcept {
		if (has_value ())
			destroy ();
	}
	template <typename... Args> T & emplace (Args &&... args) {
		reset ();
		create (std::forward<Args> (args)...);
		return value ();
	}
	template <typename U, typename... Args>
	T & emplace (std::initializer_list<U> ilist, Args &&... args) {
		reset ();
		create (std::move (ilist), std::forward<Args> (args)...);
		return value ();
	}
	void swap (Optional & other) noexcept {
		using std::swap;
		if (has_value () && other) {
			swap (value (), other.value ());
		} else if (!has_value () && other) {
			create (std::move (*other));
			other.destroy ();
		} else if (has_value () && !other) {
			other.create (std::move (value ()));
			destroy ();
		}
	}

private:
	template <typename... Args> void create (Args &&... args) {
		assert (!has_value_);
		new (&storage_) T (std::forward<Args> (args)...);
		has_value_ = true;
	}
	void destroy () noexcept {
		assert (has_value_);
		value_ptr ()->~T ();
		has_value_ = false;
	}

	// Implement replace using assignment operators if possible, or destroy+constructor
	template <typename U> void replace_value_with (U && u) {
		replace_value_with_helper (std::forward<U> (u), std::is_assignable<T, U>{});
	}
	template <typename U> void replace_value_with_helper (U && u, std::true_type) {
		value () = std::forward<U> (u);
	}
	template <typename U> void replace_value_with_helper (U && u, std::false_type) {
		destroy ();
		create (std::forward<U> (u));
	}

	// Storage is "mutable" to support muting the object if the optional is const
	T * value_ptr () const noexcept { return reinterpret_cast<T *> (&storage_); }
	mutable typename std::aligned_storage<sizeof (T), alignof (T)>::type storage_;
	bool has_value_;
};

/* a | b -> returns a if a, or b.
 * Both a and b are evaluated (no operator|| lazy semantics).
 * If both a and b are the same kind of reference (lvalue/rvalue), just return the reference.
 * If reference kind differs, an intermediate Optional<T> is built from the right refs.
 */
template <typename T>
const Optional<T> & operator| (const Optional<T> & a, const Optional<T> & b) noexcept {
	return a ? a : b;
}
template <typename T> Optional<T> && operator| (Optional<T> && a, Optional<T> && b) noexcept {
	return a ? std::move (a) : std::move (b);
}
template <typename T> Optional<T> operator| (const Optional<T> & a, Optional<T> && b) {
	return a ? a : std::move (b);
}
template <typename T> Optional<T> operator| (Optional<T> && a, const Optional<T> & b) {
	return a ? std::move (a) : b;
}
}
