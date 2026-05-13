find_package(Python 3.8 REQUIRED
  COMPONENTS Interpreter Development.Module
  OPTIONAL_COMPONENTS Development.SABIModule)

include(CMakePackageConfigHelpers)

find_package(nanobind CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH})
message(STATUS "Found nanobind: ${nanobind_DIR} (found version ${nanobind_VERSION})")

set(PYRUNIR_NATIVE_DIR "pyrunir/native")
set(PYRUNIR_NATIVE_INCLUDEDIR "${PYRUNIR_NATIVE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}")
set(PYRUNIR_NATIVE_LIBDIR "${PYRUNIR_NATIVE_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(PYRUNIR_NATIVE_BINDIR "${PYRUNIR_NATIVE_DIR}/${CMAKE_INSTALL_BINDIR}")
set(PYRUNIR_NATIVE_CMAKEDIR "${PYRUNIR_NATIVE_LIBDIR}/cmake")

nanobind_add_module(_pyrunir
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
    pyrunir/kr/gp/dl/module.cpp
    pyrunir/kr/gp/dl/parser.cpp
    pyrunir/kr/gp/dl/policy_factory.cpp
    pyrunir/kr/gp/indices.cpp
    pyrunir/kr/gp/module.cpp
    pyrunir/kr/gp/repository.cpp
    pyrunir/kr/gp/views.cpp
    pyrunir/kr/dl/semantics/datas.cpp
    pyrunir/kr/dl/semantics/indices.cpp
    pyrunir/kr/dl/semantics/module.cpp
    pyrunir/kr/dl/semantics/repositories.cpp
    pyrunir/kr/dl/semantics/views.cpp
    pyrunir/kr/dl/grammar/constructor_repository.cpp
    pyrunir/kr/dl/grammar/datas.cpp
    pyrunir/kr/dl/grammar/grammar.cpp
    pyrunir/kr/dl/grammar/grammar_factory.cpp
    pyrunir/kr/dl/grammar/indices.cpp
    pyrunir/kr/dl/grammar/module.cpp
    pyrunir/kr/dl/grammar/parser.cpp
    pyrunir/kr/dl/grammar/views.cpp
    pyrunir/kr/dl/cnf_grammar/constructor_repository.cpp
    pyrunir/kr/dl/cnf_grammar/datas.cpp
    pyrunir/kr/dl/cnf_grammar/generate.cpp
    pyrunir/kr/dl/cnf_grammar/grammar.cpp
    pyrunir/kr/dl/cnf_grammar/indices.cpp
    pyrunir/kr/dl/cnf_grammar/module.cpp
    pyrunir/kr/dl/cnf_grammar/translate.cpp
    pyrunir/kr/dl/cnf_grammar/views.cpp
)

target_link_libraries(_pyrunir PRIVATE runir::core runir::graphs)
target_include_directories(_pyrunir PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
if(APPLE)
    target_link_libraries(_pyrunir PRIVATE TBB::tbb)
elseif(UNIX)
    target_link_options(_pyrunir PRIVATE "LINKER:--no-as-needed")
    target_link_libraries(_pyrunir PRIVATE TBB::tbb)
endif()

if(RUNIR_BUILD_SHARED)
    if(APPLE)
        set(PYRUNIR_MODULE_RPATH
            "@loader_path/native/${CMAKE_INSTALL_LIBDIR}"
            "@loader_path/../pypddl/native/${CMAKE_INSTALL_LIBDIR}"
            "@loader_path/../pytyr/native/${CMAKE_INSTALL_LIBDIR}"
            "@loader_path/../pyyggdrasil/${CMAKE_INSTALL_LIBDIR}")
        set(PYRUNIR_CORE_RPATH
            "@loader_path"
            "@loader_path/../../../pypddl/native/${CMAKE_INSTALL_LIBDIR}"
            "@loader_path/../../../pytyr/native/${CMAKE_INSTALL_LIBDIR}"
            "@loader_path/../../../pyyggdrasil/${CMAKE_INSTALL_LIBDIR}")
    elseif(UNIX)
        set(PYRUNIR_MODULE_RPATH
            "$ORIGIN/native/${CMAKE_INSTALL_LIBDIR}"
            "$ORIGIN/../pypddl/native/${CMAKE_INSTALL_LIBDIR}"
            "$ORIGIN/../pytyr/native/${CMAKE_INSTALL_LIBDIR}"
            "$ORIGIN/../pyyggdrasil/${CMAKE_INSTALL_LIBDIR}")
        set(PYRUNIR_CORE_RPATH
            "$ORIGIN"
            "$ORIGIN/../../../pypddl/native/${CMAKE_INSTALL_LIBDIR}"
            "$ORIGIN/../../../pytyr/native/${CMAKE_INSTALL_LIBDIR}"
            "$ORIGIN/../../../pyyggdrasil/${CMAKE_INSTALL_LIBDIR}")
    endif()

    if(PYRUNIR_MODULE_RPATH)
        set_target_properties(_pyrunir PROPERTIES
            BUILD_RPATH "${PYRUNIR_MODULE_RPATH}"
            INSTALL_RPATH "${PYRUNIR_MODULE_RPATH}")
    endif()
endif()

install(TARGETS _pyrunir DESTINATION "pyrunir" COMPONENT pyrunir)
if(RUNIR_BUILD_SHARED)
    set_property(TARGET core PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>")
    set_property(TARGET graphs PROPERTY INTERFACE_INCLUDE_DIRECTORIES
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>")
    set_property(TARGET core PROPERTY INSTALL_RPATH "${PYRUNIR_CORE_RPATH}")
    set_property(TARGET graphs PROPERTY INSTALL_RPATH "${PYRUNIR_CORE_RPATH}")
    install(TARGETS core
            EXPORT pyrunirRunircoreTargets
            LIBRARY DESTINATION "${PYRUNIR_NATIVE_LIBDIR}" COMPONENT pyrunir
            ARCHIVE DESTINATION "${PYRUNIR_NATIVE_LIBDIR}" COMPONENT pyrunir
            RUNTIME DESTINATION "${PYRUNIR_NATIVE_BINDIR}" COMPONENT pyrunir
            INCLUDES DESTINATION "${PYRUNIR_NATIVE_INCLUDEDIR}")
    install(TARGETS graphs
            EXPORT pyrunirRunirgraphsTargets
            LIBRARY DESTINATION "${PYRUNIR_NATIVE_LIBDIR}" COMPONENT pyrunir
            ARCHIVE DESTINATION "${PYRUNIR_NATIVE_LIBDIR}" COMPONENT pyrunir
            RUNTIME DESTINATION "${PYRUNIR_NATIVE_BINDIR}" COMPONENT pyrunir
            INCLUDES DESTINATION "${PYRUNIR_NATIVE_INCLUDEDIR}")
endif()
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/runir"
        DESTINATION "${PYRUNIR_NATIVE_INCLUDEDIR}"
        COMPONENT pyrunir)
configure_package_config_file("${CMAKE_SOURCE_DIR}/Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/runirConfig.cmake"
        INSTALL_DESTINATION "${PYRUNIR_NATIVE_CMAKEDIR}/runir"
        NO_CHECK_REQUIRED_COMPONENTS_MACRO)
install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/runirConfig.cmake"
            "${CMAKE_BINARY_DIR}/runirConfigVersion.cmake"
        DESTINATION "${PYRUNIR_NATIVE_CMAKEDIR}/runir"
        COMPONENT pyrunir)
if(RUNIR_BUILD_SHARED)
    install(EXPORT pyrunirRunircoreTargets
            NAMESPACE runir::
            FILE "runircoreTargets.cmake"
            DESTINATION "${PYRUNIR_NATIVE_CMAKEDIR}/runir"
            COMPONENT pyrunir)
    install(EXPORT pyrunirRunirgraphsTargets
            NAMESPACE runir::
            FILE "runirgraphsTargets.cmake"
            DESTINATION "${PYRUNIR_NATIVE_CMAKEDIR}/runir"
            COMPONENT pyrunir)
endif()
install(DIRECTORY "${CMAKE_SOURCE_DIR}/cmake/"
        DESTINATION "${PYRUNIR_NATIVE_CMAKEDIR}/runir/cmake"
        COMPONENT pyrunir)

set(PYRUNIR_PYTHON_FILES
    pyrunir/__init__.py
    pyrunir/__init__.pyi
    pyrunir/datasets/__init__.py
    pyrunir/graphs/__init__.py
    pyrunir/kr/__init__.py
    pyrunir/kr/dl/__init__.py
    pyrunir/kr/dl/semantics/__init__.py
    pyrunir/kr/dl/grammar/__init__.py
    pyrunir/kr/dl/cnf_grammar/__init__.py
    pyrunir/kr/gp/__init__.py
    pyrunir/kr/gp/dl/__init__.py
)

foreach(python_file IN LISTS PYRUNIR_PYTHON_FILES)
    get_filename_component(python_file_directory "${python_file}" DIRECTORY)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${python_file}"
            DESTINATION "${python_file_directory}"
            COMPONENT pyrunir)
endforeach()

set(PYRUNIR_INSTALL_LIBRARY_PATHS)
foreach(native_prefix IN ITEMS "${PYPDDL_NATIVE_PREFIX}" "${TYR_NATIVE_PREFIX}" "${YGGDRASIL_NATIVE_PREFIX}")
    if(native_prefix)
        file(GLOB native_lib_dirs LIST_DIRECTORIES true "${native_prefix}/lib*")
        foreach(native_lib_dir IN LISTS native_lib_dirs)
            if(IS_DIRECTORY "${native_lib_dir}")
                list(APPEND PYRUNIR_INSTALL_LIBRARY_PATHS "${native_lib_dir}")
            endif()
        endforeach()
    endif()
endforeach()
list(JOIN PYRUNIR_INSTALL_LIBRARY_PATHS ":" PYRUNIR_INSTALL_LIBRARY_PATH)

if(PYRUNIR_INSTALL_LIBRARY_PATH)
    install(CODE
        "set(ENV{LD_LIBRARY_PATH} \"${PYRUNIR_INSTALL_LIBRARY_PATH}:$ENV{LD_LIBRARY_PATH}\")
         set(ENV{DYLD_LIBRARY_PATH} \"${PYRUNIR_INSTALL_LIBRARY_PATH}:$ENV{DYLD_LIBRARY_PATH}\")"
        COMPONENT pyrunir)
endif()

nanobind_add_stub(
    pyrunir_stubs
    MODULE pyrunir._pyrunir
    RECURSIVE
    INSTALL_TIME
    OUTPUT_PATH "pyrunir"
    PYTHON_PATH "."
    COMPONENT pyrunir
)

install(SCRIPT "${CMAKE_SOURCE_DIR}/cmake/patch_python_stubs.cmake" COMPONENT pyrunir)

add_custom_target(pyrunir_wheel DEPENDS _pyrunir)
