cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml" TEMPLATE_FEATURE_PROJECT_VERSION_LINE REGEX "^version[ \t]*=" LIMIT_COUNT 1)
if(NOT TEMPLATE_FEATURE_PROJECT_VERSION_LINE)
    message(FATAL_ERROR "Could not read project version from pyproject.toml")
endif()
string(REGEX REPLACE "^version[ \t]*=[ \t]*\"([^\"]+)\".*$" "\\1" TEMPLATE_FEATURE_PROJECT_VERSION "${TEMPLATE_FEATURE_PROJECT_VERSION_LINE}")

project(template_feature VERSION "${TEMPLATE_FEATURE_PROJECT_VERSION}" LANGUAGES CXX)

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

option(TEMPLATE_FEATURE_BUILD_TESTS "Enables compilation of template_feature tests." OFF)
if(TEMPLATE_FEATURE_BUILD_TESTS)
    message("Build tests enabled.")
    enable_testing()
else()
    message("Build tests disabled.")
endif()

option(BUILD_SHARED_LIBS "Build shared libraries." ON)
if(BUILD_SHARED_LIBS)
    message("Build shared libraries enabled.")
else()
    message("Build shared libraries disabled.")
endif()

option(TEMPLATE_FEATURE_LINK_STATIC_DEPENDENCIES "Prefer static dependency libraries when finding installed dependencies." OFF)
option(BUILD_PYTEMPLATE_FEATURE "Build template_feature for the pytemplate_feature Python wheel." OFF)

if(BUILD_PYTEMPLATE_FEATURE)
    set(TEMPLATE_FEATURE_INSTALL_PREFIX "pytemplate_feature/native/")
    set(TEMPLATE_FEATURE_INSTALL_COMPONENT pytemplate_feature)
    set(TEMPLATE_FEATURE_INSTALL_INTERFACE_INCLUDEDIR "${TEMPLATE_FEATURE_INSTALL_PREFIX}${CMAKE_INSTALL_INCLUDEDIR}")
else()
    set(TEMPLATE_FEATURE_INSTALL_PREFIX "")
    set(TEMPLATE_FEATURE_INSTALL_COMPONENT template_feature)
    set(TEMPLATE_FEATURE_INSTALL_INTERFACE_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
endif()

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include("configure_yggdrasil")
include("configure_tyr")
configure_yggdrasil()
configure_tyr()

find_package(fmt CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(fmt_FOUND)
    message(STATUS "Found fmt: ${fmt_DIR} (found version ${fmt_VERSION})")
endif()

find_package(tyr CONFIG REQUIRED COMPONENTS core PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
if(tyr_FOUND)
    message(STATUS "Found tyr: ${tyr_DIR} (found version ${tyr_VERSION})")
endif()

add_subdirectory(src)

if(BUILD_PYTEMPLATE_FEATURE)
    add_subdirectory(python)
endif()

if(TEMPLATE_FEATURE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/template_feature"
    DESTINATION "${TEMPLATE_FEATURE_INSTALL_PREFIX}${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT ${TEMPLATE_FEATURE_INSTALL_COMPONENT})

install(
    DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
    DESTINATION "${TEMPLATE_FEATURE_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}/cmake/template_feature/cmake"
    COMPONENT ${TEMPLATE_FEATURE_INSTALL_COMPONENT}
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/template_featureConfigVersion.cmake"
    VERSION ${template_feature_VERSION}
    COMPATIBILITY ExactVersion
)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/template_featureConfig.cmake"
    INSTALL_DESTINATION "${TEMPLATE_FEATURE_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}/cmake/template_feature"
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/template_featureConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/template_featureConfigVersion.cmake"
    DESTINATION "${TEMPLATE_FEATURE_INSTALL_PREFIX}${CMAKE_INSTALL_LIBDIR}/cmake/template_feature"
    COMPONENT ${TEMPLATE_FEATURE_INSTALL_COMPONENT}
)
