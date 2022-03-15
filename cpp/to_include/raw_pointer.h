#pragma once

#include <duck/integer.h>

#include <cassert>
#include <cstddef> // size_t std::nullptr_t
#include <cstdint> // uintN_t uintptr_t

struct Ptr {
	/* Raw pointer like object ; allows cleaner raw pointer manipulation
	 */
	std::uintptr_t p;

	Ptr () = default; // Unitialized
	Ptr (std::nullptr_t) : p (0) {}
	explicit Ptr (std::uintptr_t ptr) : p (ptr) {}
	Ptr (const void * ptr) : p (reinterpret_cast<std::uintptr_t> (ptr)) {}

	template <typename T> T as (void) const { return reinterpret_cast<T> (p); }
	template <typename T> T & as_ref (void) const {
		assert (p != 0);
		return *as<T *> ();
	}
	operator void * (void) const { return as<void *> (); }

	template <typename T = char> Ptr add (std::size_t off) const {
		return Ptr (p + sizeof (T) * off);
	}
	template <typename T = char> Ptr sub (std::size_t off) const {
		return Ptr (p - sizeof (T) * off);
	}
	std::size_t sub (Ptr ptr) const { return p - ptr.p; }

	Ptr & operator+= (std::size_t off) { return *this = add (off); }
	Ptr operator- (std::size_t off) const { return sub (off); }
	Ptr & operator-= (std::size_t off) { return *this = sub (off); }

	// align : backward ; align_up : forward
	Ptr align_down (std::size_t al) const { return Ptr (Integer::align_down (p, al)); }
	Ptr align_up (std::size_t al) const { return Ptr (Integer::align_up (p, al)); }
	bool is_aligned (std::size_t al) const { return p % al == 0; }

	// Compute diff
	std::size_t operator- (Ptr other) const { return p - other.p; }
};

inline Ptr operator+ (Ptr lhs, std::size_t off) {
	return lhs.add (off);
}
inline Ptr operator+ (std::size_t off, Ptr rhs) {
	return rhs.add (off);
}

inline bool operator< (Ptr lhs, Ptr rhs) {
	return lhs.p < rhs.p;
}
inline bool operator> (Ptr lhs, Ptr rhs) {
	return lhs.p > rhs.p;
}
inline bool operator<= (Ptr lhs, Ptr rhs) {
	return lhs.p <= rhs.p;
}
inline bool operator>= (Ptr lhs, Ptr rhs) {
	return lhs.p >= rhs.p;
}
inline bool operator== (Ptr lhs, Ptr rhs) {
	return lhs.p == rhs.p;
}
inline bool operator!= (Ptr lhs, Ptr rhs) {
	return lhs.p != rhs.p;
}
