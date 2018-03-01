#pragma once

// View structures
// STATUS: prototype, NSC

#include <string>
#include <vector>

#include <fmt/core.h> // Use their compatibility string_view

namespace duck {
/* string_view: Const reference (pointer) to a sequence of char.
 * Does not own the data, only points to it.
 * The char sequence may not be null terminated.
 *
 * std::string_view exists in C++17, but not before sadly.
 */
using fmt::string_view;

inline string_view make_string_view (const char * begin, const char * end) {
	return {begin, static_cast<std::size_t> (end - begin)};
}
std::string to_string (string_view sv);

bool is_prefix_of (string_view prefix, string_view str);
bool is_prefix_of (char prefix, string_view str);

// Split at separator. Does not remove empty parts, does not trim whitespace.
std::vector<string_view> split (char separator, string_view text);
} // namespace duck
