cmake_minimum_required(VERSION 3.21)

project(downstream_template_user LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python 3.8 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(nanobind CONFIG REQUIRED)
find_package(template_feature CONFIG REQUIRED COMPONENTS core)

nanobind_add_module(downstream_template_user
    src/downstream_template_user.cpp
)

target_link_libraries(downstream_template_user PRIVATE template_feature::core)

if(DOWNSTREAM_TEMPLATE_FEATURE_RUNTIME_LIBRARY_DIR)
    if(APPLE)
        set_target_properties(downstream_template_user PROPERTIES
            BUILD_RPATH "${DOWNSTREAM_TEMPLATE_FEATURE_RUNTIME_LIBRARY_DIR}"
            INSTALL_RPATH "@loader_path")
    elseif(UNIX)
        set_target_properties(downstream_template_user PROPERTIES
            BUILD_RPATH "${DOWNSTREAM_TEMPLATE_FEATURE_RUNTIME_LIBRARY_DIR}"
            INSTALL_RPATH "$ORIGIN")
    endif()
endif()

install(TARGETS downstream_template_user DESTINATION src)

