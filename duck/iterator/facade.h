#pragma once

// Iterator builder classes

// TODO Partially finished
// Lacking element access
// Other problem, traits definition...

#include <duck/iterator/traits.h>
#include <iterator>
#include <type_traits>
#include <utility>

namespace duck {
namespace Iterator {

	namespace Detail {
		// Test to not override the moe/copy constructors in a universal ref overload.
		template <typename T, typename Self> constexpr bool non_self () {
			using DecayedT = typename std::decay<T>::type;
			return !std::is_same<Self, DecayedT>::value && !std::is_base_of<Self, DecayedT>::value;
		}
		
		// Test if distance method exists
		template <typename ItImpl> class HasDistanceMethod {
		private:
			template <typename T>
			static constexpr decltype (std::declval<T> ().distance (std::declval<T> ()), bool())
			test (int) {
				return true;
			}
			template <typename T> static constexpr bool test (...) { return false; }

		public:
			enum { value = test<ItImpl> (int()) };
		};

		// Trait to determine the reference typedef of iterator
		template <typename ItImpl, bool has_reference_type> struct GetFacadeReferenceTypeImpl {};
		template <typename ItImpl> struct GetFacadeReferenceTypeImpl<ItImpl, true> {
			using Type = typename ItImpl::reference;
		};
		template <typename ItImpl> struct GetFacadeReferenceTypeImpl<ItImpl, false> {
			using Type = decltype (std::declval<ItImpl> ().deref ());
		};
		template <typename ItImpl>
		using GetFacadeReferenceType =
		    typename GetFacadeReferenceTypeImpl<ItImpl, HasReferenceType<ItImpl>::value>::Type;

		// Trait to determine the value_type typedef of iterator
		template <typename ItImpl, bool has_value_type> struct GetFacadeValueTypeImpl {};
		template <typename ItImpl> struct GetFacadeValueTypeImpl<ItImpl, true> {
			using Type = typename ItImpl::value_type;
		};
		template <typename ItImpl> struct GetFacadeValueTypeImpl<ItImpl, false> {
			using Type = typename std::decay<GetFacadeReferenceType<ItImpl>>::type;
		};
		template <typename ItImpl>
		using GetFacadeValueType =
		    typename GetFacadeValueTypeImpl<ItImpl, HasValueType<ItImpl>::value>::Type;

		// Trait to determine the pointer typedef of iterator
		template <typename ItImpl, bool has_pointer_type> struct GetFacadePointerTypeImpl {};
		template <typename ItImpl> struct GetFacadePointerTypeImpl<ItImpl, true> {
			using Type = typename ItImpl::pointer;
		};
		template <typename ItImpl> struct GetFacadePointerTypeImpl<ItImpl, false> {
			using Type = typename std::add_pointer<GetFacadeReferenceType<ItImpl>>::type;
		};
		template <typename ItImpl>
		using GetFacadePointerType =
		    typename GetFacadePointerTypeImpl<ItImpl, HasPointerType<ItImpl>::value>::Type;

		// Trait to determine the difference_type typedef of iterator
		template <typename ItImpl, bool has_difference_type, bool has_distance_method>
		struct GetFacadeDifferenceTypeImpl {};
		template <typename ItImpl, bool has_distance_method>
		struct GetFacadeDifferenceTypeImpl<ItImpl, true, has_distance_method> {
			using Type = typename ItImpl::difference_type;
		};
		template <typename ItImpl> struct GetFacadeDifferenceTypeImpl<ItImpl, false, true> {
			using Type = decltype (std::declval<ItImpl> ().distance (std::declval<ItImpl> ()));
		};
		template <typename ItImpl> struct GetFacadeDifferenceTypeImpl<ItImpl, false, false> {
			using Type = std::ptrdiff_t;
		};
		template <typename ItImpl>
		using GetFacadeDifferenceType =
		    typename GetFacadeDifferenceTypeImpl<ItImpl, HasDifferenceType<ItImpl>::value,
		                                         HasDistanceMethod<ItImpl>::value>::Type;
	}

	template <typename Impl> class Facade : public Impl {
		// Iterator facade.
		//
		// Typedefs can be provided by Impl, or deduced from calls:
		// - reference: return type of Impl::deref()
		// - value_type: std::decay of reference
		// - pointer: add_pointer to reference
		// - difference_type: return type of std distance, or std::ptrdiff_t
		//
		// Requirements on Impl by iterator category:
		// - output: Impl::next(), Impl::deref()
		// - input: + Impl::equal(it)
		// - forward: + Impl::Impl(const Impl&) [copy]
		// - bidirectional: + Impl::prev()
		// - random access: + Impl::advance(n), Impl::distance (it)
		//
		// TODO gen next/prev if advance, equal if distance ?
		// TODO simplify wrapping of a sub-iterator
	public:
		using value_type = Detail::GetFacadeValueType<Impl>;
		using difference_type = Detail::GetFacadeDifferenceType<Impl>;
		using reference = Detail::GetFacadeReferenceType<Impl>;
		using pointer = Detail::GetFacadePointerType<Impl>;
		// TODO what for category !

		// Constructors: defaulted, will be enabled if the matching one is available in Impl
		Facade () = default;
		Facade (const Facade &) = default;
		Facade (Facade &&) = default;
		Facade & operator= (const Facade &) = default;
		Facade & operator= (Facade &&) = default;
		~Facade () = default;

		// Forwarding constructor (special disambiguation for 1 argument case)
		template <typename T, typename = typename std::enable_if<Detail::non_self<T, Facade> ()>>
		Facade (T && t) : Impl (std::forward<T> (t)) {}
		template <typename T, typename... Args>
		Facade (T && t, Args &&... args) : Impl (std::forward<T> (t), std::forward<Args> (args)...) {}

		// Input / output

		Facade & operator++ () {
			Impl::next ();
			return *this;
		}

		reference operator* () const { return Impl::deref (); }
		pointer operator-> () const { return &(*(*this)); }

		bool operator== (const Facade & other) const { return Impl::equal (other); }
		bool operator!= (const Facade & other) const { return !(*this == other); }

		// Forward

		Facade operator++ (int) {
			Facade tmp (*this);
			++*this;
			return tmp;
		}

		// Bidir

		Facade & operator-- () {
			Impl::prev ();
			return *this;
		}
		Facade operator-- (int) {
			Facade tmp (*this);
			--*this;
			return tmp;
		}

		// Random access

		Facade & operator+= (difference_type n) {
			Impl::advance (n);
			return *this;
		}
		Facade operator+ (difference_type n) const {
			Facade tmp (*this);
			tmp += n;
			return tmp;
		}
		friend Facade operator+ (difference_type n, const Facade & it) { return it + n; }

		Facade & operator-= (difference_type n) { return *this += (-n); }
		Facade operator- (difference_type n) const { return *this + (-n); }

		difference_type operator- (const Facade & other) const { return Impl::distance (other); }

		reference operator[] (difference_type n) const { return *(*this + n); }

		bool operator< (const Facade & other) const { return (*this - other) < 0; }
		bool operator> (const Facade & other) const { return (*this - other) > 0; }
		bool operator<= (const Facade & other) const { return (*this - other) <= 0; }
		bool operator>= (const Facade & other) const { return (*this - other) >= 0; }
	};
}
}
