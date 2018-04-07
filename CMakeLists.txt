cmake_minimum_required (VERSION 3.2)
project (duck LANGUAGES CXX VERSION 0.1)

# Use C++14 for now
set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED YES)
set (CMAKE_CXX_EXTENSIONS NO)

# Deps
add_subdirectory (external/fmt EXCLUDE_FROM_ALL) # fmt target
add_subdirectory (external/doctest EXCLUDE_FROM_ALL) # doctest target
add_subdirectory (external/gsl EXCLUDE_FROM_ALL) # "gsl" target for utils

### Duck lib ###

# Check demangling support (used in tests)
include (CheckCXXSymbolExists)
CHECK_CXX_SYMBOL_EXISTS("abi::__cxa_demangle" "cxxabi.h" DUCK_HAVE_DEMANGLING)

# Lib
add_subdirectory (duck)

### Tests ###
enable_testing ()
add_subdirectory (test)

### Experiments ###
add_subdirectory (experiments)
