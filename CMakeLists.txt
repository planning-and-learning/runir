cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/read_project_version.cmake")
runir_read_project_version(RUNIR_PROJECT_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml")

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

message("Build shared libraries enabled.")

option(RUNIR_ENABLE_FMT_FORMATTERS "Enable Runir's public fmt::formatter specializations." ON)
if(RUNIR_ENABLE_FMT_FORMATTERS)
    add_compile_definitions(RUNIR_ENABLE_FMT_FORMATTERS=1 TYR_ENABLE_FMT_FORMATTERS=1 LOKI_ENABLE_FMT_FORMATTERS=1 YGG_ENABLE_FMT_FORMATTERS=1)
else()
    add_compile_definitions(RUNIR_ENABLE_FMT_FORMATTERS=0 TYR_ENABLE_FMT_FORMATTERS=0 LOKI_ENABLE_FMT_FORMATTERS=0 YGG_ENABLE_FMT_FORMATTERS=0)
endif()

option(RUNIR_HEADER_INSTANTIATION "Enable template definitions in public headers at higher compile time costs." OFF)
if(RUNIR_HEADER_INSTANTIATION)
    add_compile_definitions(RUNIR_HEADER_INSTANTIATION)
endif()

option(RUNIR_BUILD_EXECUTABLES "Build runir executables." OFF)
option(RUNIR_BUILD_PYRUNIR "Build pyrunir Python bindings." OFF)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include("${PROJECT_SOURCE_DIR}/cmake/bootstrap_pyyggdrasil.cmake")

find_package(yggdrasil 0.0.21 CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(yggdrasil_FOUND)
    message(STATUS "Found yggdrasil: ${yggdrasil_DIR} (found version ${yggdrasil_VERSION})")
endif()

yggdrasil_find_python_native_prefix(pypddl PYPDDL_NATIVE_PREFIX)
yggdrasil_find_python_native_prefix(pytyr TYR_NATIVE_PREFIX)
yggdrasil_register_python_native_runtime_prefix("pypddl/native" "${PYPDDL_NATIVE_PREFIX}")
yggdrasil_register_python_native_runtime_prefix("pytyr/native" "${TYR_NATIVE_PREFIX}")
yggdrasil_register_python_native_runtime_prefix("pyyggdrasil" "${YGGDRASIL_NATIVE_PREFIX}")

find_package(fmt CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(fmt_FOUND)
    message(STATUS "Found fmt: ${fmt_DIR} (found version ${fmt_VERSION})")
endif()

find_package(TBB CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(TBB_FOUND)
    include_directories(${TBB_INCLUDE_DIRS})
    message(STATUS "Found TBB: ${TBB_DIR} (found version ${TBB_VERSION})")
endif()

find_package(tyr 0.0.27 CONFIG REQUIRED COMPONENTS core PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(tyr_FOUND)
    message(STATUS "Found tyr: ${tyr_DIR} (found version ${tyr_VERSION})")
endif()


if(RUNIR_BUILD_TESTS OR RUNIR_BUILD_EXECUTABLES)
    # Benchmark PDDL comes from the pypddl-datasets cache, resolved (and
    # downloaded on demand) at configure time.
    if(NOT Python3_EXECUTABLE)
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
    endif()
    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -c "import pypddl_datasets, sys; sys.stdout.write(str(pypddl_datasets.data_root()))"
        OUTPUT_VARIABLE RUNIR_BENCHMARKS_DIR
        RESULT_VARIABLE RUNIR_BENCHMARKS_RESULT)
    if(NOT RUNIR_BENCHMARKS_RESULT EQUAL 0)
        message(FATAL_ERROR "Could not resolve the benchmark data root; install 'pypddl-datasets>=0.0.5,<0.1' into the configuring Python (${Python3_EXECUTABLE}).")
    endif()
    message(STATUS "Benchmark data: ${RUNIR_BENCHMARKS_DIR}")
    add_compile_definitions(BENCHMARKS_DIR="${RUNIR_BENCHMARKS_DIR}")
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
    COMPATIBILITY SameMinorVersion
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
