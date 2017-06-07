#pragma once

// Vector with small size optimisation (no allocator support)
// STATUS: prototype

#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

namespace duck {

constexpr std::size_t small_vector_minimum_inline_size = 1;

template <typename T> class SmallVectorBase {
	/* Base of SmallVector, independent from the inline storage size (N).
	 *
	 * This has a quasi complete vector API. Missing stuff:
	 * - No Allocator support (too crazy, uses default malloc/free)
	 * - Functions that can shrink the storage assume inline capacity is 1.
	 *
	 * TODO replace clear()+append() by a replace() in many places
	 * (replace copy/move assigns on already built Ts instead of destroying + recreating)
	 */
private:
	// Internal typedefs
	using internal_size_type = std::uint32_t;

public:
	// STL typedefs
	using value_type = T;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using reference = value_type &;
	using const_reference = const value_type &;
	using pointer = value_type *;
	using const_pointer = const value_type *;
	using iterator = pointer;
	using const_iterator = const_pointer;
	using reverse_iterator = std::reverse_iterator<iterator>;
	using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	static_assert (sizeof (size_type) >= sizeof (internal_size_type),
	               "size_type is smaller than internal_size_type");

	// Basic (no constructor)
	SmallVectorBase () = delete;
	SmallVectorBase (const SmallVectorBase &) = delete;
	SmallVectorBase (SmallVectorBase &&) = delete;
	~SmallVectorBase () {
		clear ();
		free_storage_if_allocated ();
		set_storage_to_inline ();
	}

	SmallVectorBase & operator= (const SmallVectorBase & other) {
		clear ();
		append_sequence (other.begin (), other.end ());
		return *this;
	}
	SmallVectorBase & operator= (SmallVectorBase && other) noexcept {
		clear ();
		if (other.is_allocated ()) {
			// Steal buffer
			free_storage_if_allocated ();
			size_ = other.size_;
			capacity_ = other.capacity_;
			data_ = other.data_;
			other.size_ = 0;
			other.set_storage_to_inline ();
		} else {
			// Move elements
			append_sequence_by_move (other.begin (), other.end ());
		}
		return *this;
	}
	SmallVectorBase & operator= (std::initializer_list<T> ilist) {
		assign (std::move (ilist));
		return *this;
	}

	void assign (size_type count, const T & value) {
		clear ();
		copy_construct_until_size_is (count, value);
	}
	template <typename InputIt,
	          typename Category = typename std::iterator_traits<InputIt>::iterator_category>
	void assign (InputIt first, InputIt last) {
		clear ();
		append_sequence_impl (std::move (first), std::move (last), Category{});
	}
	void assign (const std::initializer_list<T> ilist) {
		clear ();
		append_sequence_by_move (ilist.begin (), ilist.end ());
	}

	// Element access
	reference at (size_type pos) {
		if (pos >= size_)
			throw std::out_of_range{"at()"};
		return nth (pos);
	}
	const_reference at (size_type pos) const {
		if (pos >= size_)
			throw std::out_of_range{"at()"};
		return nth (pos);
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
		return std::numeric_limits<internal_size_type>::max ();
	}
	void reserve (size_type new_cap) {
		// Reserve just moves data to the exact required capacity
		if (new_cap > capacity_)
			move_to_new_allocated_storage (new_cap);
	}
	constexpr size_type capacity () const noexcept { return capacity_; }
	void shrink_to_fit () { shrink_to_fit_impl (small_vector_minimum_inline_size); }

	// Modifiers TODO insert emplace erase
	void clear () noexcept { delete_backward_until_size_is (0); }
	reference push_back (const T & value) { return emplace_back (value); }
	reference push_back (T && value) { return emplace_back (std::move (value)); }
	template <typename... Args> reference emplace_back (Args &&... args) {
		grow_if_needed (1);
		return unchecked_emplace_back (std::forward<Args> (args)...);
	}
	void pop_back () noexcept {
		nthp (size_ - 1)->~T ();
		--size_;
	}
	void resize (size_type count) {
		delete_backward_until_size_is (count);
		default_construct_until_size_is (count);
	}
	void resize (size_type count, const value_type & value) {
		delete_backward_until_size_is (count);
		copy_construct_until_size_is (count, value);
	}
	void swap (SmallVectorBase & other) noexcept {
		using std::swap;
		// TODO still problematic
	}

	// SmallVector specific API
	constexpr bool is_allocated () const noexcept { return data_ != inline_storage_ptr (); }
	template <typename... Args> reference unchecked_emplace_back (Args &&... args) {
		// Does not check for capacity
		auto * object = nthp (size_);
		::new (object) T (std::forward<Args> (args)...);
		++size_;
		return *object;
	}
	template <typename InputIt,
	          typename Category = typename std::iterator_traits<InputIt>::iterator_category>
	void append_sequence (InputIt first, InputIt last) {
		append_sequence_impl (std::move (first), std::move (last), Category{});
	}
	template <typename InputIt,
	          typename Category = typename std::iterator_traits<InputIt>::iterator_category>
	void append_sequence_by_move (InputIt first, InputIt last) {
		append_sequence_by_move_impl (std::move (first), std::move (last), Category{});
	}

protected:
	// Always initialized as empty
	SmallVectorBase (pointer initial_storage, size_type initial_capacity) noexcept
	    : size_ (0), capacity_ (initial_capacity), data_ (initial_storage) {}
	// Default to inline_storage (capacity must be given by SmallVector<T, N>)
	SmallVectorBase (size_type initial_capacity) noexcept
	    : SmallVectorBase (inline_storage_ptr (), initial_capacity) {}

	// Internal accessors (marked const for performance)
	constexpr pointer nthp (internal_size_type index) const noexcept { return data_ + index; }
	constexpr reference nth (internal_size_type index) const noexcept { return *nthp (index); }

	// Build/delete helpers
	void delete_backward_until_size_is (internal_size_type target_count) noexcept {
		while (size_ > target_count)
			pop_back ();
	}
	void default_construct_until_size_is (internal_size_type target_count) {
		reserve (target_count);
		while (size_ < target_count)
			unchecked_emplace_back ();
	}
	void copy_construct_until_size_is (internal_size_type target_count, const T & value) {
		reserve (target_count);
		while (size_ < target_count)
			unchecked_emplace_back (value);
	}
	template <typename It> void append_sequence_impl (It first, It last, std::input_iterator_tag) {
		for (; first != last; ++first)
			emplace_back (*first);
	}
	template <typename It>
	void append_sequence_impl (It first, It last, std::random_access_iterator_tag) {
		// More efficient if we can compute the size
		reserve (size () + (last - first));
		for (; first != last; ++first)
			unchecked_emplace_back (*first);
	}
	template <typename It>
	void append_sequence_by_move_impl (It first, It last, std::input_iterator_tag) {
		for (; first != last; ++first)
			emplace_back (std::move (*first));
	}
	template <typename It>
	void append_sequence_by_move_impl (It first, It last, std::random_access_iterator_tag) {
		// More efficient if we can compute the size
		reserve (size () + (last - first));
		for (; first != last; ++first)
			unchecked_emplace_back (std::move (*first));
	}

	// Storage helpers
	void free_storage_if_allocated () {
		if (is_allocated ())
			::operator delete (data_);
	}
	void set_storage_to_inline () {
		data_ = inline_storage_ptr ();
		capacity_ = small_vector_minimum_inline_size;
	}
	void grow_if_needed (internal_size_type will_insert) {
		if (size_ + will_insert > capacity_)
			move_to_new_allocated_storage (capacity_ * 2);
	}
	void move_to_new_storage (pointer new_storage, internal_size_type new_cap) {
		// Relocates data to a new storage (no checks).
		for (internal_size_type i = 0; i < size_; ++i) {
			new (new_storage + i) T (std::move (data_[i]));
			data_[i].~T ();
		}
		free_storage_if_allocated ();
		data_ = new_storage;
		capacity_ = new_cap;
	}
	void move_to_new_allocated_storage (internal_size_type new_cap) {
		// Create a new allocated storage and relocate current data to it.
		move_to_new_storage (static_cast<pointer> (::operator new (new_cap * sizeof (T))), new_cap);
	}

	// Access inline storage (implemented by static upcast to the min SmallVector size).
	pointer inline_storage_ptr () noexcept;
	const_pointer inline_storage_ptr () const noexcept;

	// Common implementations for SmallVector<N>
	void shrink_to_fit_impl (internal_size_type inline_storage_capacity) {
		if (size_ == capacity_ || capacity_ == inline_storage_capacity)
			return; // Cannot shrink
		if (size_ <= inline_storage_capacity)
			move_to_new_storage (inline_storage_ptr (), inline_storage_capacity);
		else
			move_to_new_allocated_storage (size_);
	}

private:
	internal_size_type size_;
	internal_size_type capacity_;
	pointer data_;
};

template <typename T, std::size_t N> class SmallVector : public SmallVectorBase<T> {
private:
	static_assert (N > 0, "SmallVector must have non zero inline storage size");
	friend class SmallVectorBase<T>; // for Base::inline_storage_ptr()
	using Base = SmallVectorBase<T>;

public:
	using size_type = typename Base::size_type;

	// Basic
	SmallVector () noexcept : Base (N) {}

	SmallVector (size_type size, const T & value) : SmallVector () {
		this->copy_construct_until_size_is (size, value);
	}
	explicit SmallVector (size_type size) : SmallVector () {
		this->default_construct_until_size_is (size);
	}

	SmallVector (const SmallVectorBase<T> & other) : SmallVector () { *this = other; }

	template <typename InputIt,
	          typename Category = typename std::iterator_traits<InputIt>::iterator_category>
	SmallVector (InputIt first, InputIt last) : SmallVector () {
		this->append_sequence_impl (first, last, Category{});
	}

	SmallVector (SmallVectorBase<T> && other) noexcept : SmallVector () { *this = std::move (other); }

	SmallVector (std::initializer_list<T> ilist) : SmallVector () { *this = std::move (ilist); }

	// Element access: all in SmallVectorBase

	// Iterators: all in SmallVectorBase

	// Capacity
	void shrink_to_fit () { this->shrink_to_fit_impl (N); } // override with real size

	// Modifiers: all in SmallVectorBase

private:
	typename std::aligned_storage<N * sizeof (T), alignof (T)>::type inline_storage_;
};

// Inline storage should be placed at the same address irrelevant of size.
template <typename T> auto SmallVectorBase<T>::inline_storage_ptr () noexcept -> pointer {
	return reinterpret_cast<pointer> (
	    &static_cast<SmallVector<T, small_vector_minimum_inline_size> *> (this)->inline_storage_);
}
template <typename T>
auto SmallVectorBase<T>::inline_storage_ptr () const noexcept -> const_pointer {
	return reinterpret_cast<const_pointer> (
	    &static_cast<const SmallVector<T, small_vector_minimum_inline_size> *> (this)
	         ->inline_storage_);
}
}
