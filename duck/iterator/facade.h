#pragma once

// Iterator builder classes
// STATUS: prototype

#include <duck/iterator/maybe_traits.h>
#include <duck/meta/maybe_type.h>
#include <duck/type_traits.h>
#include <iterator>
#include <utility>

namespace duck {
namespace Iterator {

	namespace Detail {
		// Maybe-type for decltype of ItImpl::deref() return value
		template <typename ItImpl> class MaybeDerefReturnType {
		private:
			template <typename T>
			static auto test (T) -> decltype (std::declval<T> ().deref (), std::true_type{});
			static auto test (...) -> std::false_type;

			template <typename T, bool has_type> struct MaybeTypeImpl {};
			template <typename T> struct MaybeTypeImpl<T, true> {
				using Type = decltype (std::declval<T> ().deref ());
			};

		public:
			enum { value = decltype (test (std::declval<ItImpl> ()))::value };
			using MaybeType = MaybeTypeImpl<ItImpl, value>;
		};

		// Maybe-type for decltype of ItImpl::distance() return value
		template <typename ItImpl> class MaybeDistanceReturnType {
		private:
			template <typename T>
			static auto test (T)
			    -> decltype (std::declval<T> ().distance (std::declval<T> ()), std::true_type{});
			static auto test (...) -> std::false_type;

			template <typename T, bool has_type> struct MaybeTypeImpl {};
			template <typename T> struct MaybeTypeImpl<T, true> {
				using Type = decltype (std::declval<T> ().distance (std::declval<T> ()));
			};

		public:
			enum { value = decltype (test (std::declval<ItImpl> ()))::value };
			using MaybeType = MaybeTypeImpl<ItImpl, value>;
		};

		// Maybe-type for reference of iterator facade
		template <typename ItImpl>
		using MaybeFacadeReferenceType =
		    Maybe::SelectFirstDefined<MaybeReferenceType<ItImpl>, MaybeDerefReturnType<ItImpl>>;

		// Maybe-type for value_type of iterator facade
		template <typename ItImpl>
		using MaybeFacadeValueType =
		    Maybe::SelectFirstDefined<MaybeValueType<ItImpl>,
		                              Maybe::Decay<MaybeFacadeReferenceType<ItImpl>>>;

		// Maybe-type for pointer of iterator facade
		template <typename ItImpl>
		using MaybeFacadePointerType =
		    Maybe::SelectFirstDefined<MaybePointerType<ItImpl>,
		                              Maybe::AddPointer<MaybeFacadeReferenceType<ItImpl>>>;

		// Maybe-type for difference_type of iterator facade
		template <typename ItImpl>
		using MaybeFacadeDifferenceType =
		    Maybe::SelectFirstDefined<MaybeDifferenceType<ItImpl>,
		                              Detail::MaybeDistanceReturnType<ItImpl>,
		                              Maybe::DefinedMaybeType<std::ptrdiff_t>>;
	}

	template <typename Impl> class Facade : public Impl {
		/* Iterator facade.
		 *
		 * Typedefs can be provided by Impl, or deduced from calls:
		 * - reference: return type of Impl::deref()
		 * - value_type: std::decay of reference
		 * - pointer: add_pointer to reference
		 * - difference_type: return type of std distance, or std::ptrdiff_t
		 *
		 * Requirements on Impl by iterator category:
		 * - output: Impl::next(), Impl::deref()
		 * - input: + Impl::equal(it)
		 * - forward: + Impl::Impl(const Impl&) [copy]
		 * - bidirectional: + Impl::prev()
		 * - random access: + Impl::advance(n), Impl::distance (it)
		 *
		 * Impl methods must be accessible from the facade.
		 * They can be protected, but only if ALL type typedefs are provided by Impl.
		 * If some of them are not provided, methods must be public.
		 * (internally, the typedef deduction requires public access to methods).
		 *
		 * TODO simplify wrapping of a sub-iterator
		 */
	public:
		using value_type = Maybe::Unpack<Detail::MaybeFacadeValueType<Impl>>;
		using difference_type = Maybe::Unpack<Detail::MaybeFacadeDifferenceType<Impl>>;
		using reference = Maybe::Unpack<Detail::MaybeFacadeReferenceType<Impl>>;
		using pointer = Maybe::Unpack<Detail::MaybeFacadePointerType<Impl>>;
		// TODO what for category !

		// Constructors: defaulted, will be enabled if the matching one is available in Impl
		Facade () = default;
		Facade (const Facade &) = default;
		Facade (Facade &&) = default;
		Facade & operator= (const Facade &) = default;
		Facade & operator= (Facade &&) = default;
		~Facade () = default;

		// Forwarding constructor (special disambiguation for 1 argument case)
		template <typename T, typename = typename std::enable_if<Traits::NonSelf<T, Facade>::value>>
		Facade (T && t) : Impl (std::forward<T> (t)) {}
		template <typename T, typename... Args>
		Facade (T && t, Args &&... args) : Impl (std::forward<T> (t), std::forward<Args> (args)...) {}

		// Input / output

		Facade & operator++ () {
			Impl::next ();
			return *this;
		}

		reference operator* () { return Impl::deref (); }
		pointer operator-> () { return &(*(*this)); }

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
