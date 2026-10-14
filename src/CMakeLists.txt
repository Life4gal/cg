project(cg-core-1)

add_library(
	${PROJECT_NAME} STATIC

	# ===================================
	# DOMAIN
	# ===================================
	
	# ===================================
	# STATE
	# ===================================

	src/state/card.cpp
	src/state/field.cpp
	src/state/game.cpp
	
	# ===================================
	# FLOW
	# ===================================

	src/flow/turn.cpp
	src/flow/battle.cpp
	src/flow/priority.cpp
)

target_sources(
	${PROJECT_NAME} PUBLIC
	FILE_SET HEADERS
	BASE_DIRS include
	FILES
	
	# ===================================
	# UTILITY
	# ===================================

	include/cg/utility/enum.hpp
	include/cg/utility/flag.hpp
	include/cg/utility/functional.hpp
	include/cg/utility/opacity.hpp

	# ===================================
	# DOMAIN
	# ===================================

	include/cg/domain/card.hpp
	include/cg/domain/effect.hpp
	include/cg/domain/reason.hpp
	include/cg/domain/phase.hpp
	include/cg/domain/timing.hpp
	include/cg/domain/field.hpp
	include/cg/domain/player.hpp
	include/cg/domain/summon.hpp
	include/cg/domain/card_definition.hpp
	
	# ===================================
	# DRIVER
	# ===================================

	include/cg/driver/action.hpp
	include/cg/driver/request.hpp
	include/cg/driver/response.hpp
	include/cg/driver/driver.hpp

	# ===================================
	# STATE
	# ===================================

	include/cg/state/id.hpp
	include/cg/state/card.hpp
	include/cg/state/field.hpp
	include/cg/state/game.hpp
	
	# ===================================
	# FLOW
	# ===================================

	include/cg/flow/turn.hpp
	include/cg/flow/battle.hpp
	include/cg/flow/priority.hpp
)

target_include_directories(
	${PROJECT_NAME} PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_options(
	${PROJECT_NAME} 
	
	PRIVATE
	${CG_COMPILE_FLAGS}
)

target_compile_features(
	${PROJECT_NAME} 
	
	PUBLIC
	cxx_std_23
)

target_link_libraries(
	${PROJECT_NAME}

	PUBLIC
	nlohmann_json::nlohmann_json
	magic_enum::magic_enum

	PRIVATE
	libassert::assert
	spdlog::spdlog
)
