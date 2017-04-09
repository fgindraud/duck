#pragma once

// Iterator builder classes

// TODO Partially finished
// Lacking element access
// Other problem, traits definition...

#include <iterator>
#include <type_traits>
#include <utility>

namespace duck {
namespace Iterator {

	namespace Detail {
		template <typename T, typename Self> constexpr bool non_self () {
			using DecayedT = typename std::decay<T>::type;
			return !std::is_same<Self, DecayedT>::value && !std::is_base_of<Self, DecayedT>::value;
		}
	}

	template <typename Impl> class Facade : public Impl {
		// Iterator facade.
		//
		// Requirements on Impl by iterator category:
		// - output: Impl::next(), Impl::deref()
		// - input: + Impl::equal(it)
		// - forward: + Impl::Impl(const Impl&) [copy]
		// - bidirectional: + Impl::prev()
		// - random access: + Impl::advance(n), Impl::distance (it)
		//
		// TODO gen next/prev if advance, equal if distance ?
	public:
		// TODO generate typedefs (use sfinae):
		// - typedef to Impl::thing if it exists
		// - or try extracting it from methods
		// - or default...
		using value_type = typename Impl::value_type;
		using difference_type = typename Impl::difference_type;
		using reference = typename Impl::reference;
		using pointer = typename Impl::pointer;
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
