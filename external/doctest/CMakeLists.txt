# Doctest is added a standalone header file, taken from the releases downloads

# Define a doctest library (header-only)
add_library (doctest INTERFACE)
target_include_directories (doctest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
