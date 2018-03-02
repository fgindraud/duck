# Gsl is taken from gsl-lite (master)
# Added as a standalone header file, renamed to gsl.h

# Provides basic useful things like span, not_null & such.

# Define a gsl library (header-only)
add_library (gsl INTERFACE)
target_include_directories (gsl INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
