#pragma once

// Single writer, then multiple readers pointer type.
// STATUS: mature

#include <cassert>
#include <memory>
#include <type_traits>

namespace duck {

// Forward declaration
template <typename T> class FreezableUniquePtr;
template <typename T> class FrozenSharedPtr;

template <typename T> class FreezableUniquePtr {
	/* A unique pointer to a mutable ressource.
	 *
	 * Internally it is a shared_ptr:
	 * - allow shared_from_this classes to not be UB
	 * - avoid a separate control block allocation when converting to FrozenSharedPtr.
	 * Thus copy is manually disabled.
	 */
public:
	FreezableUniquePtr () = default;
	FreezableUniquePtr (const FreezableUniquePtr &) = delete;
	FreezableUniquePtr & operator= (const FreezableUniquePtr &) = delete;
	FreezableUniquePtr (FreezableUniquePtr &&) = default;
	FreezableUniquePtr & operator= (FreezableUniquePtr &&) = default;
	~FreezableUniquePtr () = default;

	// Move construct from unique_ptr ; prefer the static make function.
	explicit FreezableUniquePtr (std::unique_ptr<T> && ptr) noexcept : ptr_ (std::move (ptr)) {}

	// Builds "in place", less overhead than unique_ptr constructor.
	template <typename... Args> static FreezableUniquePtr make (Args &&... args) {
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
	template <typename U,
	          typename = typename std::enable_if<std::is_convertible<U *, T *>::value>::type>
	explicit FreezableUniquePtr (FreezableUniquePtr<U> && ptr) noexcept
	    : ptr_ (static_cast<std::shared_ptr<U>> (std::move (ptr))) {}
	template <typename U,
	          typename = typename std::enable_if<std::is_convertible<U *, T *>::value>::type>
	FreezableUniquePtr & operator= (FreezableUniquePtr<U> && ptr) noexcept {
		ptr_ = static_cast<std::shared_ptr<U>> (std::move (ptr));
		return *this;
	}
	explicit operator std::shared_ptr<T> () && noexcept { return std::move (ptr_); }

	// Transform to a FrozenSharedPtr
	FrozenSharedPtr<T> freeze () &&;

private:
	// Private to avoid violating the unique contract by passing a shared shared_ptr.
	FreezableUniquePtr (std::shared_ptr<T> && ptr) noexcept : ptr_ (std::move (ptr)) {}

	std::shared_ptr<T> ptr_;
};

template <typename T, typename... Args>
FreezableUniquePtr<T> make_freezable_unique (Args &&... args) {
	return FreezableUniquePtr<T>::make (std::forward<Args> (args)...);
}

template <typename T> class FrozenSharedPtr {
	/* A shared pointer to an immutable ressource.
	 */
public:
	using ConstT = typename std::add_const<T>::type;

	// All move/copy constructor/assignemnt default

	// Move construct from FreezableUniquePtr
	explicit FrozenSharedPtr (FreezableUniquePtr<T> && ptr) noexcept
	    : ptr_ (static_cast<std::shared_ptr<T>> (std::move (ptr))) {}

	// Access
	constexpr explicit operator bool () const noexcept { return bool(ptr_); }
	ConstT * get () const noexcept { return ptr_.get (); }
	ConstT & operator* () const noexcept { return *ptr_; }
	ConstT * operator-> () const noexcept { return get (); }

	// Conversion: can upcast
	template <typename U,
	          typename = typename std::enable_if<std::is_convertible<U *, T *>::value>::type>
	explicit FrozenSharedPtr (const FrozenSharedPtr<U> & ptr) noexcept : ptr_ (ptr.get_shared ()) {}
	template <typename U,
	          typename = typename std::enable_if<std::is_convertible<U *, T *>::value>::type>
	explicit FrozenSharedPtr (FrozenSharedPtr<U> && ptr) noexcept
	    : ptr_ (std::move (ptr).get_shared ()) {}
	template <typename U,
	          typename = typename std::enable_if<std::is_convertible<U *, T *>::value>::type>
	FrozenSharedPtr & operator= (const FrozenSharedPtr<U> & ptr) noexcept {
		ptr_ = ptr.get_shared ();
		return *this;
	}
	template <typename U,
	          typename = typename std::enable_if<std::is_convertible<U *, T *>::value>::type>
	FrozenSharedPtr & operator= (FrozenSharedPtr<U> && ptr) noexcept {
		ptr_ = std::move (ptr).get_shared ();
		return *this;
	}

	// Allow shared_from_this functionnality, may violate the properties !
  // FIXME make it safer
  // use a custom base enable_shared_from_this type, and std::is_base_of check ?
	template <typename SharedFromThisType>
	static FrozenSharedPtr shared_from_this (const SharedFromThisType & t) {
		return FrozenSharedPtr{t.shared_from_this ()};
	}

	// Impl access (considered internal)
	const std::shared_ptr<ConstT> & get_shared () const & noexcept { return ptr_; }
	std::shared_ptr<ConstT> && get_shared () && noexcept { return std::move (ptr_); }

private:
	std::shared_ptr<ConstT> ptr_;
};

template <typename T> FrozenSharedPtr<T> FreezableUniquePtr<T>::freeze () && {
	return FrozenSharedPtr<T>{std::move (*this)};
}
}
