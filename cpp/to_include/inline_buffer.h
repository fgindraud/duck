#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

// A ref to a non resizable array
template <typename T, typename SizeType> class ArrayRef {
public:
	BufferRef (SizeType size, T * bufp) : size_ (size), buffer_ (bufp) {}

	SizeType size () const { return size_; }
	T & operator[] (SizeType index) { return buffer_[index]; }
	// TODO add at(), iterators, ...

private:
	const SizeType size_;
	T * buffer_; // TODO not_null
};

namespace Impl {
template <typename T, typename SizeType = std::size_t, typename Tag = void,
          typename UseSizeFieldOfTag = void>
struct InlineArrayElement {
	SizeType size_;
};
}

template <typename T, typename SizeType = std::size_t> class InlineBuffer final {

public:
	T & operator[] (SizeType i) { return buffer_[i]; }
	SizeType size () const { return size_; }

private:
	const SizeType size_;
	T buffer_[];
};

template <typename T, typename SizeType = std::size_t>
InlineBuffer<T, SizeType> * make_inline_buffer (SizeType n) {
	return nullptr;
}
