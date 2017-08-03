#pragma once

// Variant classes
// Static takes a static list of types.
// Dynamic takes a size/alignement and supports any fitting type.
// SmallUniqPtr (in a separate header)
// STATUS: WIP

#include <duck/type_operations.h>
#include <duck/type_traits.h>
#include <duck/utility.h>
#include <exception>
#include <utility>

namespace duck {
namespace Variant {

	class BadVariantAccess : public std::exception {
	public:
		BadVariantAccess () = default;
		const char * what () const noexcept override {
			return "Variant accessed with type different from current type";
		}
	};

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

		// Wrap operator ()
		template <typename Visitor, typename T>
		void wrap_call_operator (void * storage, Visitor & visitor) {
			visitor (*reinterpret_cast<T *> (storage));
		}
		template <typename Visitor> void noop_call_operator (void *, Visitor &) {}
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

		template <typename T, int = GetTypePos<T>::value> explicit Static (T && t) {
			// This constructor is SFINAE restricted to supported types
			build<typename std::decay<T>::type> (std::forward<T> (t));
		}
		template <typename T, typename... Args> explicit Static (InPlace<T>, Args &&... args) {
			build<T> (std::forward<Args> (args)...);
		}

		~Static () { reset (); }

		// TODO delete if not all can be copied ? noexcept spec ?
		Static (const Static & other) {
			other.type_ops ().copy_construct (&storage_, &other.storage_);
			index_ = other.index ();
		}
		Static & operator= (const Static & other) {
			if (index () == other.index ()) {
				type_ops ().copy_assign (&storage_, &other.storage_);
			} else {
				reset ();
				other.type_ops ().copy_construct (&storage_, &other.storage_);
				index_ = other.index ();
			}
			return *this;
		}
		Static (Static && other) {
			other.type_ops ().move_construct (&storage_, &other.storage_);
			index_ = other.index ();
		}
		Static & operator= (Static && other) {
			if (index () == other.index ()) {
				type_ops ().move_assign (&storage_, &other.storage_);
			} else {
				reset ();
				other.type_ops ().move_construct (&storage_, &other.storage_);
				index_ = other.index ();
			}
			return *this;
		}

		constexpr int index () const noexcept { return index_; }
		constexpr bool valid () const noexcept { return index () != invalid_index; }
		template <typename T, int = GetTypePos<T>::value> constexpr bool is_type () const noexcept {
			return index () == index_for_type<T> ();
		}

		template <typename T, typename... Args> T & emplace (Args &&... args) {
			reset ();
			return build<T> (std::forward<Args> (args)...);
		}
		template <typename T, typename U, typename... Args>
		T & emplace (std::initializer_list<U> ilist, Args &&... args) {
			reset ();
			return build<T> (std::move (ilist), std::forward<Args> (args)...);
		}

		template <typename T> T & get_unsafe () noexcept { return reinterpret_cast<T &> (storage_); }
		template <typename T> const T & get_unsafe () const noexcept {
			return reinterpret_cast<const T &> (storage_);
		}
		template <typename T> T & get () {
			if (!is_type<T> ())
				throw BadVariantAccess{};
			return get_unsafe<T> ();
		}
		template <typename T> const T & get () const {
			if (!is_type<T> ())
				throw BadVariantAccess{};
			return get_unsafe<T> ();
		}

		// TODO support return type ?
		// TODO support stateful Visitor (by ref)
		// TODO static check of case coverage already done, make it less obscure on error ?
		template <typename Visitor> void visit (Visitor visitor) {
			using FuncType = void (*) (void * storage, Visitor & vis);
			static constexpr FuncType function_by_type[sizeof...(Types) + 1] = {
			    Detail::noop_call_operator<Visitor>, Detail::wrap_call_operator<Visitor, Types>...};
			function_by_type[index_ + 1](&storage_, visitor);
		}

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
		const Type::Operations & type_ops () const noexcept {
			static constexpr Type::Operations ops_by_index[sizeof...(Types) + 1] = {
			    Type::noop_operations (), Type::operations<Types> ()...};
			return ops_by_index[index_ + 1];
		}

		// Destroy object, put in invalid state
		void reset () noexcept {
			type_ops ().destroy (&storage_);
			index_ = invalid_index;
		}

		// Build in place
		template <typename T, typename... Args> T & build (Args &&... args) {
			auto * obj = new (&storage_) T (std::forward<Args> (args)...);
			index_ = index_for_type<T> ();
			return *obj;
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
				throw BadVariantAccess{};
			return reinterpret_cast<T &> (storage_);
		}
		template <typename T> const T & as () const {
			if (&Type::destroy<T> != destructor_)
				throw BadVariantAccess{};
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
