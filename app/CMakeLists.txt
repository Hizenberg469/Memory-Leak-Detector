set(TESTING_APP "app.c")

add_executable(${TEST_APP} 
					${TESTING_APP})

target_include_directories(${TEST_APP} PUBLIC
							${HEADER_DIR})

target_link_libraries(${TEST_APP} PUBLIC
							${MLD_LIB})