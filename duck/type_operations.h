#pragma once

// Utility wrappers for type operations (construction, copy, moves, ...)

#include <cassert>
#include <type_traits>

namespace duck {
namespace Type {
	/* Type erased functions for various type operations:
	 * - default construction
	 * - destruction
	 * - copy construction / assignment
	 * - move construction / assignment
	 *
	 * A specialisation for T == void is a noop.
	 * For types that do not provide some operators, these functions trigger an assert.
	 */

	// Default construction
	template <typename T> void default_construct_impl (void * storage, std::true_type) {
		new (storage) T ();
	}
	template <typename> void default_construct_impl (void *, std::false_type) {
		assert (!"no default constructor");
	}
	template <typename T> void default_construct (void * storage) {
		default_construct_impl<T> (storage, std::is_default_constructible<T>{});
	}
	template <> void default_construct<void> (void *) {}

	// Destructor
	template <typename T> void destroy_impl (void * storage, std::true_type) {
		reinterpret_cast<T *> (storage)->~T ();
	}
	template <typename> void destroy_impl (void *, std::false_type) { assert (!"no destructor"); }
	template <typename T> void destroy (void * storage) {
		destroy_impl<T> (storage, std::is_destructible<T>{});
	}
	template <> void destroy<void> (void *) {}

	// Copy construction
	template <typename T>
	void copy_construct_impl (void * storage, const void * from_storage, std::true_type) {
		new (storage) T (*reinterpret_cast<const T *> (from_storage));
	}
	template <typename> void copy_construct_impl (void *, const void *, std::false_type) {
		assert (!"no copy constructor");
	}
	template <typename T> void copy_construct (void * storage, const void * from_storage) {
		copy_construct_impl<T> (storage, from_storage, std::is_copy_constructible<T>{});
	}
	template <> void copy_construct<void> (void *, const void *) {}

	// Copy assignment
	template <typename T>
	void copy_assign_impl (void * storage, const void * from_storage, std::true_type) {
		*reinterpret_cast<T *> (storage) = *reinterpret_cast<const T *> (from_storage);
	}
	template <typename> void copy_assign_impl (void *, const void *, std::false_type) {
		assert (!"no copy assignment operator");
	}
	template <typename T> void copy_assign (void * storage, const void * from_storage) {
		copy_assign_impl<T> (storage, from_storage, std::is_copy_assignable<T>{});
	}
	template <> void copy_assign<void> (void *, const void *) {}

	// Move construction
	template <typename T>
	void move_construct_impl (void * storage, void * from_storage, std::true_type) {
		new (storage) T (std::move (*reinterpret_cast<T *> (from_storage)));
	}
	template <typename> void move_construct_impl (void *, void *, std::false_type) {
		assert (!"no move constructor");
	}
	template <typename T> void move_construct (void * storage, void * from_storage) {
		move_construct_impl<T> (storage, from_storage, std::is_move_constructible<T>{});
	}
	template <> void move_construct<void> (void *, void *) {}

	// Move assignment
	template <typename T>
	void move_assign_impl (void * storage, void * from_storage, std::true_type) {
		*reinterpret_cast<T *> (storage) = std::move (*reinterpret_cast<T *> (from_storage));
	}
	template <typename> void move_assign_impl (void *, void *, std::false_type) {
		assert (!"no move assignment operator");
	}
	template <typename T> void move_assign (void * storage, void * from_storage) {
		move_assign_impl<T> (storage, from_storage, std::is_move_assignable<T>{});
	}
	template <> void move_assign<void> (void *, void *) {}

	struct Operations {
		void (*const destroy) (void *);
		void (*const copy_construct) (void *, const void *);
		void (*const copy_assign) (void *, const void *);
		void (*const move_construct) (void *, void *);
		void (*const move_assign) (void *, void *);
		void (*const default_construct) (void *);
	};
	template <typename T> constexpr Operations operations () noexcept {
		return {destroy<T>,        copy_construct<T>, copy_assign<T>,
		        move_construct<T>, move_assign<T>,    default_construct<T>};
	}
}
}
