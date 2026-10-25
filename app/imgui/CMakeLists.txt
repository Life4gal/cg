project(cg-app-imgui)

find_package(glfw3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

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
	imgui::imgui
	glfw
	OpenGL::GL
)

# 复制本地对局所需资源到可执行文件目录
add_custom_target(
	${PROJECT_NAME}-resources
	ALL

	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
		${CMAKE_SOURCE_DIR}/configs
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/configs
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
		${CMAKE_SOURCE_DIR}/decks
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/decks
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
		${CMAKE_SOURCE_DIR}/cards
		$<TARGET_FILE_DIR:${PROJECT_NAME}>/cards

	COMMENT "Copying ImGui app resources"
)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-resources)
