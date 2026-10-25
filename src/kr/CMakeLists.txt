add_library(kr SHARED
    domain_context.cpp
    task_context.cpp
    dl/query_construction.cpp
    dl/repository.cpp
    dl/translations/ygg_query.cpp
    dl/semantics/evaluation.cpp
    dl/semantics/heuristics/projection.cpp
    dl/semantics/repository.cpp
    dl/semantics/state_evaluation_context.cpp
    dl/cnf_grammar/constructor_repository.cpp
    dl/cnf_grammar/generate.cpp
    dl/cnf_grammar/symbol_repositories.cpp
    dl/cnf_grammar/translate.cpp
    dl/grammar/constructor_repository.cpp
    dl/grammar/base/grammar_factory.cpp
    dl/grammar/base/parser.cpp
    dl/grammar/parser/parser.cpp
    dl/grammar/parser/parser_instantiations.cpp
    dl/grammar/symbol_repositories.cpp
    dl/symbol_repositories.cpp
    ps/base/dl/parser.cpp
    ps/base/dl/parser/parser.cpp
    ps/base/dl/parser/parser_instantiations.cpp
    ps/base/dl/sketch_factory.cpp
    ps/base/dl/incomplete_structural_termination.cpp
    ps/base/dl/structural_termination.cpp
    ps/base/dl/structural_termination/analysis.cpp
    ps/base/dl/structural_termination/incomplete_result.cpp
    ps/base/dl/structural_termination/result.cpp
    ps/base/sketch_executor.cpp
    ps/ext/dl/incomplete_structural_termination.cpp
    ps/ext/dl/program_structural_termination.cpp
    ps/ext/dl/module_factory.cpp
    ps/ext/dl/parser/parser.cpp
    ps/ext/dl/parser/parser_instantiations.cpp
    ps/ext/dl/parser.cpp
    ps/ext/dl/structural_termination.cpp
    ps/ext/dl/structural_termination/analysis.cpp
    ps/ext/dl/structural_termination/incomplete_result.cpp
    ps/ext/dl/structural_termination/result.cpp
    ps/ext/program_executor.cpp
    ps/icp/dl/parser.cpp
    ps/icp/dl/parser/parser.cpp
    ps/icp/program_executor.cpp
    ps/repository.cpp
    ps/structural_termination.cpp
    ps/structural_termination/incomplete.cpp
    ps/structural_termination/policy_graph.cpp
    ps/structural_termination/scc_refinement_forest.cpp
    ps/structural_termination/sieve.cpp
    uns/dl/classifier_factory.cpp
    uns/dl/parser.cpp
    uns/dl/parser/parser.cpp
    uns/dl/parser/parser_instantiations.cpp
)
add_library(runir::kr ALIAS kr)

set_target_properties(kr PROPERTIES OUTPUT_NAME runir_kr)

target_compile_features(kr PUBLIC cxx_std_20)

target_compile_definitions(kr
    PUBLIC
        FMT_HEADER_ONLY
        # The DL grammar AST variants have up to 30 alternatives (include/runir/kr/dl/grammar/ast/ast.hpp).
        # The MPL list limit comes from tyr and loki (50); every translation unit must agree on it.
        BOOST_VARIANT_LIMIT_TYPES=30
        $<$<BOOL:${RUNIR_DELTA_EVALUATION}>:RUNIR_DELTA_EVALUATION=1>
)

target_include_directories(kr
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(kr
    PUBLIC
        yggdrasil::yggdrasil
        yggdrasil::database
        fmt::fmt-header-only
        runir::datasets
        tyr::core
)

if(APPLE)
    set(RUNIR_KR_LIBRARY_ORIGIN "@loader_path")
elseif(UNIX)
    set(RUNIR_KR_LIBRARY_ORIGIN "$ORIGIN")
endif()

set_target_properties(kr PROPERTIES INSTALL_RPATH "${RUNIR_KR_LIBRARY_ORIGIN}")

install(
    TARGETS kr
    EXPORT runirTargets
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT runir
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT runir
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
