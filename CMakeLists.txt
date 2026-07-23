cmake_minimum_required(VERSION 3.21)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/read_project_version.cmake")
runir_read_project_version(RUNIR_PROJECT_VERSION "${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml")

project(runir VERSION "${RUNIR_PROJECT_VERSION}" LANGUAGES CXX)

# Project configuration

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

# Options

option(RUNIR_USE_LLD "Use LLVM lld with Clang when available." ON)
option(RUNIR_ENABLE_LTO "Enable link-time optimization for Release builds." ON)
option(
    RUNIR_HEADER_INSTANTIATION
    "Instantiate templates in in-tree translation units at higher compile time cost."
    OFF
)
option(RUNIR_BUILD_PYRUNIR "Build pyrunir Python bindings." OFF)
option(RUNIR_BUILD_TESTS "Build Runir tests." OFF)
option(RUNIR_BUILD_EXECUTABLES "Build Runir executables." OFF)

# Build configuration

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(
        CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE
    )
endif()

add_compile_definitions(YGG_ENABLE_FMT_FORMATTERS=1)

if(RUNIR_HEADER_INSTANTIATION)
    add_compile_definitions(RUNIR_HEADER_INSTANTIATION)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W1 /EHsc /bigobj /MP")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " /IGNORE:4006,4044,4075")
else()
    # TODO: Add -Wextra and fix all warnings

    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    add_compile_options(-Wall)

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -g2 -ggdb")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g2 -fno-omit-frame-pointer -fno-lto")

    if(
        RUNIR_USE_LLD
        AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    )
        find_program(LLD_LINKER NAMES ld.lld)
        if(LLD_LINKER)
            message(STATUS "Using lld: ${LLD_LINKER}")
            add_link_options(-fuse-ld=lld)
        else()
            message(STATUS "lld not found, falling back to default linker")
        endif()
    elseif(RUNIR_USE_LLD AND RUNIR_ENABLE_LTO AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "Skipping lld with GNU because GCC LTO requires the GCC linker plugin")
    endif()

    if(RUNIR_ENABLE_LTO AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # Cap release LTO parallelism to reduce RAM spikes.
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=2")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto=2")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -flto=2")
        set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -flto=2")
    elseif(
        RUNIR_ENABLE_LTO
        AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    )
        # ThinLTO reduces release compile and link costs compared with full LTO.
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=thin")
        if(APPLE)
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-mllvm,-threads=2")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,-mllvm,-threads=2")
            set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -Wl,-mllvm,-threads=2")
        else()
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--thinlto-jobs=2")
            set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -Wl,--thinlto-jobs=2")
            set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} -Wl,--thinlto-jobs=2")
        endif()
    endif()
endif()

if(CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Build configurations: ${CMAKE_CONFIGURATION_TYPES}")
else()
    message(STATUS "Build configuration: ${CMAKE_BUILD_TYPE}")
endif()

# Dependencies

set(RUNIR_YGGDRASIL_REQUIRED_VERSION 0.0.25)
set(RUNIR_TYR_REQUIRED_VERSION 0.0.32)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

include("${PROJECT_SOURCE_DIR}/cmake/bootstrap_pyyggdrasil.cmake")

find_package(
    yggdrasil ${RUNIR_YGGDRASIL_REQUIRED_VERSION}
    CONFIG REQUIRED
    PATHS ${CMAKE_PREFIX_PATH}
    NO_DEFAULT_PATH
)
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

find_package(
    tyr ${RUNIR_TYR_REQUIRED_VERSION}
    CONFIG REQUIRED COMPONENTS core
    PATHS ${CMAKE_PREFIX_PATH}
    NO_DEFAULT_PATH
)
if(tyr_FOUND)
    message(STATUS "Found tyr: ${tyr_DIR} (found version ${tyr_VERSION})")
endif()

# Benchmark setup

if(RUNIR_BUILD_TESTS OR RUNIR_BUILD_EXECUTABLES)
    # Benchmark PDDL comes from the pypddl-datasets cache, resolved (and
    # downloaded on demand) at configure time.
    find_package(Python 3.10 REQUIRED COMPONENTS Interpreter)
    execute_process(
        COMMAND
            "${Python_EXECUTABLE}"
            -c
            "import pypddl_datasets, sys; sys.stdout.write(str(pypddl_datasets.data_root()))"
        OUTPUT_VARIABLE RUNIR_BENCHMARKS_DIR
        RESULT_VARIABLE RUNIR_BENCHMARKS_RESULT
    )
    if(NOT RUNIR_BENCHMARKS_RESULT EQUAL 0)
        message(
            FATAL_ERROR
            "Could not resolve the benchmark data root; install 'pypddl-datasets>=0.0.9,<0.1' into the configuring Python (${Python_EXECUTABLE})."
        )
    endif()
    message(STATUS "Benchmark data: ${RUNIR_BENCHMARKS_DIR}")
    add_compile_definitions(BENCHMARKS_DIR="${RUNIR_BENCHMARKS_DIR}")
endif()

# Targets

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
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation and package export

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/runir"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT runir
)

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

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
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
