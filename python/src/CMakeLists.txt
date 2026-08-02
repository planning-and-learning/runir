find_package(
    Python 3.9 REQUIRED
    COMPONENTS Interpreter Development.Module
    OPTIONAL_COMPONENTS Development.SABIModule
)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
message(STATUS "Found nanobind: ${nanobind_DIR} (found version ${nanobind_VERSION})")

yggdrasil_python_package_init(PYRUNIR pyrunir)

file(
    GLOB_RECURSE PYRUNIR_KR_SOURCES
    CONFIGURE_DEPENDS
    "pyrunir/kr/*.cpp"
)

nanobind_add_module(
    _pyrunir NB_SHARED
    module.cpp
    pyrunir/module.cpp
    pyrunir/datasets/config.cpp
    pyrunir/datasets/equivalence_graph.cpp
    pyrunir/datasets/module.cpp
    pyrunir/datasets/state_graph.cpp
    pyrunir/datasets/task_class.cpp
    pyrunir/graphs/module.cpp
    ${PYRUNIR_KR_SOURCES}
)

target_link_libraries(_pyrunir PRIVATE runir::core)
target_include_directories(_pyrunir PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

yggdrasil_python_module_rpaths(PYRUNIR _pyrunir)

install(TARGETS _pyrunir DESTINATION "pyrunir" COMPONENT pyrunir)

foreach(runir_target IN ITEMS core datasets graphs kr)
    set_property(
        TARGET ${runir_target} PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
    )
endforeach()

yggdrasil_install_native_export(
    PKG_UPPER PYRUNIR
    PACKAGE pyrunir
    CONFIG_NAME runir
    NAMESPACE runir::
    EXPORT_NAME pyrunirRunirTargets
    EXPORT_FILE runirTargets.cmake
    TARGETS core datasets graphs kr
    HEADER_DIRS "${CMAKE_SOURCE_DIR}/include/runir"
    CONFIG_TEMPLATE "${CMAKE_SOURCE_DIR}/Config.cmake.in"
    VERSION_FILE "${CMAKE_BINARY_DIR}/runirConfigVersion.cmake"
    CMAKE_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake"
)

yggdrasil_install_runtime_path_fixup(PYRUNIR pyrunir)

set(PYRUNIR_GENERATED_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/pyrunir/_version.py")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pyrunir")
file(WRITE "${PYRUNIR_GENERATED_VERSION_FILE}" "__version__ = \"${PROJECT_VERSION}\"\n")
install(
    FILES "${PYRUNIR_GENERATED_VERSION_FILE}"
    DESTINATION "pyrunir"
    COMPONENT pyrunir
)

yggdrasil_install_python_files(
    pyrunir "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        pyrunir/__init__.py
        pyrunir/__main__.py
        pyrunir/__init__.pyi
        pyrunir/py.typed
        pyrunir/datasets/__init__.py
        pyrunir/graphs/__init__.py
        pyrunir/kr/__init__.py
        pyrunir/kr/dl/__init__.py
        pyrunir/kr/dl/base/__init__.py
        pyrunir/kr/dl/ext/__init__.py
        pyrunir/kr/dl/ext/semantics/__init__.py
        pyrunir/kr/dl/base/semantics/__init__.py
        pyrunir/kr/dl/base/grammar/__init__.py
        pyrunir/kr/dl/base/cnf_grammar/__init__.py
        pyrunir/kr/ps/__init__.py
        pyrunir/kr/ps/base/__init__.py
        pyrunir/kr/ps/ext/__init__.py
        pyrunir/kr/ps/ext/dl/__init__.py
        pyrunir/kr/ps/base/dl/__init__.py
        pyrunir/kr/dl/uns/__init__.py
        pyrunir/kr/dl/uns/semantics/__init__.py
        pyrunir/kr/uns/__init__.py
        pyrunir/kr/uns/dl/__init__.py
)

yggdrasil_install_provider_env(
    PYRUNIR pyrunir
    PREFIXES "${PYPDDL_NATIVE_PREFIX}" "${TYR_NATIVE_PREFIX}" "${YGGDRASIL_NATIVE_PREFIX}"
)

yggdrasil_install_python_stubs(
    PYRUNIR pyrunir
    MODULE pyrunir._pyrunir
    PRIVATE_MODULE _pyrunir
    RENAME_PACKAGES pyrunir pytyr pypddl pyyggdrasil
)

add_custom_target(pyrunir_wheel DEPENDS _pyrunir)
