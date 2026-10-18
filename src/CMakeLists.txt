project(cg-core-1)

add_library(
	${PROJECT_NAME} STATIC

	# ===================================
	# SERVER
	# ===================================

	src/server/deck.cpp
	src/server/room.cpp
	src/server/net_server.cpp
	src/server/native_server.cpp

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
	include/cg/utility/type_list.hpp
	include/cg/utility/variant.hpp

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
	include/cg/domain/win.hpp
	include/cg/domain/reject_reason.hpp
	
	# ===================================
	# DRIVER
	# ===================================

	include/cg/driver/action.hpp
	include/cg/driver/request.hpp
	include/cg/driver/response.hpp
	include/cg/driver/driver.hpp
	
	# ===================================
	# SERVER
	# ===================================

	include/cg/server/deck.hpp
	include/cg/server/message.hpp
	include/cg/server/codec.hpp
	include/cg/server/server.hpp
	include/cg/server/room.hpp
	include/cg/server/net_server.hpp
	include/cg/server/native_server.hpp

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

# codec.hpp/nlohmann/variant组合会生成大量section,MSVC需要/bigobj
if (CG_COMPILER_MSVC OR CG_COMPILER_CLANG_CL)
	target_compile_options(
		${PROJECT_NAME}

		PRIVATE
		"/bigobj"
	)
endif (CG_COMPILER_MSVC OR CG_COMPILER_CLANG_CL)

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
	ixwebsocket::ixwebsocket
)
