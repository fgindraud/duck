#pragma once

// Variant classes

#include <cstdint>
#include <duck/type_traits.h>
#include <duck/utility.h>
#include <typeindex>
#include <utility>

namespace duck {
namespace Variant {

	namespace Detail {
		// Get nth type in a pack
		template <int n, typename... Args> struct GetNthType;
		template <typename First, typename... Others> struct GetNthType<0, First, Others...> {
			using Type = First;
		};
		template <int n, typename First, typename... Others> struct GetNthType<n, First, Others...> {
			using Type = typename GetNthType<n - 1, Others...>::Type;
		};

		// Get index of a type in a pack
		template <typename T, typename... Args> struct GetTypeId;
		template <typename T, typename... Others> struct GetTypeId<T, T, Others...> {
			enum { value = 0 };
		};
		template <typename T, typename First, typename... Others>
		struct GetTypeId<T, First, Others...> {
			enum { value = GetTypeId<T, Others...>::value + 1 };
		};

		// Wrap common functions
		using WrappedMethod = void (*) (void *);
		template <typename T> inline void wrap_destructor (void * p) { static_cast<T *> (p)->~T (); }
	}

	template <typename... Types> class StaticList {
		// Variant for a static list of types
	public:
		static constexpr std::size_t alignment = max (alignof (Types)...);
		static constexpr std::size_t size = max (sizeof (Types)...);

		using TypeTag = std::uint8_t; // TODO use bounded int
		template <int type_id> using TypeForId = typename Detail::GetNthType<type_id, Types...>::Type;
		template <typename T> static constexpr TypeTag id_for_type () {
			return TypeTag (Detail::GetTypeId<typename std::decay<T>::type, Types...>::value);
		}

		template <typename T, typename = typename std::enable_if<Traits::NonSelf<T, StaticList>::value>>
		explicit StaticList (T && t) : type_ (id_for_type<T> ()) {
			new (&storage_) typename std::decay<T>::type (std::forward<T> (t));
		}

		constexpr TypeTag type () const noexcept { return type_; }

		// TODO support for destructor table, template access, ...

	private:
		typename std::aligned_storage<size, alignment>::type storage_;
		TypeTag type_;
	};

	template <std::size_t len, std::size_t align> class Dynamic {
		// Variant with a fixed size, but no type restriction as long as it fits
		// TODO improve
	public:
		template <typename T, typename = typename std::enable_if<Traits::NonSelf<T, Dynamic>::value>>
		explicit Dynamic (T && t) : destructor_ (Detail::wrap_destructor<T>) {
			static_assert (sizeof (T) <= len, "T must fit in reserved size");
			static_assert (alignof (T) <= align, "T align requirement must fit in reserved storage");
			new (&storage_) T (std::forward<T> (t));
		}

		template <typename T> T & as () {
			if (&Detail::wrap_destructor<T> != destructor_)
				throw std::bad_cast{};
			return reinterpret_cast<T &> (storage_);
		}
		template <typename T> const T & as () const {
			if (&Detail::wrap_destructor<T> != destructor_)
				throw std::bad_cast{};
			return reinterpret_cast<const T &> (storage_);
		}

		Dynamic (const Dynamic &) = delete;
		Dynamic (Dynamic &&) = delete;
		Dynamic & operator= (const Dynamic &) = delete;
		Dynamic & operator= (Dynamic &&) = delete;

		~Dynamic () { destructor_ (&storage_); }

	private:
		typename std::aligned_storage<len, align>::type storage_;
		Detail::WrappedMethod destructor_;
	};

	// TODO variant storing in place a Derived class ? Problem of finding the ptr to base...
}
}
