find_package(Python 3.9 REQUIRED
  COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
message(STATUS "Found nanobind: ${nanobind_DIR} (found version ${nanobind_VERSION})")

yggdrasil_python_package_init(PYRUNIR pyrunir)

nanobind_add_module(_pyrunir NB_SHARED
    module.cpp
    pyrunir/module.cpp
    pyrunir/datasets/config.cpp
    pyrunir/datasets/equivalence_graph.cpp
    pyrunir/datasets/module.cpp
    pyrunir/datasets/state_graph.cpp
    pyrunir/datasets/task_class.cpp
    pyrunir/graphs/module.cpp
    pyrunir/kr/module.cpp
    pyrunir/kr/dl/module.cpp
    pyrunir/kr/dl/base/module.cpp
    pyrunir/kr/dl/base/parser.cpp
    pyrunir/kr/dl/ext/datas.cpp
    pyrunir/kr/dl/ext/indices.cpp
    pyrunir/kr/dl/ext/module.cpp
    pyrunir/kr/dl/ext/parser.cpp
    pyrunir/kr/dl/ext/repository.cpp
    pyrunir/kr/dl/ext/semantics/datas.cpp
    pyrunir/kr/dl/ext/semantics/indices.cpp
    pyrunir/kr/dl/ext/semantics/module.cpp
    pyrunir/kr/dl/ext/semantics/repositories.cpp
    pyrunir/kr/dl/ext/semantics/views.cpp
    pyrunir/kr/ps/base/module.cpp
    pyrunir/kr/ps/base/dl/indices.cpp
    pyrunir/kr/ps/base/dl/module.cpp
    pyrunir/kr/ps/base/dl/parser.cpp
    pyrunir/kr/ps/base/dl/sketch_factory.cpp
    pyrunir/kr/ps/base/dl/structural_termination.cpp
    pyrunir/kr/ps/base/dl/views.cpp
    pyrunir/kr/ps/base/indices.cpp
    pyrunir/kr/ps/module.cpp
    pyrunir/kr/ps/base/sketch_executor.cpp
    pyrunir/kr/ps/base/repository.cpp
    pyrunir/kr/ps/base/syntactic_complexity.cpp
    pyrunir/kr/ps/base/views.cpp
    pyrunir/kr/ps/ext/datas.cpp
    pyrunir/kr/ps/ext/indices.cpp
    pyrunir/kr/ps/ext/module.cpp
    pyrunir/kr/ps/ext/module_factory.cpp
    pyrunir/kr/ps/ext/parser.cpp
    pyrunir/kr/ps/ext/structural_termination.cpp
    pyrunir/kr/ps/ext/repository.cpp
    pyrunir/kr/ps/ext/module_program_executor.cpp
    pyrunir/kr/ps/ext/views.cpp
    pyrunir/kr/dl/base/semantics/datas.cpp
    pyrunir/kr/dl/base/semantics/indices.cpp
    pyrunir/kr/dl/base/semantics/module.cpp
    pyrunir/kr/dl/base/semantics/repositories.cpp
    pyrunir/kr/dl/base/semantics/views.cpp
    pyrunir/kr/dl/base/grammar/constructor_repository.cpp
    pyrunir/kr/dl/base/grammar/datas.cpp
    pyrunir/kr/dl/base/grammar/grammar.cpp
    pyrunir/kr/dl/base/grammar/grammar_factory.cpp
    pyrunir/kr/dl/base/grammar/indices.cpp
    pyrunir/kr/dl/base/grammar/module.cpp
    pyrunir/kr/dl/base/grammar/views.cpp
    pyrunir/kr/dl/base/cnf_grammar/constructor_repository.cpp
    pyrunir/kr/dl/base/cnf_grammar/datas.cpp
    pyrunir/kr/dl/base/cnf_grammar/generate.cpp
    pyrunir/kr/dl/base/cnf_grammar/grammar.cpp
    pyrunir/kr/dl/base/cnf_grammar/indices.cpp
    pyrunir/kr/dl/base/cnf_grammar/module.cpp
    pyrunir/kr/dl/base/cnf_grammar/translate.cpp
    pyrunir/kr/dl/base/cnf_grammar/views.cpp
    pyrunir/kr/dl/uns/module.cpp
    pyrunir/kr/dl/uns/indices.cpp
    pyrunir/kr/dl/uns/parser.cpp
    pyrunir/kr/dl/uns/repository.cpp
    pyrunir/kr/dl/uns/semantics/module.cpp
    pyrunir/kr/dl/uns/semantics/indices.cpp
    pyrunir/kr/dl/uns/semantics/datas.cpp
    pyrunir/kr/dl/uns/semantics/views.cpp
    pyrunir/kr/dl/uns/semantics/repositories.cpp
    pyrunir/kr/ps/uns/module.cpp
    pyrunir/kr/ps/uns/indices.cpp
    pyrunir/kr/ps/uns/repository.cpp
    pyrunir/kr/ps/uns/sketch_executor.cpp
    pyrunir/kr/ps/uns/syntactic_complexity.cpp
    pyrunir/kr/ps/uns/views.cpp
    pyrunir/kr/ps/uns/dl/indices.cpp
    pyrunir/kr/ps/uns/dl/module.cpp
    pyrunir/kr/ps/uns/dl/parser.cpp
    pyrunir/kr/ps/uns/dl/sketch_factory.cpp
    pyrunir/kr/ps/uns/dl/structural_termination.cpp
    pyrunir/kr/ps/uns/dl/views.cpp
)

target_link_libraries(_pyrunir PRIVATE runir::core)
target_include_directories(_pyrunir PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

yggdrasil_python_module_rpaths(PYRUNIR _pyrunir)

install(TARGETS _pyrunir DESTINATION "pyrunir" COMPONENT pyrunir)

foreach(runir_target IN ITEMS core datasets graphs kr)
    set_property(TARGET ${runir_target} PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>")
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
    CMAKE_SOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/cmake")

yggdrasil_install_runtime_path_fixup(PYRUNIR pyrunir)

yggdrasil_install_python_files(pyrunir "${CMAKE_CURRENT_SOURCE_DIR}"
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
    pyrunir/kr/ps/base/dl/__init__.py
    pyrunir/kr/dl/uns/__init__.py
    pyrunir/kr/dl/uns/semantics/__init__.py
    pyrunir/kr/ps/uns/__init__.py
    pyrunir/kr/ps/uns/dl/__init__.py)

yggdrasil_install_provider_env(PYRUNIR pyrunir
    PREFIXES "${PYPDDL_NATIVE_PREFIX}" "${TYR_NATIVE_PREFIX}" "${YGGDRASIL_NATIVE_PREFIX}")

yggdrasil_install_python_stubs(PYRUNIR pyrunir
    MODULE pyrunir._pyrunir
    PRIVATE_MODULE _pyrunir
    RENAME_PACKAGES pyrunir pytyr pypddl pyyggdrasil)

add_custom_target(pyrunir_wheel DEPENDS _pyrunir)
