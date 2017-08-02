#pragma once

// Variant classes
// Static takes a static list of types.
// Dynamic takes a size/alignement and supports any fitting type.
// SmallUniqPtr (in a separate header)
// STATUS: WIP

#include <cstdint>
#include <duck/type_operations.h>
#include <duck/type_traits.h>
#include <duck/utility.h>
#include <limits>
#include <typeindex>
#include <utility>

namespace duck {
namespace Variant {

	namespace Detail {
		// Get nth type in a pack (SFINAE fails if bad index)
		template <int n, typename... Args> struct GetNthType;
		template <typename First, typename... Others> struct GetNthType<0, First, Others...> {
			using Type = First;
		};
		template <int n, typename First, typename... Others> struct GetNthType<n, First, Others...> {
			using Type = typename GetNthType<n - 1, Others...>::Type;
		};

		// Get index of a type in a pack (SFINAE fails if not found)
		template <typename T, typename... Args> struct GetTypeId;
		template <typename T, typename... Others> struct GetTypeId<T, T, Others...> {
			enum { value = 0 };
		};
		template <typename T, typename First, typename... Others>
		struct GetTypeId<T, First, Others...> {
			enum { value = GetTypeId<T, Others...>::value + 1 };
		};
	}

	template <typename... Types> class Static {
		// Variant for a static list of types
	private:
		static constexpr auto alignment = max (alignof (Types)...);
		static constexpr auto size = max (sizeof (Types)...);
		static constexpr int invalid_index = -1;
		// TODO move to invalid==0 and shift indexes
		// So a default noop operations table can be used to handle valueless cases

	public:
		template <int index> using TypeForIndex = typename Detail::GetNthType<index, Types...>::Type;
		template <typename T>
		using IndexForType = Detail::GetTypeId<typename std::decay<T>::type, Types...>;
		template <typename T> static constexpr int index_for_type () { return IndexForType<T>::value; }

		template <typename T, int index = IndexForType<T>::value> explicit Static (T && t) {
			// This constructor is SFINAE restricted to supported types
			new (&storage_) typename std::decay<T>::type (std::forward<T> (t));
			index_ = index_for_type<T> ();
		}
		template <typename T, typename... Args> explicit Static (InPlace<T>, Args &&... args) {
			new (&storage_) T (std::forward<Args> (args)...);
			index_ = index_for_type<T> ();
		}

		constexpr int index () const noexcept { return index_; }

		// TODO support for destructor table, template access, ...

	private:
		int index_{invalid_index};
		typename std::aligned_storage<size, alignment>::type storage_;

		// Generate type ops table TODO
		//static constexpr Type::Operations type_ops[1];
	};

	template <std::size_t len, std::size_t align> class Dynamic {
		// Variant with a fixed size, but no type restriction as long as it fits
		// TODO improve
	public:
		template <typename T, typename = typename std::enable_if<Traits::NonSelf<T, Dynamic>::value>>
		explicit Dynamic (T && t) : destructor_ (Type::destroy<T>) {
			static_assert (sizeof (T) <= len, "T must fit in reserved size");
			static_assert (alignof (T) <= align, "T align requirement must fit in reserved storage");
			new (&storage_) T (std::forward<T> (t));
		}

		template <typename T> T & as () {
			if (&Type::destroy<T> != destructor_)
				throw std::bad_cast{};
			return reinterpret_cast<T &> (storage_);
		}
		template <typename T> const T & as () const {
			if (&Type::destroy<T> != destructor_)
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
		void (*destructor_) (void *);
	};
}
}
