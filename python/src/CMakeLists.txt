find_package(
    Python 3.11 REQUIRED
    COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule
)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
message(STATUS "Found nanobind: ${nanobind_DIR} (found version ${nanobind_VERSION})")

brokkr_python_package_init(PYRUNIR pyrunir)

file(
    GLOB_RECURSE PYRUNIR_KR_SOURCES
    CONFIGURE_DEPENDS
    "pyrunir/kr/*.cpp"
)

file(
    GLOB_RECURSE PYRUNIR_SERIALIZATION_SOURCES
    CONFIGURE_DEPENDS
    "pyrunir/serialization/*.cpp"
)

nanobind_add_module(
    _pyrunir
    NB_SUPPRESS_WARNINGS
    module.cpp
    pyrunir/module.cpp
    pyrunir/datasets/config.cpp
    pyrunir/datasets/equivalence_graph.cpp
    pyrunir/datasets/module.cpp
    pyrunir/datasets/state_graph.cpp
    pyrunir/datasets/task_class.cpp
    ${PYRUNIR_SERIALIZATION_SOURCES}
    ${PYRUNIR_KR_SOURCES}
    BACKEND_MODULE nanobind_backend
)

target_link_libraries(_pyrunir PRIVATE runir::core)
target_include_directories(_pyrunir PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

brokkr_python_module_rpaths(PYRUNIR _pyrunir)

install(TARGETS _pyrunir DESTINATION "pyrunir" COMPONENT pyrunir)

foreach(runir_target IN ITEMS core datasets kr)
    set_property(
        TARGET ${runir_target} PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
    )
endforeach()

brokkr_install_native_export(
    PKG_UPPER PYRUNIR
    PACKAGE pyrunir
    CONFIG_NAME runir
    NAMESPACE runir::
    EXPORT_NAME pyrunirRunirTargets
    EXPORT_FILE runirTargets.cmake
    TARGETS core datasets kr
    HEADER_DIRS "${CMAKE_SOURCE_DIR}/include/runir"
    CONFIG_TEMPLATE "${CMAKE_SOURCE_DIR}/Config.cmake.in"
    VERSION_FILE "${CMAKE_BINARY_DIR}/runirConfigVersion.cmake"
)

brokkr_install_runtime_path_fixup(PYRUNIR pyrunir)

set(PYRUNIR_GENERATED_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/pyrunir/_version.py")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pyrunir")
file(WRITE "${PYRUNIR_GENERATED_VERSION_FILE}" "__version__ = \"${PROJECT_VERSION}\"\n")
install(
    FILES "${PYRUNIR_GENERATED_VERSION_FILE}"
    DESTINATION "pyrunir"
    COMPONENT pyrunir
)

brokkr_install_python_files(
    pyrunir "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        pyrunir/__init__.py
        pyrunir/__main__.py
        pyrunir/__init__.pyi
        pyrunir/py.typed
        pyrunir/datasets/__init__.py
        pyrunir/kr/__init__.py
        pyrunir/kr/dl/__init__.py
        pyrunir/kr/dl/semantics/__init__.py
        pyrunir/kr/dl/grammar/__init__.py
        pyrunir/kr/dl/cnf_grammar/__init__.py
        pyrunir/kr/ps/__init__.py
        pyrunir/kr/ps/base/__init__.py
        pyrunir/kr/ps/icp/__init__.py
        pyrunir/kr/ps/icp/dl/__init__.py
        pyrunir/kr/ps/ext/__init__.py
        pyrunir/kr/ps/ext/dl/__init__.py
        pyrunir/kr/ps/ext/explore/__init__.py
        pyrunir/kr/ps/ext/explore/graph.py
        pyrunir/kr/ps/ext/explore/results.py
        pyrunir/kr/ps/ext/explore/session.py
        pyrunir/kr/ps/ext/explore/server.py
        pyrunir/kr/ps/base/dl/__init__.py
        pyrunir/kr/uns/__init__.py
        pyrunir/kr/uns/dl/__init__.py
        pyrunir/serialization/__init__.py
)

brokkr_install_provider_env(
    PYRUNIR pyrunir
    PREFIXES "${LOKI_NATIVE_PREFIX}" "${TYR_NATIVE_PREFIX}" "${YGGDRASIL_NATIVE_PREFIX}" "${URD_NATIVE_PREFIX}" "${VEFR_NATIVE_PREFIX}"
)

brokkr_install_python_stubs(
    PYRUNIR pyrunir
    MODULE pyrunir._pyrunir
    PRIVATE_MODULE _pyrunir
    RENAME_PACKAGES pyrunir pytyr pypddl pyyggdrasil pyvefr
)

# pyrunir/kr/__init__.py re-exports pyyggdrasil's ParseError and SemanticError. nanobind's stub
# generator leaves out names that another package defines, so the installed stub, which type checkers
# read instead of __init__.py, gets the same re-export (once: an existing stub is kept on reinstall).
install(CODE [[
    set(pyrunir_kr_stub "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/pyrunir/kr/__init__.pyi")
    set(pyrunir_reexport "from pyyggdrasil.diagnostics import ParseError as ParseError, SemanticError as SemanticError")
    file(READ "${pyrunir_kr_stub}" pyrunir_kr_stub_content)
    string(FIND "${pyrunir_kr_stub_content}" "${pyrunir_reexport}" pyrunir_reexport_position)
    if(pyrunir_reexport_position EQUAL -1)
        file(APPEND "${pyrunir_kr_stub}" "\n${pyrunir_reexport}\n")
    endif()
]] COMPONENT pyrunir)

add_custom_target(pyrunir_wheel DEPENDS _pyrunir)
