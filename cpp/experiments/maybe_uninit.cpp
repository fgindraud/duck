#include <cassert>
#include <new>         // launder
#include <type_traits> // aligned_storage

template <typename T> class MaybeUninit {
public:
	// Created as empty, should be empty when destroyed.
	MaybeUninit () = default;
	~MaybeUninit () = default;

	MaybeUninit (const MaybeUninit &) = delete;
	MaybeUninit & operator= (const MaybeUninit &) = delete;
	MaybeUninit (MaybeUninit &&) = delete;
	MaybeUninit & operator= (MaybeUninit &&) = delete;

	template <typename... Args> void construct (Args &&... args) {
		new (&storage_) T (std::forward<Args> (args)...);
	}

	T & assume_init () { return *std::launder (reinterpret_cast<T *> (&storage_)); }
	const T & assume_init () const { return *std::launder (reinterpret_cast<const T *> (&storage_)); }

	void destroy () { assume_init ().~T (); }

private:
	std::aligned_storage_t<sizeof (T), alignof (T)> storage_;
};

template <typename T> class Place {
public:
	Place (MaybeUninit<T> & location) : location_ (location) {}
	~Place () { assert (built_); }

	template <typename... Args> void construct (Args &&... args) {
		assert (!built_);
		location_.construct (std::forward<Args> (args)...);
		built_ = true;
	}

private:
	MaybeUninit<T> & location_;
	bool built_ = false;
};

template <typename T, std::size_t N> class Array {
public:
	Array () = default;

	~Array () {
		while (size () > 0) {
			pop ();
		}
	}

	std::size_t size () const { return size_; }

	[[nodiscard]] Place<T> push () {
		assert (size_ < N);
		MaybeUninit<T> & uninit = storage_[size_];
		size_ += 1;
		return {uninit};
	}
	void pop () {
		assert (size_ > 0);
		size_ -= 1;
		storage_[size_].destroy ();
	}

	T & operator[] (std::size_t i) {
		assert (i < size_);
		return storage_[i].assume_init ();
	}

private:
	MaybeUninit<T> storage_[N];
	std::size_t size_{0};
};

int main () {
	Array<int, 10> a;
	a.push ().construct (42);

	return 0;
}