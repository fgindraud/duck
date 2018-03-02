#include <duck/view.h>

#include <algorithm>
#include <cstdlib>

namespace duck {
// string_view

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
		r.emplace_back (part_begin, part_end);
		if (part_end == end) {
			break;
		}
		part_begin = part_end + 1; // Skip separator
	}
	return r;
}
} // namespace duck
