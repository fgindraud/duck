cmake_minimum_required (VERSION 3.6)
project (cpp-utils LANGUAGES CXX)

### Lib ###

# List of C++ features that we use
set (wanted_features
	cxx_alias_templates
	cxx_alignof
	cxx_auto_type
	cxx_constexpr
	cxx_default_function_template_args
	cxx_defaulted_functions
	cxx_delegating_constructors
	cxx_deleted_functions
	cxx_generalized_initializers
	cxx_noexcept
	cxx_nonstatic_member_init
	cxx_nullptr
	cxx_override
	cxx_range_for
	cxx_relaxed_constexpr
	cxx_return_type_deduction
	cxx_rvalue_references
	cxx_static_assert
	cxx_variadic_templates
	)

# Header only library target (no compilation, just -I and requirements)
add_library (lib INTERFACE)
target_include_directories (lib INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
	)
target_compile_features (lib INTERFACE ${wanted_features}) # This will force a -std=c++14 if needed

# Install header lib and cmake crap
install (TARGETS lib EXPORT cpp-utils
	ARCHIVE  DESTINATION lib
	LIBRARY  DESTINATION lib
	RUNTIME  DESTINATION bin
	)
install (DIRECTORY include DESTINATION include)
install (EXPORT cpp-utils DESTINATION share/cpp-utils/cmake)
export (TARGETS lib FILE cpp-utils.cmake)

### Tests ###

enable_testing ()
find_package (Boost COMPONENTS unit_test_framework REQUIRED)

set (TEST_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_bin)

file (GLOB test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*.cpp)
foreach (test_file ${test_files})
	get_filename_component (test_name ${test_file} NAME_WE)
	# Divide between two types of test:
	# - normal test, compile then run with no failed assert
	# - compile_failure tests, that should fail to compile
	if (${test_name} MATCHES ".*_nocompile")
		# To avoid numerous files, these tests can define N cases (use #ifdef)
		# We will try to compile it N times with TEST_CASE_<i> defined each time
		# N is determined from the file name
		if (${test_name} MATCHES ".*_([0-9]+)_nocompile")
			set (n ${CMAKE_MATCH_1})
		else (${test_name} MATCHES ".*_([0-9]+)_nocompile")
			set (n 1)
		endif (${test_name} MATCHES ".*_([0-9]+)_nocompile")
		foreach (i RANGE 1 ${n})
			# Generate <i>-th case
			set (sub_test_name "${test_name}_${i}")
			add_executable (${sub_test_name} ${test_file})
			target_link_libraries (${sub_test_name} lib)
			target_compile_definitions (${sub_test_name} PRIVATE "TEST_CASE_${i}")
			set_target_properties (
				${sub_test_name} PROPERTIES
				RUNTIME_OUTPUT_DIRECTORY ${TEST_BIN_DIR}
				# Prevent building with normal tests
				EXCLUDE_FROM_ALL TRUE
				EXCLUDE_FROM_DEFAULT_BUILD TRUE
				)
			add_test (
				NAME ${sub_test_name}
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				COMMAND ${CMAKE_COMMAND} --build . --target ${sub_test_name} --config $<CONFIGURATION>
				)
			set_tests_properties (${sub_test_name} PROPERTIES WILL_FAIL TRUE)
		endforeach (i RANGE 1 ${n})
	else (${test_name} MATCHES ".*_nocompile")
		# Normal test, compile and run later
		# I use boost unit test framework for these
		add_executable (${test_name} ${test_file})
		target_link_libraries (${test_name} lib Boost::unit_test_framework)
		target_compile_definitions (${test_name} PRIVATE "BOOST_TEST_DYN_LINK")
		set_target_properties (${test_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TEST_BIN_DIR})
		add_test (
			NAME ${test_name}
			WORKING_DIRECTORY ${TEST_BIN_DIR}
			COMMAND ${TEST_BIN_DIR}/${test_name}
			)
	endif (${test_name} MATCHES ".*_nocompile")
endforeach (test_file)
