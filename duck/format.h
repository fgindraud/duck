#pragma once

// Printf utilities

#include <fmt/format.h>

#include <duck/view.h>

/* Fmtlib defines its own string_view.
 * Enable formatting for our own string_view, by just converting to fmtlib's one.
 * Inside namespace fmt:
 * duck::string_view is our own.
 * string_view is the fmtlib one.
 */
namespace fmt {
template <> struct formatter<duck::string_view> : formatter<string_view> {
	// parse() is directly inherited

	// Wrapper for format()
	template <typename FormatContext> auto format (duck::string_view sv, FormatContext & ctx) {
		return formatter<string_view>::format (string_view (sv.data (), sv.size ()), ctx);
	}
};
} // namespace fmt
