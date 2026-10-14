project(cg-app-dummy)

add_executable(
	${PROJECT_NAME}
	
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_compile_options(
	${PROJECT_NAME}

	PUBLIC
	${CG_COMPILE_FLAGS}
)

target_compile_features(
	${PROJECT_NAME}

	PRIVATE
	cxx_std_23
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	
	cg-core-1
)
