cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml" RUNIR_PROJECT_VERSION_LINE REGEX "^version[ \t]*=" LIMIT_COUNT 1)
if(NOT RUNIR_PROJECT_VERSION_LINE)
    message(FATAL_ERROR "Could not read project version from pyproject.toml")
endif()
string(REGEX REPLACE "^version[ \t]*=[ \t]*\"([^\"]+)\".*$" "\\1" RUNIR_PROJECT_VERSION "${RUNIR_PROJECT_VERSION_LINE}")

project(runir VERSION "${RUNIR_PROJECT_VERSION}" LANGUAGES CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -Wuninitialized -pedantic -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_DEBUG "-O3 -DDEBUG")

set(default_build_type "Debug")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}', as none was specified.")
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
endif()
message(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)

option(RUNIR_BUILD_TESTS "Enables compilation of runir tests." OFF)
if(RUNIR_BUILD_TESTS)
    message("Build tests enabled.")
    enable_testing()
else()
    message("Build tests disabled.")
endif()

option(RUNIR_BUILD_SHARED "Build runir native libraries as shared libraries." ON)
set(BUILD_SHARED_LIBS ${RUNIR_BUILD_SHARED})
if(RUNIR_BUILD_SHARED)
    message("Build shared libraries enabled.")
else()
    message("Build shared libraries disabled.")
endif()

option(RUNIR_LINK_STATIC_DEPENDENCIES "Prefer static dependency libraries when finding installed dependencies." OFF)
option(RUNIR_ENABLE_FMT_FORMATTERS "Enable Runir's public fmt::formatter specializations." ON)
if(RUNIR_ENABLE_FMT_FORMATTERS)
    add_compile_definitions(RUNIR_ENABLE_FMT_FORMATTERS=1)
else()
    add_compile_definitions(RUNIR_ENABLE_FMT_FORMATTERS=0)
endif()

option(RUNIR_BUILD_EXECUTABLES "Build runir executables." OFF)
option(RUNIR_BUILD_PYRUNIR "Build pyrunir Python bindings." OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include("configure_yggdrasil")
include("configure_pypddl")
include("configure_tyr")
configure_yggdrasil()
configure_pypddl()
configure_tyr()

find_package(fmt CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(fmt_FOUND)
    message(STATUS "Found fmt: ${fmt_DIR} (found version ${fmt_VERSION})")
endif()

find_package(TBB CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(TBB_FOUND)
    include_directories(${TBB_INCLUDE_DIRS})
    message(STATUS "Found TBB: ${TBB_DIR} (found version ${TBB_VERSION})")
endif()

find_package(tyr 0.0.13 CONFIG REQUIRED COMPONENTS core PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(tyr_FOUND)
    message(STATUS "Found tyr: ${tyr_DIR} (found version ${tyr_VERSION})")
endif()

add_subdirectory(src)

if(RUNIR_BUILD_EXECUTABLES)
    find_package(argparse CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
    if(argparse_FOUND)
        message(STATUS "Found argparse: ${argparse_DIR} (found version ${argparse_VERSION})")
    endif()
    add_subdirectory(exe)
endif()

if(RUNIR_BUILD_PYRUNIR)
    add_subdirectory(python/src)
endif()

if(RUNIR_BUILD_TESTS)
    add_subdirectory(tests)
endif()

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/runir"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT runir)

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/runir/cmake"
    COMPONENT runir
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/runirConfigVersion.cmake"
    VERSION ${runir_VERSION}
    COMPATIBILITY ExactVersion
)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/runirConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/runir"
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/runirConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/runirConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/runir"
    COMPONENT runir
)
