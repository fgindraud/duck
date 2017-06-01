#pragma once

// Analog to std::unique_ptr<T>, but has a local storage to avoid new() for small objects
// Intended to store small virtual classes with less overhead.
// TODO continue. will differ from unique_ptr.

#include <cassert>
#include <type_traits>
#include <utility>

namespace duck {

// TODO it will require T to have virtual move() if using move assign/constr
// TODO same problems as smallvector : needs a base ?
// TODO add alignment template arg ?

template <typename T, std::size_t StorageLen> class SmallUniquePtr {
private:
	using StorageType = typename std::aligned_storage<StorageLen>::type;
	static constexpr auto storage_align = alignof (StorageType);

public:
	using pointer = T *;
	using const_pointer = const T *;

	// Constructors
	SmallUniquePtr () = default;

	// Modifiers
	template <typename U, typename... Args> void emplace (Args &&... args) {
		static_assert (std::is_base_of<T, U>::value, "emplace must be used with type derived from T");
		// TODO clean old ; exception safety
		auto p = create_storage<U> ();
		data_ = new (p) U (std::forward<Args> (args)...);
	}
	template <typename U, typename V, typename... Args> void emplace (std::initializer_list<V> ilist, Args &&... args) {
		static_assert (std::is_base_of<T, U>::value, "emplace must be used with type derived from T");
		auto p = create_storage<U> ();
		data_ = new (p) U (std::move (ilist), std::forward<Args> (args)...);
	}

	// FIXME what if data differs from create_storage<T> value ?
	// Does delete handle it ?

	// Observers
	constexpr pointer get () const noexcept { return data_; }
	constexpr operator bool () const noexcept { return data_; }
	pointer operator-> () const noexcept { return data_; }
	typename std::add_lvalue_reference<T>::type operator* () const noexcept { return *data_; }

	bool is_allocated () const noexcept {
		return data_ != nullptr && is_allocated_assuming_non_null ();
	}
	bool is_allocated_assuming_non_null () const noexcept { return data_ != get_inline_storage (); }

private:
	pointer get_inline_storage () noexcept { return reinterpret_cast<pointer> (&storage_); }
	const_pointer get_inline_storage () const noexcept {
		return reinterpret_cast<const_pointer> (&storage_);
	}

	template <typename U,
	          typename = typename std::enable_if<(sizeof (U) <= StorageLen &&
	                                              alignof (U) <= storage_align)>::type>
	pointer create_storage () {
		return get_inline_storage ();
	}

	pointer data_{nullptr};
	StorageType storage_;
};

// Comparison TODO ?
}
