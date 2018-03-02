#include <duck/command_line.h>

#include <algorithm>
#include <cassert>
#include <regex>
#include <string>
#include <vector>

#include <duck/format.h>

namespace duck {
auto CommandLineParser::new_named_uninitialized_option (std::initializer_list<string_view> names)
    -> Option & {
	if (names.size () == 0) {
		throw Exception ("Declaring option with no name");
	}
	auto index = static_cast<int> (options_.size ());
	options_.emplace_back ();
	for (auto name : names) {
		if (name.empty ()) {
			throw Exception ("Empty option name declaration");
		}
		auto r = option_index_by_name_.emplace (to_string (name), index);
		if (!r.second) {
			throw Exception (fmt::format ("Option '{}' has already been declared", name));
		}
	}
	return options_.back ();
}

void CommandLineParser::add_flag (std::initializer_list<string_view> names, string_view description,
                                  std::function<void()> action) {
	auto & opt = new_named_uninitialized_option (names);
	opt.type = Option::Type::Flag;
	opt.flag_action = std::move (action);
	opt.description = to_string (description);
}

void CommandLineParser::add_value_option (std::initializer_list<string_view> names,
                                          string_view value_name, string_view description,
                                          std::function<void(string_view value)> action) {
	auto & opt = new_named_uninitialized_option (names);
	opt.type = Option::Type::Value;
	opt.value_action = std::move (action);
	opt.value_name = to_string (value_name);
	opt.description = to_string (description);
}

void CommandLineParser::add_positional_argument (string_view value_name, string_view description,
                                                 std::function<void(string_view value)> action) {
	positional_arguments_.emplace_back ();
	auto & arg = positional_arguments_.back ();
	arg.action = std::move (action);
	arg.value_name = to_string (value_name);
	arg.description = to_string (description);
}

void CommandLineParser::usage (std::FILE * output, string_view program_name) const {
	std::size_t description_text_offset = 0;

	const bool has_options = !option_index_by_name_.empty ();
	const bool has_positional_arguments = !positional_arguments_.empty ();

	std::vector<std::string> option_text (options_.size ());
	std::string usage_line_pos_arg_text;

	if (has_options) {
		assert (!options_.empty ());
		// List of options : create option text
		for (const auto & e : option_index_by_name_) {
			// Append option name to option line
			auto & text = option_text[e.second];
			const auto & opt_name = e.first;
			if (!text.empty ()) {
				text.append (", ");
			}
			text.append (opt_name.size () == 1 ? "-" : "--");
			text.append (opt_name);
		}
		for (std::size_t i = 0; i < options_.size (); ++i) {
			const auto & opt = options_[i];
			if (opt.type == Option::Type::Value) {
				// Append value name for value options
				auto & text = option_text[i];
				text.append (" <");
				text.append (opt.value_name);
				text.append (">");
			}
		}

		// Get len for alignment of description text
		auto max_option_text_len_it = std::max_element (
		    option_text.begin (), option_text.end (),
		    [](const std::string & lhs, const std::string & rhs) { return lhs.size () < rhs.size (); });
		description_text_offset = std::max (description_text_offset, max_option_text_len_it->size ());
	}

	if (has_positional_arguments) {
		for (const auto & pos_arg : positional_arguments_) {
			usage_line_pos_arg_text.append (" ");
			usage_line_pos_arg_text.append (pos_arg.value_name);
			description_text_offset = std::max (description_text_offset, pos_arg.value_name.size ());
		}
	}

	fmt::print (output, "Usage: {}{}{}\n", program_name, has_options ? " [options]" : "",
	            usage_line_pos_arg_text);

	auto print_opt_arg_line = [this, &output, &description_text_offset](string_view name,
	                                                                    string_view description) {
		fmt::print (output, "  {0: <{1}}  {2}\n", name, description_text_offset, description);
	};

	if (has_options) {
		fmt::print (output, "\nOptions:\n");
		print_opt_arg_line ("--", "Disable option parsing");
		for (std::size_t i = 0; i < options_.size (); ++i) {
			print_opt_arg_line (option_text[i], options_[i].description);
		}
	}
	if (has_positional_arguments) {
		fmt::print (output, "\nArguments:\n");
		for (const auto & pos_arg : positional_arguments_) {
			print_opt_arg_line (pos_arg.value_name, pos_arg.description);
		}
	}
}

static string_view make_string_view (std::cmatch::const_reference match_result) {
	return {match_result.first, match_result.second};
}

void CommandLineParser::parse (const CommandLineView & command_line) {
	std::size_t current = 0;
	const std::size_t nb = command_line.nb_arguments ();

	std::size_t nb_pos_arg_seen = 0;
	const std::size_t nb_pos_arg_needed = positional_arguments_.size ();

	bool option_parsing_enabled = !option_index_by_name_.empty ();

	// Option is: one or two dashes, then option name, then '=<value>' or nothing
	std::regex option_name_regex{"^(--|-)([^=]+)(=|$)"};
	assert (option_name_regex.mark_count () == 3);
	std::cmatch matched_positions;

	while (current < nb) {
		auto arg = command_line.argument (current);

		if (option_parsing_enabled && is_prefix_of ('-', arg)) {
			if (arg == "--") {
				// Special case, consider everything after that as positional arguments
				option_parsing_enabled = false;
			} else {
				// Argument is an option
				if (!std::regex_search (arg.begin (), arg.end (), matched_positions, option_name_regex)) {
					throw Exception (fmt::format ("Bad format: option '{}'", arg));
				}
				assert (matched_positions.ready ());
				assert (matched_positions.size () == 4);
				auto dashes = make_string_view (matched_positions[1]);
				auto opt_name = make_string_view (matched_positions[2]);
				auto equal_sign_or_empty = make_string_view (matched_positions[3]);
				auto opt_name_with_dashes = string_view (dashes.begin (), opt_name.end ());

				auto it = option_index_by_name_.find (opt_name);
				if (it == option_index_by_name_.end ()) {
					throw Exception (fmt::format ("Unknown option '{}'", opt_name_with_dashes));
				}
				Option & opt = options_[static_cast<std::size_t> (it->second)];

				if (opt.type == Option::Type::Flag) {
					// Simple flag option
					if (!equal_sign_or_empty.empty ()) {
						throw Exception (fmt::format ("Flag '{}' takes no value", opt_name_with_dashes));
					}
					assert (opt.flag_action);
					opt.flag_action ();
				} else {
					// Value option, extract value
					string_view value;
					if (!equal_sign_or_empty.empty ()) {
						// Value is the rest of the argument
						value = string_view (equal_sign_or_empty.end (), arg.end ());
					} else {
						// Value is the next argument
						++current;
						if (current == nb) {
							throw Exception (fmt::format ("Option '{}' requires a value", opt_name_with_dashes));
						}
						value = command_line.argument (current);
					}
					assert (opt.value_action);
					opt.value_action (value);
				}
			}
		} else {
			// Argument is a positional argument
			if (nb_pos_arg_seen == nb_pos_arg_needed) {
				throw Exception (
				    fmt::format ("Unexpected argument '{}' at position {}: requires {} arguments", arg,
				                 nb_pos_arg_seen, nb_pos_arg_needed));
			}
			auto & pos_arg = positional_arguments_[nb_pos_arg_seen];
			assert (pos_arg.action);
			pos_arg.action (arg);
			++nb_pos_arg_seen;
		}
		++current;
	}

	if (nb_pos_arg_seen != nb_pos_arg_needed) {
		throw Exception (fmt::format ("Only {} positional arguments given, {} requested",
		                              nb_pos_arg_seen, nb_pos_arg_needed));
	}
}

CommandLineParser::Exception::Exception (string_view message) : message_ (to_string (message)) {}
CommandLineParser::Exception::Exception (std::string && message) : message_ (std::move (message)) {}
CommandLineParser::Exception::Exception (const char * message) : message_ (message) {}

const char * CommandLineParser::Exception::what () const noexcept {
	return message_.c_str ();
}
} // namespace duck
