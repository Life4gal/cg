cmake_minimum_required(VERSION 3.25)

# set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(VCPKG_ROOT "C:/workspace/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

set(
	CG_MAJOR_VERSION
	1
	CACHE STRING
	"CardGame major version"
)
set(
	CG_MINOR_VERSION
	0
	CACHE STRING
	"CardGame minor version"
)
set(
	CG_PATCH_VERSION
	0
	CACHE STRING
	"CardGame patch version"
)
set(
	CG_VERSION
	"${CG_MAJOR_VERSION}.${CG_MINOR_VERSION}.${CG_PATCH_VERSION}"
	CACHE STRING
	"CardGame version"
)

project(
	cg
	VERSION ${CG_VERSION}
	DESCRIPTION "CardGame"
	HOMEPAGE_URL "https://github.com/Life4gal/cg"
	LANGUAGES CXX
)

# ===================================================================================================
# DEFINES

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/defines.cmake)

# ===================================================================================================
# DEPENDENCIES

#find_package(xxx CONFIG REQUIRED)

# ===================================================================================================
# LIBRARY

# cg-core
add_subdirectory(core)

# ===================================================================================================
# EXECUTABLE

add_executable(
	cg
	
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_compile_options(
	cg
	PUBLIC
	
	${CG_COMPILE_FLAGS}
)

target_compile_features(
	cg
	PRIVATE
	cxx_std_23
)

target_link_libraries(
	cg
	PRIVATE
	
	cg-core
)
