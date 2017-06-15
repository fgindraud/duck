#pragma once

// Formatting library: basic combinating elements
// STATUS: WIP

#include <duck/format/core.h>
#include <type_traits>
#include <utility> // forward / move

namespace duck {
namespace Format {
	/* Repeats a formatter a fixed number of times.
	 */
	template <typename F> class Repeated : public Base<Repeated<F>> {
	private:
		F formatter_;
		std::size_t times_;

	public:
		constexpr Repeated (const F & f, std::size_t times) : formatter_ (f), times_ (times) {}
		constexpr Repeated (F && f, std::size_t times) : formatter_ (std::move (f)), times_ (times) {}

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

	// TODO add padding, ...
	// TODO add iterable
}
}
