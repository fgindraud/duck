#pragma once

// Non allocating comparison with strings.
// STATUS: prototype

#include <duck/format/core.h>
#include <iterator>

namespace duck {
namespace Format {

	namespace Detail {
		template <typename InputIt> class ComparisonIterator {
			/* Weird output iterator performing a comparison.
			 * Given two range (ab, ae) and (bb, be):
			 * bool is_equal = std::copy (ab, ae, ComparisonIterator (bb, be));
			 * represents if the range values are equal.
			 *
			 * Each "write" to the ComparisonIterator performs a single element check.
			 */
		private:
			friend struct CompareOnAssign;
			struct CompareOnAssign {
				// Temporary struct used to implement the comparison on assignement.
				ComparisonIterator & it;
				template <typename T> CompareOnAssign & operator= (const T & t) {
					if (it.equal_ && it.current_ != it.end_)
						it.equal_ = (*it.current_ == t);
					return *this;
				}
			};

		public:
			using iterator_category = std::output_iterator_tag;
			using reference = CompareOnAssign;
			using value_type = void;
			using pointer = void;
			using difference_type = std::ptrdiff_t;

			constexpr ComparisonIterator () = default;
			constexpr ComparisonIterator (InputIt begin, InputIt end) noexcept
			    : current_ (begin), end_ (end) {}

			constexpr operator bool () const {
				// Test if equal so far, and have the same length (we parsed all of (begin, end)).
				return equal_ && current_ == end_;
			}

			// Output iterator interface
			ComparisonIterator & operator++ () {
				if (current_ == end_)
					equal_ = false; // Past the end, ranges are not equal in length
				else
					++current_;
				return *this;
			}
			reference operator* () noexcept { return {*this}; }

		private:
			InputIt current_{};
			InputIt end_{};
			bool equal_{true};
		};

		// Free functions that build a ComparisonIterator for string types.
		template <std::size_t N>
		constexpr auto make_comparison_iterator (const char (&str)[N])
		    -> ComparisonIterator<decltype (std::begin (str))> {
			return {std::begin (str), std::end (str) - 1};
		}

		inline auto make_comparison_iterator (const std::string & str)
		    -> ComparisonIterator<std::string::const_iterator> {
			return {std::begin (str), std::end (str)};
		}
	}

	// This iterator implements comparison of formatters with a fixed string.
	template <typename F, typename T, typename = enable_if_t<IsFormatter<F>::value>>
	constexpr bool operator== (const F & formatter, const T & t) {
		return formatter.write (Detail::make_comparison_iterator (t));
	}
}
}
