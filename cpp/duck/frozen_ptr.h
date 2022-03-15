#pragma once

// Single writer, then multiple readers pointer type.
// STATUS: mature

#include <duck/type_traits.h>
#include <memory>
#include <utility>

namespace duck {

// Forward declaration
template <typename T> class FreezablePtr;
template <typename T> class FrozenPtr;

template <typename T> class FreezablePtr {
	/* A unique pointer to a mutable ressource.
	 *
	 * Internally it is a shared_ptr:
	 * - allow shared_from_this classes to not be UB
	 * - avoid a separate control block allocation when converting to FrozenPtr.
	 * Thus copy is manually disabled.
	 */
public:
	FreezablePtr () = default;
	FreezablePtr (const FreezablePtr &) = delete;
	FreezablePtr & operator= (const FreezablePtr &) = delete;
	FreezablePtr (FreezablePtr &&) = default;
	FreezablePtr & operator= (FreezablePtr &&) = default;
	~FreezablePtr () = default;

	// Move construct from unique_ptr ; prefer the static make function.
	FreezablePtr (std::unique_ptr<T> && ptr) noexcept : ptr_ (std::move (ptr)) {}

	// Builds "in place", less overhead than unique_ptr constructor.
	template <typename... Args> static FreezablePtr make (Args &&... args) {
		return {std::make_shared<T> (std::forward<Args> (args)...)};
	}

	// Access
	constexpr explicit operator bool () const noexcept { return bool(ptr_); }
	T * get () const noexcept { return ptr_.get (); }
	T & operator* () const noexcept { return *ptr_; }
	T * operator-> () const noexcept { return get (); }

	/* Conversion:
	 * - always a move to ensure it is unique.
	 * - can upcast if convertible.
	 * - can extract the shared_ptr from here (useful for conversion)
	 */
	template <typename U, typename = enable_if_t<std::is_convertible<U *, T *>::value>>
	explicit FreezablePtr (FreezablePtr<U> && ptr) noexcept
	    : ptr_ (static_cast<std::shared_ptr<U>> (std::move (ptr))) {}
	template <typename U, typename = enable_if_t<std::is_convertible<U *, T *>::value>>
	FreezablePtr & operator= (FreezablePtr<U> && ptr) noexcept {
		ptr_ = static_cast<std::shared_ptr<U>> (std::move (ptr));
		return *this;
	}
	explicit operator std::shared_ptr<T> () && noexcept { return std::move (ptr_); }

	// Transform to a FrozenPtr
	FrozenPtr<T> freeze () &&;

private:
	// Private to avoid violating the unique contract by passing a shared shared_ptr.
	FreezablePtr (std::shared_ptr<T> && ptr) noexcept : ptr_ (std::move (ptr)) {}

	std::shared_ptr<T> ptr_;
};

// Equivalent to std::make_unique
template <typename T, typename... Args> FreezablePtr<T> make_freezable (Args &&... args) {
	return FreezablePtr<T>::make (std::forward<Args> (args)...);
}
template <typename T> FreezablePtr<T> make_freezable (T && t) {
	return FreezablePtr<T>::make (std::forward<T> (t));
}

template <typename T> class FrozenPtr {
	/* A shared pointer to an immutable ressource.
	 */
public:
	using ConstT = add_const_t<T>;

	// All move/copy constructor/assignemnt default

	// Move construct from FreezablePtr
	FrozenPtr (FreezablePtr<T> && ptr) noexcept
	    : ptr_ (static_cast<std::shared_ptr<T>> (std::move (ptr))) {}

	// Build in place (skip Freezable pointer)
	template <typename... Args> static FrozenPtr make (Args &&... args) {
		return std::make_shared<ConstT> (std::forward<Args> (args)...);
	}

	// Access
	constexpr explicit operator bool () const noexcept { return bool(ptr_); }
	ConstT * get () const noexcept { return ptr_.get (); }
	ConstT & operator* () const noexcept { return *ptr_; }
	ConstT * operator-> () const noexcept { return get (); }

	// Conversion: can upcast
	template <typename U, typename = enable_if_t<std::is_convertible<U *, T *>::value>>
	explicit FrozenPtr (const FrozenPtr<U> & ptr) noexcept : ptr_ (ptr.get_shared ()) {}
	template <typename U, typename = enable_if_t<std::is_convertible<U *, T *>::value>>
	explicit FrozenPtr (FrozenPtr<U> && ptr) noexcept : ptr_ (std::move (ptr).get_shared ()) {}
	template <typename U, typename = enable_if_t<std::is_convertible<U *, T *>::value>>
	FrozenPtr & operator= (const FrozenPtr<U> & ptr) noexcept {
		ptr_ = ptr.get_shared ();
		return *this;
	}
	template <typename U, typename = enable_if_t<std::is_convertible<U *, T *>::value>>
	FrozenPtr & operator= (FrozenPtr<U> && ptr) noexcept {
		ptr_ = std::move (ptr).get_shared ();
		return *this;
	}

	// Allow shared_from_this functionnality, may violate the properties !
	// FIXME use a custom base enable_shared_from_this type, and std::is_base_of check
	template <typename SharedFromThisType>
	static FrozenPtr shared_from_this (const SharedFromThisType & t) {
		return FrozenPtr{t.shared_from_this ()};
	}

	// Impl access (considered internal)
	const std::shared_ptr<ConstT> & get_shared () const & noexcept { return ptr_; }
	std::shared_ptr<ConstT> && get_shared () && noexcept { return std::move (ptr_); }

private:
	// Only used by shared_from_this
	FrozenPtr (std::shared_ptr<ConstT> && ptr) noexcept : ptr_ (std::move (ptr)) {}

	std::shared_ptr<ConstT> ptr_;
};

// Similar to std::make_shared
template <typename T, typename... Args> FrozenPtr<T> make_frozen (Args &&... args) {
	return FrozenPtr<T>::make (std::forward<Args> (args)...);
}
template <typename T> FrozenPtr<T> make_frozen (T && t) {
	return FrozenPtr<T>::make (std::forward<T> (t));
}

// Deferred freeze() impl
template <typename T> FrozenPtr<T> FreezablePtr<T>::freeze () && {
	return FrozenPtr<T>{std::move (*this)};
}
} // namespace duck
