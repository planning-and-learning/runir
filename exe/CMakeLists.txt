add_executable(state_graph state_graph.cpp)

target_link_libraries(state_graph PRIVATE runir::core)

if(RUNIR_BUILD_TESTS)
    set(RUNIR_STATE_GRAPH_TEST_ARGS
        --domain-filepath ${PROJECT_SOURCE_DIR}/data/benchmarks/classical/tests/gripper/domain.pddl
        --problem-filepath ${PROJECT_SOURCE_DIR}/data/benchmarks/classical/tests/gripper/test-1.pddl
        --num-worker-threads 1)

    add_test(
        NAME runir_exe_state_graph_lifted_smoke
        COMMAND state_graph ${RUNIR_STATE_GRAPH_TEST_ARGS})
    set_tests_properties(runir_exe_state_graph_lifted_smoke PROPERTIES PASS_REGULAR_EXPRESSION "digraph G")

    add_test(
        NAME runir_exe_state_graph_ground_smoke
        COMMAND state_graph ${RUNIR_STATE_GRAPH_TEST_ARGS} --instantiate-ground-task)
    set_tests_properties(runir_exe_state_graph_ground_smoke PROPERTIES PASS_REGULAR_EXPRESSION "digraph G")
endif()
