cmake_minimum_required (VERSION 3.6)
project (cpp-utils LANGUAGES CXX)

### Lib ###

# List of C++ features that we use
set (CXX_FEATURES
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

# TODO maybe find better #include <...> / directory system

# Header only library target (no compilation, just -I and requirements)
add_library (lib INTERFACE)
target_include_directories (lib INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/cpp-utils>
	$<INSTALL_INTERFACE:include/cpp-utils>
)
target_compile_features (lib INTERFACE ${CXX_FEATURES}) # This will force a -std=c++14 if needed

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
enable_testing ()

# Tests
add_executable (testlib test/main.cpp)
target_link_libraries (testlib lib)
add_test (testlib testlib)

# TODO add specific tests
