#include <cassert>
#include <limits>
#include <type_traits>

// Overflow helpers
template <typename Int> bool add_would_overflow (Int lhs, Int rhs) {
	static_assert (std::is_unsigned<Int>::value);
	return rhs > std::numeric_limits<Int>::max () - lhs;
}

template <typename Int, std::size_t decimal_bits> class FixedPoint {
	static_assert (std::is_unsigned<Int>::value);
	static_assert (decimal_bits < std::numeric_limits<Int>::digits);

private:
	Int raw_{};

	explicit FixedPoint (Int raw) noexcept : raw_ (raw) {}

	static constexpr Int decimal_mask = (Int (1) << decimal_bits) - 1;
	static constexpr Int integer_max = std::numeric_limits<Int>::max () >> decimal_bits;
	static constexpr Int infinity_raw = std::numeric_limits<Int>::max ();

public:
	FixedPoint () = default;

	static FixedPoint from_raw (Int raw) noexcept { return FixedPoint (raw); }
	static FixedPoint from_integer (Int i) noexcept {
		assert (i <= integer_max);
		return from_raw (i << decimal_bits);
	}
	static FixedPoint from_integer_saturating (Int i) noexcept {
		if (i <= integer_max) {
			return from_raw (i << decimal_bits);
		} else {
			return from_raw (infinity_raw);
		}
	}

	Int raw () const noexcept { return raw_; }

	Int integer_part () const noexcept { return raw_ >> decimal_bits; }
	Int decimal_part () const noexcept { return raw_ & decimal_mask; }

	explicit operator float () const noexcept { return float(raw_) / float(1 << decimal_bits); }
	explicit operator double () const noexcept { return double(raw_) / double(1 << decimal_bits); }
};

template <typename Int, std::size_t decimal_bits>
FixedPoint<Int, decimal_bits> operator+ (FixedPoint<Int, decimal_bits> lhs,
                                         FixedPoint<Int, decimal_bits> rhs) {
	assert (!add_would_overflow (lhs.raw (), rhs.raw ()));
	return FixedPoint<Int, decimal_bits>::from_raw (lhs.raw () + rhs.raw ());
}

template <typename Int> FixedPoint<Int, 0> to_fixed_point (Int i) {
	return FixedPoint<Int, 0>::from_raw (i);
}

int main () {
	auto f = to_fixed_point (42u);
	f + f;
	return 0;
}
