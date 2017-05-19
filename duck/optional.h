#pragma once

// In place optional type

#include <cassert>
#include <type_traits>
#include <utility>

namespace duck {

struct InPlace {};

template <typename T> class Optional {
public:
	using value_type = T;

	constexpr Optional () = default;
	Optional (const Optional & other) {
		if (other)
			create (*other);
	}
	Optional (Optional && other) noexcept {
		if (other)
			create (*std::move (other));
	}
	template <typename... Args> Optional (InPlace, Args &&... args) {
		create (std::forward<Args> (args)...);
	}

	~Optional () {
		if (has_value_)
			destroy ();
	}

	T & value () & noexcept {
		assert (has_value_);
		return *value_ptr ();
	}
	const T & value () const & noexcept {
		assert (has_value_);
		return *value_ptr ();
	}
	T && value () && noexcept {
		assert (has_value_);
		return std::move (*value_ptr ());
	}
	const T && value () const && noexcept {
		assert (has_value_);
		return std::move (*value_ptr ());
	}

	T * operator-> () noexcept { return value_ptr (); }
	constexpr const T * operator-> () const noexcept { return value_ptr (); }
	T & operator* () & noexcept { return *value_ptr (); }
	constexpr const T & operator* () const & noexcept { return *value_ptr (); }
	T && operator* () && noexcept { return std::move (*value_ptr ()); }
	constexpr const T && operator* () const && noexcept { return std::move (*value_ptr ()); }

	constexpr bool has_value () const noexcept { return has_value_; }
	constexpr operator bool () const noexcept { return has_value_; }

	template <typename U> constexpr T value_or (U && default_value) const & {
		return has_value_ ? *value_ptr () : static_cast<T> (std::forward<U> (default_value));
	}
	template <typename U> T value_or (U && default_value) && {
		return has_value_ ? std::move (*value_ptr ())
		                  : static_cast<T> (std::forward<U> (default_value));
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

template <typename T> class Optional<T &> {
public:
	using value_type = T &;

	constexpr Optional () = default;

	T & value () const noexcept {
		assert (has_value ());
		return *ptr_;
	}

	// no operator->
	T & operator* () const noexcept { return *ptr_; }

	constexpr bool has_value () const noexcept { return ptr_ != nullptr; }
	constexpr operator bool () const noexcept { return has_value (); }

	template <typename U> constexpr T & value_or (U & default_value) const noexcept {
		return has_value () ? *ptr_ : static_cast<T &> (default_value);
	}

private:
	T * ptr_{nullptr};
};
}
