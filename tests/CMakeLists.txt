find_package(Boost REQUIRED COMPONENTS json PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
find_package(GTest CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

function(add_runir_test test_name source_file)
    add_executable(${test_name} ${source_file})
    target_link_libraries(${test_name}
        PRIVATE
            ${ARGN}
            Boost::json
            GTest::gtest_main
    )
    target_compile_definitions(${test_name}
        PRIVATE
            RUNIR_ROOT_DIR="${PROJECT_SOURCE_DIR}"
            ROOT_DIR="${PROJECT_SOURCE_DIR}"
    )
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_runir_test(runir_datasets_equivalence_graph unit/datasets/equivalence_graph.cpp runir::datasets)
add_runir_test(runir_graphs_color_refinement unit/graphs/algorithms/color_refinement.cpp runir::graphs)
add_runir_test(runir_kr_gp_policy_executor unit/kr/gp/dl/policy_executor.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_grammar_factory unit/kr/grammar/grammar_factory.cpp runir::kr)
