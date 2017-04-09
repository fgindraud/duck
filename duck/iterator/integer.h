#pragma once

// Defines IntegerIterator (iterates on integral number spaces).
// Allows to generate an integer range with all range features.

#include <duck/iterator/facade.h>

namespace duck {

namespace Iterator {
	namespace Detail {
		template <typename Int> class IntegerImpl {
		public:
			using iterator_category = std::random_access_iterator_tag;
			using value_type = Int;
			using difference_type = std::ptrdiff_t;
			using pointer = const value_type *;
			using reference = const value_type &;

		protected:
			IntegerImpl () noexcept = default;
			IntegerImpl (Int n) noexcept : n_ (n) {}
			// + default ctors

			reference deref () const { return n_; }
			void next () { ++n_; }
			void prev () { --n_; }
			void advance (difference_type n) { n_ += n; }
			bool equal (const IntegerImpl & it) const { return distance (it) == 0; }
			difference_type distance (const IntegerImpl & it) const { return n_ - it.n_; }

		private:
			Int n_{};
		};
	}

	template <typename Int> using Integer = Facade<Detail::IntegerImpl<Int>>;
}
}
