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
	src/state/db.cpp
	
	# ===================================
	# DSL
	# ===================================

	src/dsl/archetype.cpp
	src/dsl/types.cpp
	src/dsl/effect.cpp
	src/dsl/vocabulary.cpp

	src/dsl/json/reader.cpp

	# ===================================
	# FLOW
	# ===================================

	src/flow/turn.cpp
	src/flow/battle.cpp
	src/flow/priority.cpp
	
	# ===================================
	# EXECUTION
	# ===================================

	src/execution/interpreter.cpp
	src/execution/script_registry.cpp
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
	include/cg/utility/hash.hpp

	# ===================================
	# DOMAIN
	# ===================================

	include/cg/domain/card.hpp
	include/cg/domain/effect.hpp
	include/cg/domain/reason.hpp
	include/cg/domain/phase.hpp
	include/cg/domain/timing.hpp
	include/cg/domain/event.hpp
	include/cg/domain/field.hpp
	include/cg/domain/player.hpp
	include/cg/domain/summon.hpp
	include/cg/domain/win.hpp
	include/cg/domain/reject_reason.hpp
	include/cg/domain/selection_context.hpp
	
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
	include/cg/server/server.hpp
	include/cg/server/room.hpp
	include/cg/server/net_server.hpp
	include/cg/server/native_server.hpp
		
	# ===================================
	# CODEC
	# ===================================

	include/cg/codec/codec.hpp
	include/cg/codec/domain.hpp
	include/cg/codec/dsl.hpp
	include/cg/codec/state.hpp
	include/cg/codec/action.hpp
	include/cg/codec/request.hpp
	include/cg/codec/response.hpp
	include/cg/codec/message.hpp

	# ===================================
	# STATE
	# ===================================

	include/cg/state/id.hpp
	include/cg/state/card_definition.hpp
	include/cg/state/card.hpp
	include/cg/state/field.hpp
	include/cg/state/game.hpp
	include/cg/state/db.hpp
	
	# ===================================
	# DSL
	# ===================================

	include/cg/dsl/id.hpp
	include/cg/dsl/archetype.hpp
	include/cg/dsl/identity.hpp
	include/cg/dsl/types.hpp
	include/cg/dsl/expression.hpp
	include/cg/dsl/statement.hpp
	include/cg/dsl/trigger.hpp
	include/cg/dsl/effect.hpp
	include/cg/dsl/summon_procedure.hpp
	include/cg/dsl/card_script.hpp
	include/cg/dsl/vocabulary.hpp
	include/cg/dsl/script.hpp

	include/cg/dsl/json/reader.hpp

	# ===================================
	# FLOW
	# ===================================

	include/cg/flow/turn.hpp
	include/cg/flow/battle.hpp
	include/cg/flow/priority.hpp
	
	# ===================================
	# EXECUTION
	# ===================================

	include/cg/execution/value.hpp
	include/cg/execution/effect.hpp
	include/cg/execution/context.hpp
	include/cg/execution/chain.hpp
	include/cg/execution/interpreter.hpp
	include/cg/execution/script_registry.hpp
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
