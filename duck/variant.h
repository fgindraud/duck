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
		template <typename T, typename... Args> struct GetTypePos;
		template <typename T, typename... Others> struct GetTypePos<T, T, Others...> {
			enum { value = 0 };
		};
		template <typename T, typename First, typename... Others>
		struct GetTypePos<T, First, Others...> {
			enum { value = GetTypePos<T, Others...>::value + 1 };
		};
	}

	template <typename... Types> class Static {
		// Variant for a static list of types
	private:
		static constexpr auto alignment = max (alignof (Types)...);
		static constexpr auto size = max (sizeof (Types)...);
		static_assert (sizeof...(Types) > 0, "Empty type list for Static variant");

		/* The currently stored type is indicated by the index_ variable (int).
		 * - -1 : no type stored
		 * - 0 <= i < sizeof...(Types) : i-th type in the list
		 */
		static constexpr int invalid_index = -1;
		template <typename T>
		using GetTypePos = Detail::GetTypePos<typename std::decay<T>::type, Types...>;

	public:
		template <int index> using TypeForIndex = typename Detail::GetNthType<index, Types...>::Type;
		template <typename T> static constexpr int index_for_type () { return GetTypePos<T>::value; }

		Static () = default;

		template <typename T, int index = GetTypePos<T>::value> explicit Static (T && t) {
			// This constructor is SFINAE restricted to supported types
			new (&storage_) typename std::decay<T>::type (std::forward<T> (t));
			index_ = index_for_type<T> ();
		}
		template <typename T, typename... Args> explicit Static (InPlace<T>, Args &&... args) {
			new (&storage_) T (std::forward<Args> (args)...);
			index_ = index_for_type<T> ();
		}

		~Static () { type_ops (index_).destroy (&storage_); }

		// TODO delete if not all can be copied ? noexcept spec ?
		Static (const Static & other) {
			other.type_ops (other.index_).copy_construct (&storage_, &other.storage_);
			index_ = other.index_;
		}
		Static & operator= (const Static & other) {
			type_ops (index_).destroy (&storage_);
			index_ = invalid_index;
			other.type_ops (other.index_).copy_construct (&storage_, &other.storage_);
			index_ = other.index_;
			return *this;
		}
		Static (Static && other) {
			other.type_ops (other.index_).move_construct (&storage_, &other.storage_);
			index_ = other.index_;
		}
		Static & operator= (Static && other) {
			type_ops (index_).destroy (&storage_);
			index_ = invalid_index;
			other.type_ops (other.index_).move_construct (&storage_, &other.storage_);
			index_ = other.index_;
			return *this;
		}

		constexpr int index () const noexcept { return index_; }
		constexpr bool valid () const noexcept { return index () != invalid_index; }

		// TODO template access, ...

	private:
		int index_{invalid_index};
		typename std::aligned_storage<size, alignment>::type storage_;

		/* Table of type operations by index.
		 * Computed at compile time.
		 * Must be hidden as a static var in a function due to linking restrictions.
		 * (a static constexpr class variable would have no symbol generated).
		 * The table is shifted by 1.
		 * index 0 of the table refers to "no type", and has noop type operations.
		 */
		static const Type::Operations & type_ops (int index) {
			static constexpr Type::Operations ops_by_index[sizeof...(Types) + 1] = {
			    Type::operations<void> (), Type::operations<Types> ()...};
			return ops_by_index[index + 1];
		}
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
