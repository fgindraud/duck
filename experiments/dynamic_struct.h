#pragma once

#include <cstddef>
#include <type_traits>

// Pointers
struct RawRef {
	void * p;
	void * get () const noexcept { return p; }
};
template <typename T> struct Ref : RawRef {
	operator Ref<const T> () const noexcept { return {p}; }
	T * get () const noexcept { return static_cast<T *> (p); }
	T & operator* () const noexcept { return *get (); }
	T * operator-> () const noexcept { return get (); }
};
template <typename T> struct Span; // TODO

// Struct<...> & related defs
template <typename... Elements> struct Struct;

template <typename T> struct IsStruct : std::false_type {};
template <typename... Elements> struct IsStruct<Struct<Elements...>> : std::true_type {};

template <typename... Elements> struct Ref<Struct<Elements...>> : RawRef {};

template <std::size_t I, typename S> struct StructElementImpl;
template <std::size_t I, typename S> using StructElement = typename StructElementImpl<I, S>::Type;
template <typename Head, typename... Tail> struct StructElementImpl<0, Ref<Struct<Head, Tail...>>> {
	using Type = Ref<Head>;
};
template <std::size_t I, typename Head, typename... Tail>
struct StructElementImpl<I, Ref<Struct<Head, Tail...>>>
    : StructElementImpl<I - 1, Ref<Struct<Tail...>>> {};

template <std::size_t I, typename... Elements>
StructElement<I, Ref<Struct<Elements...>>> get (Ref<Struct<Elements...>> r) {
	
}
template <std::size_t I, typename... Elements> void get (Ref<const Struct<Elements...>> r);

// TypeInfo
template <typename T> struct TypeInfo {
	static_assert (!IsStruct<T>::value, "TypeInfo must be specialised for Struct<Elements...>");

	static constexpr std::size_t size () noexcept { return sizeof (T); }
	static constexpr std::size_t alignment () noexcept { return alignof (T); }
};
