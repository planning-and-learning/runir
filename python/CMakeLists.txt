find_package(Python 3.8 REQUIRED
    COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH})
message(STATUS "Found nanobind: ${nanobind_DIR} (found version ${nanobind_VERSION})")

set(PYRUNIR_PACKAGE_DIR "pyrunir")
set(PYRUNIR_NATIVE_DIR "${PYRUNIR_PACKAGE_DIR}/native")
set(PYRUNIR_NATIVE_LIBDIR "${PYRUNIR_NATIVE_DIR}/${CMAKE_INSTALL_LIBDIR}")

nanobind_add_module(pyrunir
    src/bindings.cpp
    src/pyrunir/graphs/graphs.cpp
)

target_link_libraries(pyrunir PRIVATE runir::core runir::graphs)

if(BUILD_SHARED_LIBS)
    if(APPLE)
        set(PYRUNIR_LIBRARY_ORIGIN "@loader_path/native/${CMAKE_INSTALL_LIBDIR}")
    elseif(UNIX)
        set(PYRUNIR_LIBRARY_ORIGIN "$ORIGIN/native/${CMAKE_INSTALL_LIBDIR}")
    endif()

    if(PYRUNIR_LIBRARY_ORIGIN)
        set_target_properties(pyrunir PROPERTIES
            BUILD_RPATH "${PYRUNIR_LIBRARY_ORIGIN}"
            INSTALL_RPATH "${PYRUNIR_LIBRARY_ORIGIN}")
    endif()
endif()

install(TARGETS pyrunir DESTINATION "${PYRUNIR_PACKAGE_DIR}" COMPONENT pyrunir)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/pyrunir/__init__.py"
        DESTINATION "${PYRUNIR_PACKAGE_DIR}"
        COMPONENT pyrunir)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/pyrunir/graphs/__init__.py"
        DESTINATION "${PYRUNIR_PACKAGE_DIR}/graphs"
        COMPONENT pyrunir)

add_custom_target(pyrunir_wheel DEPENDS pyrunir)
