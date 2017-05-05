#pragma once

// A pointer with lower bits used as generic storage.
// The pointer itself must be aligned to guarantee lower bits are unused.

#include <cassert>
#include <cstdint>

namespace duck {

template <typename PtrType, std::size_t N> class TaggedPtr {
public:
	static constexpr std::size_t required_alignement = std::size_t (1) << N;

	constexpr TaggedPtr () noexcept = default;
	TaggedPtr (PtrType ptr) noexcept { set_ptr (ptr); }

	constexpr PtrType get_ptr () const noexcept {
		return reinterpret_cast<PtrType> (ptr_ & ptr_bits_mask);
	}
	void set_ptr (PtrType ptr) noexcept {
		auto repr = reinterpret_cast<Repr> (ptr);
		assert ((repr & tag_bits_mask) == 0); // Is sufficiently aligned ?
		ptr_ = (repr & ptr_bits_mask) | (ptr_ & tag_bits_mask);
	}

	bool get_bit (std::size_t index) const noexcept {
		assert (index < N);
		return bool(ptr_ & exact_bit_mask (index));
	}
	void set_bit (std::size_t index, bool value) noexcept {
		assert (index < N);
		auto all_other_bits_mask = ~exact_bit_mask (index);
		auto index_bit_mask = Repr (value) << index;
		ptr_ = (ptr_ & all_other_bits_mask) | index_bit_mask;
	}
	template <std::size_t index> bool get_bit () const noexcept {
		static_assert (index < N, "Index < N");
		return get_bit (index);
	}
	template <std::size_t index> void set_bit (bool value) noexcept {
		static_assert (index < N, "Index < N");
		set_bit (index, value);
	}

	constexpr operator PtrType () const noexcept { return get_ptr (); }
	TaggedPtr & operator= (PtrType ptr) noexcept {
		set_ptr (ptr);
		return *this;
	}

private:
	using Repr = std::uintptr_t;
	static constexpr Repr exact_bit_mask (std::size_t index) noexcept { return Repr (1) << index; }
	static constexpr Repr tag_bits_mask = exact_bit_mask (N) - 1;
	static constexpr Repr ptr_bits_mask = ~tag_bits_mask;
	Repr ptr_{0};
};
}
