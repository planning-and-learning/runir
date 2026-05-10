find_package(Python 3.8 REQUIRED
    COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH})
message(STATUS "Found nanobind: ${nanobind_DIR} (found version ${nanobind_VERSION})")

set(PYRUNIR_PACKAGE_DIR "pyrunir")
set(PYRUNIR_NATIVE_DIR "${PYRUNIR_PACKAGE_DIR}/native")
set(PYRUNIR_NATIVE_LIBDIR "${PYRUNIR_NATIVE_DIR}/${CMAKE_INSTALL_LIBDIR}")

nanobind_add_module(pyrunir
    src/module.cpp
    src/pyrunir/graphs/module.cpp
    src/pyrunir/kr/dl/module.cpp
    src/pyrunir/kr/dl/semantics/datas.cpp
    src/pyrunir/kr/dl/semantics/indices.cpp
    src/pyrunir/kr/dl/semantics/module.cpp
    src/pyrunir/kr/dl/semantics/repositories.cpp
    src/pyrunir/kr/dl/semantics/views.cpp
    src/pyrunir/kr/dl/grammar/constructor_repository.cpp
    src/pyrunir/kr/dl/grammar/datas.cpp
    src/pyrunir/kr/dl/grammar/grammar.cpp
    src/pyrunir/kr/dl/grammar/indices.cpp
    src/pyrunir/kr/dl/grammar/module.cpp
    src/pyrunir/kr/dl/grammar/parser.cpp
    src/pyrunir/kr/dl/grammar/views.cpp
    src/pyrunir/kr/dl/cnf_grammar/constructor_repository.cpp
    src/pyrunir/kr/dl/cnf_grammar/datas.cpp
    src/pyrunir/kr/dl/cnf_grammar/generate.cpp
    src/pyrunir/kr/dl/cnf_grammar/grammar.cpp
    src/pyrunir/kr/dl/cnf_grammar/indices.cpp
    src/pyrunir/kr/dl/cnf_grammar/module.cpp
    src/pyrunir/kr/dl/cnf_grammar/translate.cpp
    src/pyrunir/kr/dl/cnf_grammar/views.cpp
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
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/pyrunir/kr/__init__.py"
        DESTINATION "${PYRUNIR_PACKAGE_DIR}/kr"
        COMPONENT pyrunir)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/pyrunir/kr/dl/__init__.py"
        DESTINATION "${PYRUNIR_PACKAGE_DIR}/kr/dl"
        COMPONENT pyrunir)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/pyrunir/kr/dl/semantics/__init__.py"
        DESTINATION "${PYRUNIR_PACKAGE_DIR}/kr/dl/semantics"
        COMPONENT pyrunir)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/pyrunir/kr/dl/grammar/__init__.py"
        DESTINATION "${PYRUNIR_PACKAGE_DIR}/kr/dl/grammar"
        COMPONENT pyrunir)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/pyrunir/kr/dl/cnf_grammar/__init__.py"
        DESTINATION "${PYRUNIR_PACKAGE_DIR}/kr/dl/cnf_grammar"
        COMPONENT pyrunir)

add_custom_target(pyrunir_wheel DEPENDS pyrunir)
