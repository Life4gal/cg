project(cg-domain)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(
	${PROJECT_NAME} INTERFACE
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
	include/domain/card.hpp
	include/domain/effect.hpp
	include/domain/event.hpp
	include/domain/phase.hpp
	include/domain/zone.hpp
	include/domain/player.hpp
	include/domain/summon.hpp
	include/domain/chain.hpp
)

target_include_directories(
	${PROJECT_NAME} INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(
	${PROJECT_NAME} INTERFACE
	
	${CG_COMPILE_FLAGS}
)

target_compile_features(
	${PROJECT_NAME} INTERFACE
	cxx_std_23
)
