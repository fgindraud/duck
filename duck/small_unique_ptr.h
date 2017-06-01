#pragma once

// Analog to std::unique_ptr<T>, but has a local storage to avoid new() for small objects
// Intended to store small virtual classes with less overhead.

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
	~SmallUniquePtr () { clear (); }

	// Modifiers
	void clear () noexcept {
		if (data_) {
			if (is_allocated ())
				delete data_;
			else
				data_->~T ();
			data_ = nullptr;
		}
	}

	template <typename U = T, typename... Args> void emplace (Args &&... args) {
		clear ();
		build<U> (std::forward<Args> (args)...);
	}
	template <typename U = T, typename V, typename... Args>
	void emplace (std::initializer_list<V> ilist, Args &&... args) {
		clear ();
		build<U> (std::move (ilist), std::forward<Args> (args)...);
	}

	// Observers
	constexpr pointer get () const noexcept { return data_; }
	constexpr operator bool () const noexcept { return data_; }
	pointer operator-> () const noexcept { return data_; }
	typename std::add_lvalue_reference<T>::type operator* () const noexcept { return *data_; }

	// Both functions are undefined is pointer is null
	bool is_inline () const noexcept {
		return is_inline_helper_if_polymorphic (std::is_polymorphic<T>{});
	}
	bool is_allocated () const noexcept { return !is_inline (); }

private:
	bool is_inline_helper_if_polymorphic (std::true_type) const noexcept {
		/* If T is polymorphic, we can expect derived classes to be used.
		 * Depending on the layout of derived classes, data_ may be different from the storage pointer.
		 * Checking that data_ == &inline_storage_ is not sufficient.
		 * The condition is to check that data_ is inside the inline_storage_ buffer.
		 */
		auto inline_storage_as_byte = reinterpret_cast<const unsigned char *> (&inline_storage_);
		auto data_as_byte = reinterpret_cast<const unsigned char *> (data_);
		return inline_storage_as_byte <= data_as_byte &&
		       data_as_byte < inline_storage_as_byte + StorageLen;
	}
	bool is_inline_helper_if_polymorphic (std::false_type) const noexcept {
		/* If T is not polymorphic, using a derived class is UB (non-virtual destructor).
		 * Thus we only expect T to be used, and we can just compare pointers.
		 */
		return reinterpret_cast<const_pointer> (&inline_storage_) == data_;
	}

	template <typename U>
	using BuildInline =
	    std::integral_constant<bool, (sizeof (U) <= StorageLen && alignof (U) <= storage_align)>;

	template <typename U> pointer create_storage () {
		return create_storage_helper (sizeof (U), BuildInline<U>{});
	}
	pointer create_storage_helper (std::size_t, std::true_type) {
		return reinterpret_cast<pointer> (&inline_storage_);
	}
	pointer create_storage_helper (std::size_t size, std::false_type) {
		return static_cast<pointer> (::operator new (size));
	}

	template <typename U, typename... Args> void build (Args &&... args) {
		static_assert (std::is_base_of<T, U>::value, "build object must derive from T");
		auto p = create_storage<U> ();
		data_ = new (p) U (std::forward<Args> (args)...);
	}

	pointer data_{nullptr}; // Points to the T object, not the chosen storage
	StorageType inline_storage_;
};

// Comparison TODO ?
}
