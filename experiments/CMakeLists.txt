enable_testing ()

add_executable (dynamic_struct dynamic_struct.cpp)
target_link_libraries (dynamic_struct PRIVATE doctest)
target_include_directories (dynamic_struct PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options (dynamic_struct PRIVATE -Wall -Wextra -O2)
add_test (
	NAME dynamic_struct
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND dynamic_struct
	)
