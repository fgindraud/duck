#include <iterator>
#include <type_traits>
#include <utility>

#include <iostream>
#include <string>
#include <vector>

namespace View {

template <typename Int> class IntIterator {
	// immutable random iterator on Z
public:
	using iterator_category = std::random_access_iterator_tag;
	using value_type = Int;
	using difference_type = std::ptrdiff_t;
	using pointer = const value_type *;
	using reference = const value_type &;

	// iterator
	IntIterator () noexcept = default;
	IntIterator (Int n) noexcept : n_ (n) {}
	// + default ctors
	void swap (IntIterator & it) noexcept { std::swap (n_, it.n_); }
	reference operator* () const noexcept { return n_; }
	IntIterator & operator++ () noexcept {
		++n_;
		return *this;
	}

	// input/forward iterator
	bool operator== (const IntIterator & rhs) const noexcept { return n_ == rhs.n_; }
	bool operator!= (const IntIterator & rhs) const noexcept { return n_ != rhs.n_; }
	pointer operator-> () const noexcept { return std::addressof (n_); }
	IntIterator operator++ (int) noexcept {
		IntIterator tmp (*this);
		++*this;
		return tmp;
	}

	// bidirectional iterator
	IntIterator & operator-- () noexcept {
		--n_;
		return *this;
	}
	IntIterator operator-- (int) noexcept {
		IntIterator tmp (*this);
		--*this;
		return tmp;
	}

	// random access iterator
	template <typename T> IntIterator & operator+= (T && t) noexcept {
		return *this = *this + std::forward<T> (t);
	}
	template <typename T> IntIterator operator+ (T && t) const noexcept {
		return IntIterator (n_ + std::forward<T> (t));
	}
	template <typename T> IntIterator & operator-= (T && t) noexcept {
		return *this = *this - std::forward<T> (t);
	}
	template <typename T> IntIterator operator- (T && t) const noexcept {
		return IntIterator (n_ - std::forward<T> (t));
	}
	difference_type operator- (IntIterator it) const noexcept { return n_ - it.n_; }
	reference operator[] (std::size_t n) const noexcept { return *(*this + n); }
	bool operator< (const IntIterator & rhs) const noexcept { return n_ < rhs.n_; }
	bool operator> (const IntIterator & rhs) const noexcept { return n_ > rhs.n_; }
	bool operator<= (const IntIterator & rhs) const noexcept { return n_ <= rhs.n_; }
	bool operator>= (const IntIterator & rhs) const noexcept { return n_ >= rhs.n_; }

private:
	Int n_{};
};

// Out of class functions for IntIterator
template <typename Int> inline void swap (IntIterator<Int> & lhs, IntIterator<Int> & rhs) {
	lhs.swap (rhs);
}
template <typename Int, typename T>
inline IntIterator<Int> operator+ (T && t, IntIterator<Int> it) {
	return it + std::forward<T> (t);
}

template <typename Iterator> class Range {
	// Range is not mutable: we can only build new ranges from it.
	// Methods are
private:
	using Traits = typename std::iterator_traits<Iterator>;

public:
	using ValueType = typename Traits::value_type;
	using ReferenceType = typename Traits::reference;
	using DifferenceType = typename Traits::difference_type;

	constexpr Range (Iterator begin, Iterator end) noexcept : begin_ (begin), end_ (end) {}

	// Basic access
	constexpr Iterator begin () const noexcept { return begin_; }
	constexpr Iterator end () const noexcept { return end_; }

	// Input iterator
	bool empty () const noexcept { return begin_ == end_; }
	ReferenceType front () const noexcept { return *begin_; }
	Range pop_front () const noexcept { return {std::next (begin_), end_}; }
	// FIXME ? ReferenceType back () const noexcept { return *end_; }

	// Bidirectional iterator
	Range pop_back () const noexcept { return {begin_, std::prev (end_)}; }

	// Random access iterator
	DifferenceType size () const noexcept { return std::distance (begin_, end_); }
	Range pop_front (DifferenceType n) const noexcept { return {std::advance (begin_, n), end_}; }
	Range pop_back (DifferenceType n) const noexcept { return {begin_, std::advance (end_, -n)}; }
	ReferenceType operator[] (std::size_t n) const noexcept { return begin_[n]; }
	bool contains (Iterator it) const noexcept { return begin_ <= it && it < end_; }
	DifferenceType offset_of (Iterator it) const noexcept { return std::distance (begin_, it); }

	// "nicer" api (python like slice ; but at(size ()) return end ())
	Iterator at (DifferenceType n) const noexcept {
		auto index = n < 0 ? n + size () : n;
		return std::advance (begin_, index);
	}
	Range slice (DifferenceType from, DifferenceType to) const noexcept {
		return {at (from), at (to)};
	}

	template <typename Container> Container to_container () const { return Container (begin_, end_); }

private:
	Iterator begin_;
	Iterator end_;
};

// Factory functions for containers (enabled if Container defines the proper iterators)
template <typename Container, typename Iterator = typename Container::iterator>
Range<Iterator> range (Container & container) {
	return {container.begin (), container.end ()};
}
template <typename Container, typename Iterator = typename Container::const_iterator>
Range<Iterator> range (const Container & container) {
	return {container.begin (), container.end ()};
}

// Factory functions for integral types.
template <typename Int, typename = std::enable_if_t<std::is_integral<Int>::value>>
Range<IntIterator<Int>> range (Int from, Int to) {
	return {from, to};
}
template <typename Int, typename = std::enable_if_t<std::is_integral<Int>::value>>
Range<IntIterator<Int>> range (Int to) {
	return {Int (0), to};
}
}

template <typename T> std::ostream & operator<< (std::ostream & os, const View::Range<T> & r) {
	os << "Range(";
	for (const auto & e : r)
		os << e << ",";
	return os << ")";
}

template <typename T> class show_type;

int main () {
	const auto a = std::string ("hello world");
	auto b = View::range (a);
	auto c = b.to_container<std::vector<char>> ();
	// show_type<decltype (b)>{};
	std::cout << a << std::endl;
	std::cout << b << std::endl;
	std::cout << View::range (c) << std::endl;
	return 0;
}
