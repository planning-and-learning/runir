cmake_minimum_required(VERSION 3.21)

project(downstream_runir_user LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python 3.8 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(nanobind CONFIG REQUIRED)
find_package(runir CONFIG REQUIRED COMPONENTS core)

nanobind_add_module(downstream_runir_user
    src/downstream_runir_user.cpp
)

target_link_libraries(downstream_runir_user PRIVATE runir::core)

if(DOWNSTREAM_RUNIR_RUNTIME_LIBRARY_DIR)
    if(APPLE)
        set_target_properties(downstream_runir_user PROPERTIES
            BUILD_RPATH "${DOWNSTREAM_RUNIR_RUNTIME_LIBRARY_DIR}"
            INSTALL_RPATH "@loader_path")
    elseif(UNIX)
        set_target_properties(downstream_runir_user PROPERTIES
            BUILD_RPATH "${DOWNSTREAM_RUNIR_RUNTIME_LIBRARY_DIR}"
            INSTALL_RPATH "$ORIGIN")
    endif()
endif()

install(TARGETS downstream_runir_user DESTINATION src)

