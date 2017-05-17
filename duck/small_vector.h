#pragma once

// Vector with small size optimisation (no allocator support)

#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace duck {

template <typename T, typename Allocator = std::allocator<T>>
class SmallVectorBase : private std::allocator_traits<Allocator>::allocator_type {
	/* Base of SmallVector, independent from the inline storage size (N).
	 * This does not have a complete vector API.
	 * In particular, functions that might shrink the storage (and thus may reuse the inline storage)
	 * are only defined in SmallVector itself.
	 * Functions that grow the storage will allocate, so they can be implemented here.
	 */
private:
	// Internal typedefs
	using internal_size_type = std::uint32_t;
	using allocator_traits = std::allocator_traits<Allocator>;

public:
	// STL typedefs
	using allocator_type = typename allocator_traits::allocator_type;
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type &;
	using const_reference = const value_type &;
	using pointer = typename allocator_traits::pointer;
	using const_pointer = typename allocator_traits::const_pointer;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static_assert (sizeof (size_type) >= sizeof (internal_size_type),
	               "size_type is smaller than internal_size_type");

	// Basic TODO all destr / constr / assign / operator=
	~SmallVectorBase () {
		clear ();
		free_storage_if_allocated ();
	}
	allocator_type get_allocator () const { return *this; }

	// Element access
	reference at (size_type pos) { return pos < size_ ? nth (pos) : throw std::out_of_range{"at()"}; }
	constexpr const_reference at (size_type pos) const {
		return pos < size_ ? nth (pos) : throw std::out_of_range{"at()"};
	}
	reference operator[] (size_type pos) noexcept { return nth (pos); }
	constexpr const_reference operator[] (size_type pos) const noexcept { return nth (pos); }
	reference front () noexcept { return nth (0); }
	constexpr const_reference front () const noexcept { return nth (0); }
	reference back () noexcept { return nth (size_ - 1); }
	constexpr const_reference back () const noexcept { return nth (size_ - 1); }
	T * data () noexcept { return nthp (0); }
	constexpr const T * data () const noexcept { return nthp (0); }

	// Iterators
	iterator begin () noexcept { return data (); }
	constexpr const_iterator begin () const noexcept { return nthp (0); }
	constexpr const_iterator cbegin () const noexcept { return nthp (0); }
	iterator end () noexcept { return data () + size_; }
	constexpr const_iterator end () const noexcept { return nthp (size_); }
	constexpr const_iterator cend () const noexcept { return nthp (size_); }
	reverse_iterator rbegin () noexcept { return end (); }
	constexpr const_reverse_iterator rbegin () const noexcept { return end (); }
	constexpr const_reverse_iterator crbegin () const noexcept { return end (); }
	reverse_iterator rend () noexcept { return begin (); }
	constexpr const_reverse_iterator rend () const noexcept { return begin (); }
	constexpr const_reverse_iterator crend () const noexcept { return begin (); }

	// Capacity
	constexpr bool empty () const noexcept { return size_ == 0; }
	constexpr size_type size () const noexcept { return size_; }
	constexpr size_type max_size () const noexcept {
		return min (std::numeric_limits<internal_size_type>::max (),
		            allocator_traits::max_size (*this));
	}
	void reserve (size_type new_cap) {
		// Reserve just moves data to the exact required capacity
		if (new_cap > capacity_)
			move_to_new_allocated_storage (new_cap);
	}
	constexpr size_type capacity () const noexcept { return capacity_; }

	// Modifiers TODO insert emplace
	void clear () noexcept {
		for (; size_ > 0; --size_)
			allocator_traits::destroy (*this, nthp (size_ - 1));
	}
	reference push_back (const T & value) { return emplace_back (value); }
	reference push_back (T && value) { return emplace_back (std::move (value)); }
	template <typename... Args> reference emplace_back (Args &&... args) {
		grow_if_needed (1);
		auto * object = nthp (size_);
		allocator_traits::construct (*this, object, std::forward<Args> (args)...);
		++size_;
		return *object;
	}
	void pop_back () noexcept {
		allocator_traits::destroy (*this, nthp (size_ - 1));
		--size_;
	}
	void resize (size_type count) {
		reserve (count);
		for (; size_ > count; --size_)
			allocator_traits::destroy (*this, nthp (size_ - 1));
		for (; size_ < count; ++size_)
			allocator_traits::construct (*this, nthp (size_));
	}
	void resize (size_type count, const value_type & value) {
		reserve (count);
		for (; size_ > count; --size_)
			allocator_traits::destroy (*this, nthp (size_ - 1));
		for (; size_ < count; ++size_)
			allocator_traits::construct (*this, nthp (size_), value);
	}

	// SmallVector specific API
	constexpr bool is_allocated () const noexcept { return data_ != inline_storage_ptr (); }

protected:
	// Initialized as empty, with the inline storage.
	SmallVectorBase (size_type initial_capacity, const Allocator & allocator) noexcept
	    : allocator_type (allocator),
	      size_ (0),
	      capacity_ (initial_capacity),
	      data_ (inline_storage_ptr ()) {}

	// Internal accessors (marked const for performance)
	constexpr pointer nthp (internal_size_type index) const noexcept { return data_ + index; }
	constexpr reference nth (internal_size_type index) const noexcept { return *nthp (index); }

	void free_storage_if_allocated () {
		if (is_allocated ())
			allocator_traits::deallocate (*this, data_, capacity_);
	}
	void grow_if_needed (internal_size_type will_insert) {
		if (size_ + will_insert > capacity_)
			move_to_new_allocated_storage (capacity_ * 2);
	}
	void move_to_new_allocated_storage (internal_size_type new_cap) {
		// This function creates a new allocated storage and relocates the current data to it.
		pointer new_storage = allocator_traits::allocate (*this, new_cap);
		for (internal_size_type i = 0; i < size_; ++i) {
			allocator_traits::construct (*this, new_storage + i, std::move (data_[i]));
			allocator_traits::destroy (*this, data_ + i);
		}
		free_storage_if_allocated ();
		data_ = new_storage;
		capacity_ = new_cap;
	}

	// Access inline storage
	pointer inline_storage_ptr () noexcept;
	const_pointer inline_storage_ptr () const noexcept;

	internal_size_type size_;
	internal_size_type capacity_;
	pointer data_;
};

template <typename T, std::size_t N, typename Allocator = std::allocator<T>>
class SmallVector : public SmallVectorBase<T, Allocator> {
private:
	using Base = SmallVectorBase<T, Allocator>;

public:
	using size_type = typename Base::size_type;

	// Basic
	SmallVector () : SmallVector (Allocator ()) {}
	explicit SmallVector (const Allocator & allocator) noexcept : Base (N, allocator) {}

	SmallVector (size_type size, const T & value, const Allocator & allocator = Allocator ())
	    : SmallVector (allocator) {
		resize (size, value);
	}
	explicit SmallVector (size_type size, const Allocator & allocator = Allocator ())
	    : SmallVector (allocator) {
		resize (size);
	}

	// TODO copy and move constructors
	template <typename InputIt>
	SmallVector (InputIt first, InputIt last, const Allocator & allocator = Allocator ());

	// Element access: all in SmallVectorBase

	// Iterators: all in SmallVectorBase

	// Capacity
	void shrink_to_fit ();

	// Modifiers
	void swap (/* ??? TODO */) noexcept;

	//
	typename Base::pointer inline_storage_ptr () noexcept {
		return reinterpret_cast<typename Base::pointer> (&inline_storage_);
	}
	typename Base::const_pointer inline_storage_ptr () const noexcept {
		return reinterpret_cast<typename Base::const_pointer> (&inline_storage_);
	}

private:
	typename std::aligned_storage<N * sizeof (T), alignof (T)>::type inline_storage_;
};

// Inline storage should be placed at the same address irrelevant of size.
template <typename T, typename Allocator>
typename SmallVectorBase<T, Allocator>::pointer
SmallVectorBase<T, Allocator>::inline_storage_ptr () noexcept {
	return static_cast<SmallVector<T, 1, Allocator> *> (this)->inline_storage_ptr ();
}
template <typename T, typename Allocator>
typename SmallVectorBase<T, Allocator>::const_pointer
SmallVectorBase<T, Allocator>::inline_storage_ptr () const noexcept {
	return static_cast<const SmallVector<T, 1, Allocator> *> (this)->inline_storage_ptr ();
}
}
