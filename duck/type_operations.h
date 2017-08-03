#pragma once

// Utility wrappers for type operations (construction, copy, moves, ...)

#include <duck/debug.h>
#include <stdexcept>
#include <type_traits>
#include <typeinfo>

namespace duck {
namespace Type {
	inline void failure_operation_not_implemented (const std::string & operation,
	                                               const std::type_info & for_type) {
		throw std::runtime_error ("Operation not implemented for type '" + demangle (for_type.name ()) +
		                          "': " + operation);
	}

	/* Type erased functions for various type operations:
	 * - default construction
	 * - destruction
	 * - copy construction
	 * - move construction
	 *
	 * A specialisation for T=void is a noop.
	 */

	// Default construction
	template <typename T> void default_construct_impl (void * storage, std::true_type) {
		new (storage) T ();
	}
	template <typename T> void default_construct_impl (void *, std::false_type) {
		failure_operation_not_implemented ("default constructor", typeid (T));
	}
	template <typename T> void default_construct (void * storage) {
		default_construct_impl<T> (storage, std::is_default_constructible<T>{});
	}
	template <> void default_construct<void> (void *) {}

	// Destructor
	template <typename T> void destroy_impl (void * storage, std::true_type) {
		reinterpret_cast<T *> (storage)->~T ();
	}
	template <typename T> void destroy_impl (void *, std::false_type) {
		failure_operation_not_implemented ("destructor", typeid (T));
	}
	template <typename T> void destroy (void * storage) {
		destroy_impl<T> (storage, std::is_destructible<T>{});
	}
	template <> void destroy<void> (void *) {}

	// Copy construction
	template <typename T>
	void copy_construct_impl (void * storage, const void * from_storage, std::true_type) {
		new (storage) T (*reinterpret_cast<const T *> (from_storage));
	}
	template <typename T> void copy_construct_impl (void *, const void *, std::false_type) {
		failure_operation_not_implemented ("copy constructor", typeid (T));
	}
	template <typename T> void copy_construct (void * storage, const void * from_storage) {
		copy_construct_impl<T> (storage, from_storage, std::is_copy_constructible<T>{});
	}
	template <> void copy_construct<void> (void *, const void *) {}

	// Move construction
	template <typename T>
	void move_construct_impl (void * storage, void * from_storage, std::true_type) {
		new (storage) T (std::move (*reinterpret_cast<T *> (from_storage)));
	}
	template <typename T> void move_construct_impl (void *, void *, std::false_type) {
		failure_operation_not_implemented ("move constructor", typeid (T));
	}
	template <typename T> void move_construct (void * storage, void * from_storage) {
		move_construct_impl<T> (storage, from_storage, std::is_move_constructible<T>{});
	}
	template <> void move_construct<void> (void *, void *) {}

	struct Operations {
		void (*const destroy) (void *);
		void (*const default_construct) (void *);
		void (*const copy_construct) (void *, const void *);
		void (*const move_construct) (void *, void *);
	};
	template <typename T> constexpr Operations operations () noexcept {
		return {destroy<T>, default_construct<T>, copy_construct<T>, move_construct<T>};
	}
}
}
