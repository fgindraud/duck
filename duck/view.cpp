#include "view.h"

#include <algorithm>
#include <cstdlib>

// string_view

std::string to_string (string_view sv) {
	return {sv.begin (), sv.end ()};
}

bool is_prefix_of (string_view prefix, string_view str) {
	if (prefix.size () <= str.size ()) {
		return std::equal (prefix.begin (), prefix.end (), str.begin ());
	} else {
		return false;
	}
}
bool is_prefix_of (char prefix, string_view str) {
	return str.size () >= 1 && str.data ()[0] == prefix;
}

std::vector<string_view> split (char separator, string_view text) {
	std::vector<string_view> r;
	const auto end = text.end ();
	string_view::iterator part_begin = text.begin ();

	while (true) {
		auto part_end = std::find (part_begin, end, separator);
		r.emplace_back (make_string_view (part_begin, part_end));
		if (part_end == end) {
			break;
		}
		part_begin = part_end + 1; // Skip separator
	}
	return r;
}
