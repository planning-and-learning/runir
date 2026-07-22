find_package(Boost REQUIRED COMPONENTS json PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
find_package(GTest CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

function(add_runir_test test_name source_file)
    add_executable(${test_name} ${source_file} unit/main.cpp)
    target_include_directories(${test_name} PRIVATE ${PROJECT_SOURCE_DIR}/tests/unit)
    target_link_libraries(${test_name}
        PRIVATE
            ${ARGN}
            TBB::tbb
            Boost::json
            GTest::gtest
    )
    target_compile_definitions(${test_name}
        PRIVATE
            ROOT_DIR="${PROJECT_SOURCE_DIR}"
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_runir_test(runir_datasets_equivalence_graph unit/datasets/equivalence_graph.cpp runir::datasets)
add_runir_test(runir_datasets_object_graph unit/datasets/object_graph.cpp runir::datasets)
add_runir_test(runir_datasets_state_graph unit/datasets/state_graph.cpp runir::datasets)
add_runir_test(runir_formatter unit/formatter.cpp runir::core)
add_runir_test(runir_graphs_color_refinement unit/graphs/algorithms/color_refinement.cpp runir::graphs)
add_runir_test(runir_graphs_static_graph unit/graphs/static_graph.cpp runir::graphs)

set(RUNIR_KR_DL_TYPE_TEST_SOURCES
    unit/kr/dl/argument.cpp
    unit/kr/dl/boolean.cpp
    unit/kr/dl/concept.cpp
    unit/kr/dl/constructor.cpp
    unit/kr/dl/numerical.cpp
    unit/kr/dl/register.cpp
    unit/kr/dl/role.cpp
    unit/kr/dl/semantics/denotation.cpp
)
add_runir_test(runir_kr_dl_types "${RUNIR_KR_DL_TYPE_TEST_SOURCES}" runir::kr)

set(RUNIR_KR_DL_GRAMMAR_TYPE_TEST_SOURCES
    unit/kr/dl/grammar/boolean.cpp
    unit/kr/dl/grammar/concept.cpp
    unit/kr/dl/grammar/constructor.cpp
    unit/kr/dl/grammar/constructor_or_non_terminal.cpp
    unit/kr/dl/grammar/derivation_rule.cpp
    unit/kr/dl/grammar/grammar.cpp
    unit/kr/dl/grammar/non_terminal.cpp
    unit/kr/dl/grammar/numerical.cpp
    unit/kr/dl/grammar/role.cpp
)
add_runir_test(runir_kr_dl_grammar_types "${RUNIR_KR_DL_GRAMMAR_TYPE_TEST_SOURCES}" runir::kr)

set(RUNIR_KR_DL_CNF_GRAMMAR_TYPE_TEST_SOURCES
    unit/kr/dl/cnf_grammar/boolean.cpp
    unit/kr/dl/cnf_grammar/concept.cpp
    unit/kr/dl/cnf_grammar/constructor.cpp
    unit/kr/dl/cnf_grammar/derivation_rule.cpp
    unit/kr/dl/cnf_grammar/grammar.cpp
    unit/kr/dl/cnf_grammar/non_terminal.cpp
    unit/kr/dl/cnf_grammar/numerical.cpp
    unit/kr/dl/cnf_grammar/role.cpp
    unit/kr/dl/cnf_grammar/substitution_rule.cpp
)
add_runir_test(runir_kr_dl_cnf_grammar_types "${RUNIR_KR_DL_CNF_GRAMMAR_TYPE_TEST_SOURCES}" runir::kr)

set(RUNIR_KR_PS_TYPE_TEST_SOURCES
    unit/kr/ps/base/condition.cpp
    unit/kr/ps/base/effect.cpp
    unit/kr/ps/base/rule.cpp
    unit/kr/ps/base/sketch.cpp
    unit/kr/ps/base/dl/condition.cpp
    unit/kr/ps/base/dl/effect.cpp
    unit/kr/ps/base/dl/feature.cpp
    unit/kr/ps/ext/condition.cpp
    unit/kr/ps/ext/effect.cpp
    unit/kr/ps/ext/execution.cpp
    unit/kr/ps/ext/feature.cpp
    unit/kr/ps/ext/memory_state.cpp
    unit/kr/ps/ext/module.cpp
    unit/kr/ps/ext/module_program.cpp
    unit/kr/ps/ext/module_symbol.cpp
    unit/kr/ps/ext/rule.cpp
    unit/kr/ps/ext/rule_variant.cpp
    unit/kr/ps/ext/dl/condition.cpp
    unit/kr/ps/ext/dl/effect.cpp
    unit/kr/ps/ext/dl/feature.cpp
)
add_runir_test(runir_kr_ps_types "${RUNIR_KR_PS_TYPE_TEST_SOURCES}" runir::kr)

add_runir_test(runir_kr_ps_abstract_structural_termination unit/kr/ps/structural_termination.cpp runir::kr)
target_include_directories(runir_kr_ps_abstract_structural_termination PRIVATE ${PROJECT_SOURCE_DIR}/src)
add_runir_test(runir_kr_ps_dl_parser unit/kr/ps/dl/parser.cpp runir::kr)
add_runir_test(runir_kr_ps_sketch_executor unit/kr/ps/base/sketch_executor.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_structural_termination unit/kr/ps/base/dl/structural_termination.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_incomplete_structural_termination unit/kr/ps/base/dl/incomplete_structural_termination.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_ext_module_program_executor unit/kr/ps/ext/module_program_executor.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_ext_incomplete_structural_termination unit/kr/ps/ext/dl/incomplete_structural_termination.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_ext_structural_termination unit/kr/ps/ext/dl/structural_termination.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_grammar_factory unit/kr/grammar/grammar_factory.cpp runir::kr)
add_runir_test(runir_kr_dl_uns_constructors unit/kr/dl/uns/constructors.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_uns_classifier
    "unit/kr/uns/classifier.cpp;unit/kr/uns/feature.cpp;unit/kr/uns/dl/feature.cpp"
    runir::datasets
    runir::kr
)
