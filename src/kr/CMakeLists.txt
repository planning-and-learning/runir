add_library(kr SHARED
    database/free_relations.cpp
    database/optimization/details/cost.cpp
    database/optimization/details/estimation.cpp
    database/optimization/details/join_block.cpp
    database/optimization/details/normalization.cpp
    database/optimization/details/optimization.cpp
    database/optimization/details/structural.cpp
    database/semantics/bound/query_evaluation.cpp
    database/semantics/bound/union_normal_form.cpp
    database/datalog/aggregate.cpp
    database/datalog/construction.cpp
    database/datalog/evaluator.cpp
    database/datalog/formatter.cpp
    database/datalog/goal_search.cpp
    database/datalog/plain.cpp
    database/datalog/planner.cpp
    database/datalog/recursive.cpp
    database/datalog/repository.cpp
    database/datalog/strata.cpp
    database/datalog/stratification.cpp
    database/datalog/translation.cpp
    database/datalog/weighted.cpp
    database/semantics/details/distance.cpp
    database/semantics/details/min_distance.cpp
    database/semantics/details/plans.cpp
    database/semantics/generic_join.cpp
    database/semantics/incremental/delta.cpp
    database/semantics/incremental/details/distance_repair.cpp
    database/semantics/incremental/distance.cpp
    database/semantics/incremental/join.cpp
    database/semantics/incremental/projection.cpp
    database/semantics/query_evaluation.cpp
    database/syntax/copy.cpp
    database/syntax/formatter.cpp
    database/syntax/operator_builder.cpp
    database/syntax/query.cpp
    database/syntax/query_construction.cpp
    database/syntax/query_repository.cpp
    domain_context.cpp
    task_context.cpp
    dl/formatter.cpp
    dl/optimization.cpp
    dl/query_construction.cpp
    dl/repository.cpp
    dl/translations/ygg_query.cpp
    dl/semantics/denotation_caches.cpp
    dl/semantics/evaluation.cpp
    dl/semantics/formatter.cpp
    dl/semantics/heuristics/projection.cpp
    parser/datalog.cpp
    dl/semantics/repository.cpp
    dl/semantics/state_evaluation_context.cpp
    dl/semantics/syntactic_complexity.cpp
    dl/cnf_grammar/builder.cpp
    dl/cnf_grammar/constructor_repository.cpp
    dl/cnf_grammar/formatter.cpp
    dl/cnf_grammar/generate_ground.cpp
    dl/cnf_grammar/generate_lifted.cpp
    dl/cnf_grammar/generation_state.cpp
    dl/cnf_grammar/translate.cpp
    dl/grammar/constructor_repository.cpp
    dl/grammar/formatter.cpp
    dl/grammar/base/grammar_factory.cpp
    dl/grammar/base/parser.cpp
    dl/grammar/parser/parser.cpp
    dl/grammar/parser/parser_instantiations.cpp
    ps/base/dl/formatter.cpp
    ps/base/dl/parser.cpp
    ps/base/dl/parser/parser.cpp
    ps/base/dl/parser/parser_instantiations.cpp
    ps/base/dl/sketch_factory.cpp
    ps/base/dl/incomplete_structural_termination.cpp
    ps/base/dl/structural_termination.cpp
    ps/base/dl/structural_termination/analysis.cpp
    ps/base/dl/structural_termination/incomplete_result.cpp
    ps/base/dl/structural_termination/result.cpp
    ps/base/formatter.cpp
    ps/base/sketch_executor.cpp
    ps/ext/dl/formatter.cpp
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
    ps/ext/formatter.cpp
    ps/ext/program_executor_ground.cpp
    ps/ext/program_executor_lifted.cpp
    ps/icp/dl/parser.cpp
    ps/icp/dl/parser/parser.cpp
    ps/icp/formatter.cpp
    ps/icp/program_executor.cpp
    ps/formatter.cpp
    ps/repository.cpp
    ps/structural_termination.cpp
    ps/structural_termination/incomplete.cpp
    ps/structural_termination/policy_graph.cpp
    ps/structural_termination/scc_refinement_forest.cpp
    ps/structural_termination/sieve.cpp
    uns/dl/classifier_factory.cpp
    uns/dl/parser.cpp
    uns/dl/parser/parser.cpp
    uns/formatter.cpp
    uns/dl/parser/parser_instantiations.cpp
)
add_library(runir::kr ALIAS kr)

set_target_properties(kr PROPERTIES OUTPUT_NAME runir_kr)

# The database's sources of each directory listed share most of their instantiations: batched, they compile them once
# (brokkr's template rules, rule 11). The Datalog evaluator's sources (database/datalog) and the free relations'
# repository (database/free_relations.cpp) are compiled each on its own, and so is the query repository's
# (database/syntax/query_repository.cpp): it sees the definitions of every held part (keyed_impl.hpp), which its batch
# would then instantiate implicitly for the free relations' repository too.
brokkr_unity_directories(
    kr
    ENABLED ${RUNIR_UNITY_BUILD}
    DIRECTORIES
        database/optimization/details
        database/semantics
        database/semantics/bound
        database/semantics/details
        database/semantics/incremental
        database/syntax
)
set_source_files_properties(database/syntax/query_repository.cpp PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GCC 15 reports the value-initialized element Boost.Spirit X3 parses a list's items into as maybe uninitialized.
    set_source_files_properties(uns/dl/parser/parser_instantiations.cpp PROPERTIES COMPILE_OPTIONS -Wno-maybe-uninitialized)
    # GCC 13's IPA mod/ref pass crashes (segmentation fault) on the builder instantiations of the grammar repositories
    # (the recursion of yggdrasil's insert_child/find_child over their many record kinds), and so does its LTO streamer
    # once the pass is off for them alone: compiled without both.
    set_source_files_properties(dl/grammar/constructor_repository.cpp dl/cnf_grammar/builder.cpp PROPERTIES COMPILE_OPTIONS "-fno-ipa-modref;-fno-lto")
endif()

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
