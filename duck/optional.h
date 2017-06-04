#pragma once

// In place optional type

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
	 * A moved-from Optional still contains a value.
	 */
	static_assert (!std::is_reference<T>::value, "Optional<T> does not support references");

public:
	using value_type = T;

	constexpr Optional () = default;
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
			value () = *other;
		else if (!has_value () && other)
			create (*other);
		else if (has_value () && !other)
			destroy ();
		return *this;
	}
	Optional & operator= (Optional && other) noexcept {
		if (has_value () && other)
			value () = std::move (*other);
		else if (!has_value () && other)
			create (std::move (*other));
		else if (has_value () && !other)
			destroy ();
		return *this;
	}
	Optional & operator= (const T & t) {
		if (has_value ())
			value () = t;
		else
			create (t);
		return *this;
	}
	Optional & operator= (T && t) noexcept {
		if (has_value ())
			value () = std::move (t);
		else
			create (std::move (t));
		return *this;
	}

	// Unchecked access
	T & value () & noexcept {
		assert (has_value ());
		return *value_ptr ();
	}
	const T & value () const & noexcept {
		assert (has_value ());
		return *value_ptr ();
	}
	T && value () && noexcept {
		assert (has_value ());
		return std::move (*value_ptr ());
	}
	const T && value () const && noexcept {
		assert (has_value ());
		return std::move (*value_ptr ());
	}

	// Operator unchecked access
	T * operator-> () noexcept { return value_ptr (); }
	constexpr const T * operator-> () const noexcept { return value_ptr (); }
	T & operator* () & noexcept { return *value_ptr (); }
	constexpr const T & operator* () const & noexcept { return *value_ptr (); }
	T && operator* () && noexcept { return std::move (*value_ptr ()); }
	constexpr const T && operator* () const && noexcept { return std::move (*value_ptr ()); }

	// Status test
	constexpr bool has_value () const noexcept { return has_value_; }
	constexpr operator bool () const noexcept { return has_value (); }

	// Access with default
	template <typename U> constexpr T value_or (U && default_value) const & {
		return has_value () ? value () : static_cast<T> (std::forward<U> (default_value));
	}
	template <typename U> T value_or (U && default_value) && {
		return has_value () ? std::move (value ()) : static_cast<T> (std::forward<U> (default_value));
	}

	// Access with generated default
	template <typename Callable> T value_or_generate (Callable && callable) const & {
		return has_value () ? value () : std::forward<Callable> (callable) ();
	}
	template <typename Callable> T value_or_generate (Callable && callable) && {
		return has_value () ? std::move (value ()) : std::forward<Callable> (callable) ();
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
			return std::forward<Callable> (callable) (std::move (value ()));
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

	T * value_ptr () noexcept { return reinterpret_cast<T *> (&storage_); }
	constexpr const T * value_ptr () const noexcept {
		return reinterpret_cast<const T *> (&storage_);
	}

	typename std::aligned_storage<sizeof (T), alignof (T)>::type storage_;
	bool has_value_{false};
};
}
