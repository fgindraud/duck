cmake_minimum_required (VERSION 3.6)
project (cpp-utils LANGUAGES CXX)

enable_testing ()

# For testing only
find_package (Boost COMPONENTS unit_test_framework REQUIRED)

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
install (DIRECTORY include/ DESTINATION include)
install (EXPORT cpp-utils DESTINATION share/cpp-utils/cmake)
export (TARGETS lib FILE cpp-utils.cmake)

### Tests ###

# Tests that should compile and run
# These use boost testing framework
file (GLOB test_files_run RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*_run.cpp)
foreach (test_file ${test_files_run})
	get_filename_component (test_name ${test_file} NAME_WE)
	add_executable (${test_name} ${test_file})
	target_link_libraries (${test_name} lib Boost::unit_test_framework)
	target_compile_definitions (${test_name} PRIVATE "BOOST_TEST_DYN_LINK")
	set_target_properties (
		${test_name} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_bin
		)
	add_test (
		NAME ${test_name}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_bin
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_bin/${test_name}
		)
endforeach (test_file)

# Tests that should not compile (checking compile time failures)
# No testing framework, just a main
file (GLOB test_files_nocompile RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} test/*_nocompile.cpp)
foreach (test_file ${test_files_nocompile})
	get_filename_component (test_name ${test_file} NAME_WE)
	add_executable (${test_name} ${test_file})
	target_link_libraries (${test_name} lib)
	set_target_properties (
		${test_name} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_bin
		EXCLUDE_FROM_ALL TRUE # Do not build with make
		EXCLUDE_FROM_DEFAULT_BUILD TRUE
		)
	add_test (
		NAME ${test_name}
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMAND ${CMAKE_COMMAND} --build . --target ${test_name} --config $<CONFIGURATION>
		)
	set_tests_properties (${test_name} PROPERTIES WILL_FAIL TRUE)
endforeach (test_file)
