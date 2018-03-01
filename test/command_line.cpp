#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include <cstdio>
#include <fmt/format.h>

#include <duck/command_line.h>

TEST_CASE ("Usage message") {
	// No actual checks, but useful to manually look at the formatting for different cases

	auto parser = CommandLineParser ();

	SUBCASE ("no arg, no opt") {}
	SUBCASE ("args, no opt") {
		parser.add_positional_argument ("arg1", "Argument 1", [](string_view) {});
		parser.add_positional_argument ("arg_______2", "Argument 2", [](string_view) {});
	}
	SUBCASE ("no args, opts") {
		parser.add_flag ({"h", "help"}, "Shows help", []() {});
		parser.add_value_option ({"f", "file"}, "file", "A file argument", [](string_view) {});
	}
	SUBCASE ("args and opts") {
		parser.add_flag ({"h", "help"}, "Shows help", []() {});
		parser.add_value_option ({"f", "file"}, "file", "A file argument", [](string_view) {});
		parser.add_positional_argument ("arg1", "Argument 1", [](string_view) {});
		parser.add_positional_argument ("arg_______2", "Argument 2", [](string_view) {});
	}

	fmt::print (stdout, "#################################\n");
	parser.usage (stdout, "test");
}

TEST_CASE ("Construction errors") {
	CommandLineParser parser;
	parser.add_flag ({"h", "help"}, "Shows help", []() {});

	// No name
	CHECK_THROWS_AS (parser.add_flag ({}, "blah", []() {}), CommandLineParser::Exception);
	// Empty name
	CHECK_THROWS_AS (parser.add_flag ({""}, "blah", []() {}), CommandLineParser::Exception);
	// Name collision
	CHECK_THROWS_AS (parser.add_flag ({"h"}, "blah", []() {}), CommandLineParser::Exception);
}

TEST_CASE ("Parsing") {
	{
		const char * argv_data[] = {"prog_name", "-f", "--f"};
		auto argv = CommandLineView (sizeof (argv_data) / sizeof (*argv_data), argv_data);

		// f as a flag should match both
		auto flag_parser = CommandLineParser ();
		int flag_parser_f_seen = 0;
		flag_parser.add_flag ({"f"}, "", [&]() { flag_parser_f_seen++; });
		flag_parser.parse (argv);
		CHECK (flag_parser_f_seen == 2);

		// f as value opt eats the second --f
		auto value_parser = CommandLineParser ();
		value_parser.add_value_option ({"f"}, "", "",
		                               [](string_view value) { CHECK (value == "--f"); });
		value_parser.parse (argv);

		// Fails because args look like opts that are not defined
		auto nothing_parser = CommandLineParser ();
		CHECK_THROWS_AS (nothing_parser.parse (argv), CommandLineParser::Exception);

		// Success, no option declared
		auto arg_arg_parser = CommandLineParser ();
		arg_arg_parser.add_positional_argument ("1", "", [](string_view v) { CHECK (v == "-f"); });
		arg_arg_parser.add_positional_argument ("2", "", [](string_view v) { CHECK (v == "--f"); });
		arg_arg_parser.parse (argv);

		// Fails, with options parsing enabled '-f' is unknown.
		auto arg_opt_parser = CommandLineParser ();
		arg_opt_parser.add_flag ({"z"}, "", []() {});
		arg_opt_parser.add_positional_argument ("1", "", [](string_view) {});
		arg_opt_parser.add_positional_argument ("2", "", [](string_view) {});
		CHECK_THROWS_AS (arg_opt_parser.parse (argv), CommandLineParser::Exception);
	}
	{
		const char * argv_data[] = {"prog_name", "1a", "a2"};
		auto argv = CommandLineView (sizeof (argv_data) / sizeof (*argv_data), argv_data);

		// One unexpected arg
		auto arg_parser = CommandLineParser ();
		arg_parser.add_positional_argument ("1", "", [](string_view) {});
		CHECK_THROWS_AS (arg_parser.parse (argv), CommandLineParser::Exception);

		// Eats both args
		auto arg_arg_parser = CommandLineParser ();
		arg_arg_parser.add_positional_argument ("1", "", [](string_view v) { CHECK (v == "1a"); });
		arg_arg_parser.add_positional_argument ("2", "", [](string_view v) { CHECK (v == "a2"); });
		arg_arg_parser.parse (argv);

		// Missing one arg
		auto arg_arg_arg_parser = CommandLineParser ();
		arg_arg_arg_parser.add_positional_argument ("1", "", [](string_view) {});
		arg_arg_arg_parser.add_positional_argument ("2", "", [](string_view) {});
		arg_arg_arg_parser.add_positional_argument ("3", "", [](string_view) {});
		CHECK_THROWS_AS (arg_arg_arg_parser.parse (argv), CommandLineParser::Exception);
	}
	{
		// Value arg parsing
		const char * argv_data[] = {"prog_name", "-opt=value", "--opt", "value", "-opt", "value"};
		auto argv = CommandLineView (sizeof (argv_data) / sizeof (*argv_data), argv_data);

		auto parser = CommandLineParser ();
		parser.add_value_option ({"opt"}, "value", "desc", [](string_view v) { CHECK (v == "value"); });
		parser.parse (argv);
	}
}
