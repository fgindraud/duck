#include <array>
#include <memory>
#include <stdexcept>

// View of an N-d array.
template <typename T, std::size_t N> class ArrayView {
	static_assert (N > 0, "N non 0");

public:
	using Reference = ArrayView<T, N - 1>;

	ArrayView (T * base, const std::size_t * dimensions, const std::size_t * offsets) noexcept
	    : base_ (base), dimensions_ (dimensions), offsets_ (offsets) {}

	operator ArrayView<const T, N> () const noexcept { return {base_, dimensions_, offsets_}; }

	std::size_t size () const noexcept { return dimensions_[0]; }

	Reference operator[] (std::size_t i) const noexcept {
		return {base_ + i * offsets_[0], dimensions_ + 1, offsets_ + 1};
	}
	Reference at (std::size_t i) const {
		if (!(i < size ())) {
			throw std::out_of_range ("ArrayView::at");
		}
		return operator[] (i);
	}

private:
	T * base_;
	const std::size_t * offsets_;    // N offsets array
	const std::size_t * dimensions_; // N dimensions array
};
template <typename T> class ArrayView<T, 1> {
	// For N=1, behave as a simple span, and return references when indexed
public:
	using Reference = T &;

	ArrayView (T * base, const std::size_t * dimensions, const std::size_t *) noexcept
	    : ArrayView (base, dimensions[0]) {}

	operator ArrayView<const T, 1> () const noexcept { return {base_, size_}; }

	std::size_t size () const { return size_; }

	Reference operator[] (std::size_t i) const noexcept { return base_[i]; }
	Reference at (std::size_t i) const {
		if (!(i < size ())) {
			throw std::out_of_range ("ArrayView::at");
		}
		return operator[] (i);
	}

	// Additional API for simple span
	ArrayView (T * base, std::size_t size) noexcept : base_ (base), size_ (size) {}

	T * data () const noexcept { return base_; }

private:
	T * base_;
	std::size_t size_;
};

/* Multidimensional array with N dimensions of dynamic size.
 * Non resizable after creation.
 *
 * The array is linearized. Layout:
 * N=1: [---size0---]
 * N=2: [[---size1---]---size0---[---size1---]]
 * And so on. General case:
 * For N > 1, the linearized array is the sequence of linearized arrays of dimension N-1.
 * Each sub-array layout is a N-1 dimension arrays of sizes (size1, ..., sizeN-1).
 */
template <typename T, std::size_t N> class Array {
	static_assert (N > 0, "N non 0");

public:
	Array (const std::array<std::size_t, N> & dimensions) : dimensions_ (dimensions) {
		std::size_t offset = 1;
		for (std::size_t k = N; k > 0; --k) {
			offsets_[k - 1] = offset;
			offset *= dimensions_[k - 1];
		}
		linear_size_ = offset;
		array_.reset (new T[linear_size_]);
	}

	// Sizes of each dimension
	static constexpr std::size_t nb_dimensions () noexcept { return N; }
	const std::array<std::size_t, N> & sizes () const noexcept { return dimensions_; }
	std::size_t size (std::size_t i) const noexcept { return dimensions_[i]; }

	// Total size and access to linearized buffer
	std::size_t linear_size () const noexcept { return linear_size_; }
	T * linear_data () noexcept { return array_.get (); }
	const T * linear_data () const noexcept { return array_.get (); }

	// Generate a view struct for the array.
	ArrayView<T, N> view () noexcept {
		return {array_.get (), dimensions_.data (), offsets_.data ()};
	}
	ArrayView<const T, N> view () const noexcept {
		return {array_.get (), dimensions_.data (), offsets_.data ()};
	}

	// Indexing for first dimension
	typename ArrayView<T, N>::Reference operator[] (std::size_t i) noexcept { return view ()[i]; }
	typename ArrayView<const T, N>::Reference operator[] (std::size_t i) const noexcept {
		return view ()[i];
	}

private:
	std::unique_ptr<T> array_;
	std::size_t linear_size_;               // Total size (in T objects) of array
	std::array<std::size_t, N> dimensions_; // Size of dimension k
	std::array<std::size_t, N> offsets_;    // Offset of elements of dimension k (in T objects)
};

#include <iostream>

int main () {
	Array<double, 3> a ({2, 3, 4});

	for (std::size_t i0 = 0; i0 < a.size (0); ++i0) {
		for (std::size_t i1 = 0; i1 < a.size (1); ++i1) {
			for (std::size_t i2 = 0; i2 < a.size (2); ++i2) {
				a[i0][i1][i2] = i0 + i1 + i2;
			}
		}
	}

	std::cout << '{';
	for (std::size_t i = 0; i < a.linear_size (); ++i) {
		std::cout << a.linear_data ()[i] << ' ';
	}
	std::cout << "}\n";

	std::cout << "{\n";
	for (std::size_t i0 = 0; i0 < a.size (0); ++i0) {
		std::cout << "\t{\n";
		for (std::size_t i1 = 0; i1 < a.size (1); ++i1) {
			std::cout << "\t\t{";
			for (std::size_t i2 = 0; i2 < a.size (2); ++i2) {
				std::cout << a[i0][i1][i2] << ' ';
			}
			std::cout << "}\n";
		}
		std::cout << "\t}\n";
	}
	std::cout << "}\n";

	return 0;
}
