#pragma once

// Various utility bits

namespace duck {

// Constexpr max
template <typename T> constexpr const T & max (const T & t) {
	return t;
}
template <typename T> constexpr const T & max (const T & a, const T & b) {
	return a < b ? b : a;
}
template <typename T, typename... Others>
constexpr const T & max (const T & a, const T & b, const Others &... others) {
	return max (a, max (b, others...));
}

// Constexpr min
template <typename T> constexpr const T & min (const T & t) {
	return t;
}
template <typename T> constexpr const T & min (const T & a, const T & b) {
	return a < b ? a : b;
}
template <typename T, typename... Others>
constexpr const T & min (const T & a, const T & b, const Others &... others) {
	return min (a, min (b, others...));
}
}
