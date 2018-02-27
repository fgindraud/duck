cmake_minimum_required (VERSION 3.2)
project (duck LANGUAGES CXX VERSION 0.0.1)

# Use C++14 for now
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED YES)
set (CMAKE_CXX_EXTENSIONS NO)

# Deps
add_subdirectory (external/fmt EXCLUDE_FROM_ALL) # fmt target

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

# Check demangling support (used in tests)
include (CheckCXXSymbolExists)
CHECK_CXX_SYMBOL_EXISTS("abi::__cxa_demangle" "cxxabi.h" DUCK_HAVE_DEMANGLING)

# Header only library target (no compilation, just -I and requirements)
add_library (lib INTERFACE)
target_include_directories (lib INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
	$<INSTALL_INTERFACE:include>
	)
target_link_libraries (lib INTERFACE fmt)

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
