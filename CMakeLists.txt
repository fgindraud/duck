cmake_minimum_required (VERSION 3.2)
project (duck LANGUAGES CXX VERSION 0.0.1)

### Package ###
set (cmake_package_location lib/cmake/${PROJECT_NAME})
include (CMakePackageConfigHelpers)
configure_package_config_file (
	package.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	INSTALL_DESTINATION ${cmake_package_location}
	)
write_basic_package_version_file (
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
	)
install (FILES
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
	DESTINATION ${cmake_package_location}
	)

### Lib ###

# List of C++ features that we use (should map to c++11)
set (wanted_features
	cxx_alias_templates
	cxx_alignof
	cxx_auto_type
	cxx_constexpr
	cxx_decltype
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
	cxx_rvalue_references
	cxx_static_assert
	cxx_variadic_templates
	)

# Check demangling support (used in tests)
include (CheckCXXSymbolExists)
CHECK_CXX_SYMBOL_EXISTS("abi::__cxa_demangle" "cxxabi.h" DUCK_HAVE_DEMANGLING)

# Header only library target (no compilation, just -I and requirements)
add_library (lib INTERFACE)
target_include_directories (lib INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include>
	)
target_compile_features (lib INTERFACE ${wanted_features}) # This will force a -std=c++14 if needed

# Install header lib and cmake crap
install (TARGETS lib EXPORT ${PROJECT_NAME}-targets
	ARCHIVE  DESTINATION lib
	LIBRARY  DESTINATION lib
	RUNTIME  DESTINATION bin
	)
install (DIRECTORY ${PROJECT_NAME} DESTINATION include)
install (EXPORT ${PROJECT_NAME}-targets DESTINATION ${cmake_package_location})

### Tests ###
enable_testing ()
add_subdirectory (test)
