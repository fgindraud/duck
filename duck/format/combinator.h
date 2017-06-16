#pragma once

// Formatting library: basic combinating elements
// STATUS: WIP

#include <duck/format/core.h>
#include <iterator>
#include <stdexcept>
#include <utility> // forward / move

namespace duck {

namespace Iterator {
	template <typename OutputIt> class TruncatedOutput {
		/* Output iterator which "stops working" after a certain amount of writes.
		 * Useful to truncate the output to a specific size.
		 * TODO split counted iterator from this ? (more complex)
		 * TODO move to iterator/
		 */
	private:
		friend struct assignifunderlimit;
		struct AssignIfUnderLimit {
			// Temporary struct that only performs the assign if under the limit.
			TruncatedOutput & it;
			template <typename T> AssignIfUnderLimit & operator= (T && t) {
				if (it.count_ < it.limit_)
					*it.current_ = std::forward<T> (t);
				return *this;
			}
		};

	public:
		using iterator_category = std::output_iterator_tag;
		using reference = AssignIfUnderLimit;
		using value_type = void;
		using pointer = void;
		using difference_type = std::ptrdiff_t;

		constexpr TruncatedOutput () = default;
		constexpr TruncatedOutput (OutputIt it, std::size_t limit)
		    : current_ (std::move (it)), count_ (0), limit_ (limit) {}

		TruncatedOutput & operator++ () {
			if (count_ < limit_) {
				++current_;
				++count_;
			}
			return *this;
		}
		reference operator* () noexcept { return {*this}; }

		constexpr OutputIt base () const { return current_; }

	private:
		OutputIt current_;
		std::size_t count_;
		std::size_t limit_;
	};
}

namespace Format {
	/* Repeats a formatter a fixed number of times.
	 */
	template <typename F> class Repeated : public Base<Repeated<F>> {
	private:
		F formatter_;
		std::size_t times_;

	public:
		template <typename F_>
		constexpr Repeated (F_ && f, std::size_t times)
		    : formatter_ (std::forward<F_> (f)), times_ (times) {}

		constexpr std::size_t size () const { return formatter_.size () * times_; }
		template <typename OutputIt> OutputIt write (OutputIt it) const {
			for (std::size_t i = 0; i < times_; ++i)
				it = formatter_.write (it);
			return it;
		}

		constexpr const F & formatter () const & noexcept { return formatter_; }
		F && formatter () && noexcept { return std::move (formatter_); }
		constexpr std::size_t times () const noexcept { return times_; }
	};
	template <typename F> constexpr Repeated<F> repeated (F && f, std::size_t times) {
		return {std::forward<F> (f), times};
	}

	/* Left pad formatter F with formatter Pad until it reaches size.
	 * If pad.size () == 0, throw exception (cannot pad).
	 * If pad.size () > 1, padding may stop before reaching size.
	 */
	template <typename F, typename Pad> class LeftPad : public Base<LeftPad<F, Pad>> {
	private:
		F formatter_;
		Pad padder_;
		std::size_t target_size_;

	public:
		template <typename F_, typename Pad_>
		constexpr LeftPad (F_ && f, Pad_ && pad, std::size_t target_size)
		    : formatter_ (std::forward<F_> (f)),
		      padder_ (std::forward<Pad_> (pad)),
		      target_size_ (target_size) {}

		std::size_t size () const noexcept {}

		template <typename OutputIt> OutputIt write (OutputIt it) const {
			const auto pad_size = padder_.size ();
			const auto formatter_size = formatter_.size ();
			return it; // TODO think of the semantic
		}
	};

	// TODO add padding, ...
	// TODO add iterable / join (may need placeholder or placeholder like system ?)
}
}
