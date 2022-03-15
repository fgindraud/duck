#pragma once

// View structures
// STATUS: prototype, NSC

#include <vector>

#include <gsl.h>

namespace duck {
/* span<T> represents a reference to a segment of a T array.
 * Equivalent to a (T* base, int len).
 */
using gsl::span;

/* string_view: Const reference (pointer) to a sequence of char.
 * Does not own the data, only points to it.
 * The char sequence may not be null terminated.
 *
 * std::string_view exists in C++17, but not before sadly.
 */
using string_view = gsl::cstring_span;

using gsl::to_string; // Create new std::string from string_view

bool is_prefix_of (string_view prefix, string_view str);
bool is_prefix_of (char prefix, string_view str);

// Split at separator. Does not remove empty parts, does not trim whitespace.
std::vector<string_view> split (char separator, string_view text);
} // namespace duck
