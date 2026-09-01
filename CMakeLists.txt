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

# spdlog
find_package(spdlog CONFIG REQUIRED)

# doctest
find_package(doctest CONFIG REQUIRED)

# python & pybind11
#1> [CMake] -- Found Python: C:/workspace/vcpkg/installed/x64-windows/tools/python3/python.exe (found suitable version "3.12.9", minimum required is "3.9") found components: Interpreter Development.Module Development.Embed
set(PYBIND11_FINDPYTHON OFF)
set(Python_ROOT_DIR "C:/Program Files/Python314")
set(Python_EXECUTABLE "${Python_ROOT_DIR}/python.exe" CACHE FILEPATH "" FORCE)
set(Python_INCLUDE_DIR "${Python_ROOT_DIR}/include" CACHE PATH "" FORCE)
set(Python_LIBRARY "${Python_ROOT_DIR}/libs/python314.lib" CACHE FILEPATH "" FORCE)
find_package(pybind11 CONFIG REQUIRED)

# ===================================================================================================
# LIBRARY

# cg-core
add_subdirectory(core)

# ===================================================================================================
# TEST

add_subdirectory(tests)

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

# ===================================================================================================
# SCRIPTS

add_custom_target(
	COPY_SCRIPTS
	ALL
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CMAKE_SOURCE_DIR}/scripts scripts
)

add_dependencies(cg-tests COPY_SCRIPTS)
add_dependencies(cg COPY_SCRIPTS)
