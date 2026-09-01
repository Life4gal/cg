add_executable(
	cg-tests

	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/test_host_python.cpp
)

target_compile_options(
	cg-tests
	PUBLIC
	
	${CG_COMPILE_FLAGS}
)

target_compile_features(
	cg-tests
	PRIVATE
	cxx_std_23
)

target_link_libraries(
	cg-tests
	PRIVATE
	
	cg-core
	doctest::doctest
)
