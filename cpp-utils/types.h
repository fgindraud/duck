#pragma once

#include <cpp-utils/integer.h>

#include <cstdint> // uintN_t

namespace Impl {
template <size_t N> struct UintSelector { using Type = typename UintSelector<N + 1>::Type; };
template <> struct UintSelector<8> { using Type = uint8_t; };
template <> struct UintSelector<16> { using Type = uint16_t; };
template <> struct UintSelector<32> { using Type = uint32_t; };
template <> struct UintSelector<64> { using Type = uint64_t; };
template <> struct UintSelector<65> {};
}

/* BoundUint<N> provides the unsigned integer type that can represent [0, N]
*/
template <size_t MaxValue>
using BoundUint = typename Impl::UintSelector<Integer::representation_bits (MaxValue)>::Type;
