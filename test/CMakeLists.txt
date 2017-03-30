### Tests ###
enable_testing ()
file (GLOB test_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
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
				# Prevent building with normal tests
				EXCLUDE_FROM_ALL TRUE
				EXCLUDE_FROM_DEFAULT_BUILD TRUE
				)
			add_test (
				NAME ${sub_test_name}
				COMMAND ${CMAKE_COMMAND} --build . --target ${sub_test_name} --config $<CONFIGURATION>
				)
			set_tests_properties (${sub_test_name} PROPERTIES WILL_FAIL TRUE)
		endforeach (i RANGE 1 ${n})
	else (${test_name} MATCHES ".*_nocompile")
		# Normal test, compile and run later
		# I use boost unit test framework for these
		add_executable (${test_name} ${test_file})
		target_link_libraries (${test_name} lib)
		target_compile_options (${test_name} PRIVATE -Wall -Wextra)
		add_test (
			NAME ${test_name}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			COMMAND ${test_name}
			)
	endif (${test_name} MATCHES ".*_nocompile")
endforeach (test_file)
