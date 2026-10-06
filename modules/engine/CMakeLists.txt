project(cg-engine)

add_library(
	${PROJECT_NAME} STATIC

	# ===================================
	# STATE
	# ===================================
	src/state/card.cpp
	src/state/zone.cpp
	src/state/state.cpp
	src/state/link.cpp

	# ===================================
	# SUMMON
	# ===================================
	src/summon/procedure.cpp
)

target_sources(
	${PROJECT_NAME} PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
	include/engine/opacity.hpp
	# ===================================
	# STATE
	# ===================================
	include/engine/state/id.hpp
	include/engine/state/handle.hpp
	include/engine/state/card.hpp
	include/engine/state/zone.hpp
	include/engine/state/state.hpp
	include/engine/state/link.hpp

	# ===================================
	# SUMMON
	# ===================================
	include/engine/summon/spec.hpp
	include/engine/summon/procedure.hpp

)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(
	${PROJECT_NAME} PRIVATE
	
	${CG_COMPILE_FLAGS}
)

target_compile_features(
	${PROJECT_NAME} PUBLIC
	cxx_std_23
)

target_link_libraries(
	${PROJECT_NAME}

	PUBLIC
	cg-domain

	PRIVATE
	libassert::assert
)
