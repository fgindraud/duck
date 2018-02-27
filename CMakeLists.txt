cmake_minimum_required (VERSION 3.2)
project (duck LANGUAGES CXX VERSION 0.1)

# Use C++14 for now
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED YES)
set (CMAKE_CXX_EXTENSIONS NO)

# Deps
set (FMT_INSTALL ON)
add_subdirectory (external/fmt EXCLUDE_FROM_ALL) # fmt target

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

### Tests ###
enable_testing ()
add_subdirectory (test)
