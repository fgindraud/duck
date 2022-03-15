#include <cstddef>
#include <new>
#include <type_traits>

// Pointers
struct RawRef {
	void * p{nullptr};
	void * get () const noexcept { return p; }
};
template <typename T> struct Ref : RawRef {
	operator Ref<const T> () const noexcept { return {p}; }
	T * get () const noexcept { return static_cast<T *> (p); }
	T & operator* () const noexcept { return *get (); }
	T * operator-> () const noexcept { return get (); }
};

template <typename T> struct UniqueRef : Ref<T> {
	UniqueRef (const UniqueRef &) = delete;
	UniqueRef & operator= (const UniqueRef &) = delete;
	UniqueRef (UniqueRef && other) {
		this->p = other.p;
		other.p = nullptr;
	}
	UniqueRef & operator= (UniqueRef && other) {
		reset ();
		this->p = other.p;
		other.p = nullptr;
	}
	~UniqueRef () { reset (); }

	void reset () {
		if (this->get () != nullptr) {
			delete this->get ();
			this->p = nullptr;
		}
	}

	// Internal use only..
	explicit UniqueRef (void * from_raw_pointer) { this->p = from_raw_pointer; }
	void * release () && {
		void * p = this->p;
		this->p = nullptr;
		return p;
	}
};

template <std::size_t S, std::size_t A> UniqueRef<std::aligned_storage_t<S, A>> allocate () {
	void * p;
	if constexpr (A > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
		p = operator new (S, std::align_val_t (A));
	} else {
		p = operator new (S);
	}
	return UniqueRef<std::aligned_storage_t<S, A>> (p);
}

template <typename T> using AlignedStorageFor = std::aligned_storage_t<sizeof (T), alignof (T)>;

template <typename T> UniqueRef<AlignedStorageFor<T>> allocate_memory_for () {
	return allocate<sizeof (T), alignof (T)> ();
}

template <typename T, typename... Args>
UniqueRef<T> construct (UniqueRef<AlignedStorageFor<T>> && memory, Args &&... args) {
	return UniqueRef<T> (new (std::move (memory).release ()) T (std::forward<Args> (args)...));
}

template <typename T> UniqueRef<AlignedStorageFor<T>> destroy (UniqueRef<T> && object) {
	object->~T ();
	return UniqueRef<AlignedStorageFor<T>> (std::move (object).release ());
}

int main () {
	auto p = allocate_memory_for<int> ();
	auto i = construct<int> (std::move (p), 42);
	*i = -42;
	auto p2 = destroy (std::move (i));
	return 0;
}

// Aggregate<...> & related defs
template <typename... Elements> struct Aggregate;

template <typename T> struct IsAggregate : std::false_type {};
template <typename... Elements> struct IsAggregate<Aggregate<Elements...>> : std::true_type {};

template <typename... Elements> struct Ref<Aggregate<Elements...>> : RawRef {};

template <std::size_t I, typename S> struct AggregateElementImpl;
template <std::size_t I, typename S>
using AggregateElement = typename AggregateElementImpl<I, S>::Type;
template <typename Head, typename... Tail>
struct AggregateElementImpl<0, Ref<Aggregate<Head, Tail...>>> {
	using Type = Ref<Head>;
};
template <std::size_t I, typename Head, typename... Tail>
struct AggregateElementImpl<I, Ref<Aggregate<Head, Tail...>>>
    : AggregateElementImpl<I - 1, Ref<Aggregate<Tail...>>> {};

template <std::size_t I, typename... Elements>
AggregateElement<I, Ref<Aggregate<Elements...>>> get (Ref<Aggregate<Elements...>> r) {}
template <std::size_t I, typename... Elements> void get (Ref<const Aggregate<Elements...>> r);

// TypeInfo
template <typename T> struct TypeInfo {
	static_assert (!IsAggregate<T>::value, "TypeInfo must be specialised for Aggregate<Elements...>");

	static constexpr std::size_t size () noexcept { return sizeof (T); }
	static constexpr std::size_t alignment () noexcept { return alignof (T); }
};
