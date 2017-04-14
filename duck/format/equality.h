#pragma once

// Non allocating comparison with strings

#include <duck/format/format.h>
#include <duck/iterator/facade.h>

namespace duck {
namespace Format {

	namespace Detail {
		template <typename InputIt> class ComparisonIteratorImpl {
		private:
			friend struct CompareOnAssign;
			struct CompareOnAssign {
				ComparisonIteratorImpl & it;
				template <typename T> CompareOnAssign & operator= (const T & t) {
					if (it.equal_ && it.current_ != it.end_)
						it.equal_ = (*it.current_ == t);
					return *this;
				}
			};

		public:
			using iterator_category = std::output_iterator_tag;
			using value_type = void;
			using pointer = void;

			ComparisonIteratorImpl () = default;
			ComparisonIteratorImpl (InputIt begin, InputIt end) : current_ (begin), end_ (end) {}

			operator bool () const { return equal_ && current_ == end_; }

			CompareOnAssign deref () { return {*this}; }
			void next () {
				if (current_ == end_)
					equal_ = false; // testing past the end
				else
					++current_;
			}

		private:
			InputIt current_{};
			InputIt end_{};
			bool equal_{true};
		};

		template <typename InputIt>
		using ComparisonIterator = Iterator::Facade<ComparisonIteratorImpl<InputIt>>;

		template <std::size_t N>
		auto make_comparison_iterator (const char (&str)[N])
		    -> ComparisonIterator<decltype (std::begin (str))> {
			return {std::begin (str), std::end (str) - 1};
		}

		auto make_comparison_iterator (const std::string & str)
		    -> ComparisonIterator<std::string::const_iterator> {
			return {std::begin (str), std::end (str)};
		}
	}

	template <typename F, typename T, typename = typename std::enable_if<IsFormatter<F>::value>::type>
	bool operator== (const F & formatter, const T & t) {
		return formatter.write (Detail::make_comparison_iterator (t));
	}

	//TODO clean... this is awkward
}
}
