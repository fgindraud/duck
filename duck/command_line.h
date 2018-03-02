#pragma once

// Command line parser
// STATUS: operational

#include <exception>
#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <vector>

#include <duck/view.h>

namespace duck {
/* Command line view.
 * Nicer interface to represent (argc, argv).
 */
class CommandLineView {
public:
	CommandLineView (int argc, const char * const * argv) : argc_ (argc), argv_ (argv) {}

	// Raw access
	string_view program_name () const { return {argv_[0]}; }
	std::size_t nb_arguments () const noexcept { return argc_ - 1; }
	string_view argument (std::size_t index) const { return {argv_[index + 1]}; }

private:
	int argc_;
	const char * const * argv_;
};

/* Command line argument parser.
 *
 * Declare flags, options with values and positional arguments.
 * Provide callback functions that will be called when the options are detected (actions).
 * Then call parse, which will parse the entire command line and trigger callbacks.
 * Also generate a usage message from the list of options.
 */
class CommandLineParser {
public:
	class Exception;

	/* Declare options.
	 * Throws Exception if no names are given for named options.
	 * 'action' is the function to call when the option is discovered.
	 * Supported option names can contain anything else than '='.
	 * Due to shell parsing, users should only use alphanumeric and '-' '_' to avoid problems.
	 *
	 * Usage text:
	 * 'description' should fit on one line (very short text).
	 * 'value_name' is the name of the value pattern.
	 */
	void add_flag (std::initializer_list<string_view> names, string_view description,
	               std::function<void()> action);
	void add_value_option (std::initializer_list<string_view> names, string_view value_name,
	                       string_view description, std::function<void(string_view value)> action);

	/* Declare positional arguments.
	 * Each call of add_positional_argument declares a single argument.
	 * Arguments declared that way are mandatory (all must be present).
	 * They are parsed in the order of declaration.
	 * Usage text is the same as for options.
	 */
	void add_positional_argument (string_view value_name, string_view description,
	                              std::function<void(string_view value)> action);

	/* Print usage to output.
	 * Options are given in the order of declaration.
	 * Options names are listed lexicographically.
	 */
	void usage (std::FILE * output, string_view program_name) const;

	/* Parse arguments in order of appearance on the command line.
	 *
	 * For each flag or option, call the appropriate callback each time it is discovered.
	 * Anything starting with '-' is considered an option, and must match a defined option name.
	 * Options can use single or dual dashes ('-' / '--') even for short options (one letter).
	 * Value options will consume data after an equal sign ('-f=value'), or the next argument.
	 * The value is given to the action callback.
	 *
	 * All elements which are not option-like are considered positional arguments.
	 * The number of all found positional argument must match the declared ones.
	 * Values are given to the callbacks in order of declaration.
	 *
	 * Option '--' stops option parsing: next arguments are always considered positional.
	 * A parser with no options declared starts as if '--' was passed before any user argument.
	 *
	 * Throws Exception if a parsing error occurs.
	 */
	void parse (const CommandLineView & command_line);

private:
	/* Options are stored in a vector, and indexed by name for quick search
	 *
	 * Use std::map as we search with a string_view, and only map supports template keys.
	 * An unordered_map would create a std::string everytime a comparison is made.
	 */
	struct Option {
		enum class Type {
			Flag, // Option is an optional flag without value
			Value // Option is an optional flag with a value
		};
		Type type;

		std::function<void(string_view value)> value_action;
		std::function<void()> flag_action;

		// Usage text (may be null)
		std::string value_name;
		std::string description;
	};
	std::map<std::string, int, std::less<>> option_index_by_name_;
	std::vector<Option> options_;

	struct PositionalArgument {
		std::function<void(string_view value)> action;
		std::string value_name;
		std::string description;
	};
	std::vector<PositionalArgument> positional_arguments_;

	Option & new_named_uninitialized_option (std::initializer_list<string_view> names);
};

class CommandLineParser::Exception : public std::exception {
public:
	Exception (string_view message);
	Exception (std::string && message);
	Exception (const char * message);
	const char * what () const noexcept final;

private:
	std::string message_;
};
} // namespace duck
