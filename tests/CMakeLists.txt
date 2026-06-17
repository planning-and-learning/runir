find_package(Boost REQUIRED COMPONENTS json PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
find_package(GTest CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

function(add_runir_test test_name source_file)
    add_executable(${test_name} ${source_file} unit/main.cpp)
    target_link_libraries(${test_name}
        PRIVATE
            ${ARGN}
            TBB::tbb
            Boost::json
            GTest::gtest
    )
    target_compile_definitions(${test_name}
        PRIVATE
            RUNIR_ROOT_DIR="${PROJECT_SOURCE_DIR}"
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
add_runir_test(runir_kr_ps_sketch_executor unit/kr/ps/base/dl/sketch_executor.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_structural_termination unit/kr/ps/base/dl/structural_termination.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_incomplete_structural_termination unit/kr/ps/base/dl/incomplete_structural_termination.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_ext_module_program_executor unit/kr/ps/ext/module_program_executor.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_ext_structural_termination unit/kr/ps/ext/dl/structural_termination.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_ps_ext_ceg_structural_termination unit/kr/ps/ext/dl/ceg_structural_termination.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_grammar_factory unit/kr/grammar/grammar_factory.cpp runir::kr)
add_runir_test(runir_kr_dl_uns_constructors unit/kr/dl/uns/constructors.cpp runir::datasets runir::kr)
add_runir_test(runir_kr_uns_classifier unit/kr/uns/classifier.cpp runir::datasets runir::kr)
