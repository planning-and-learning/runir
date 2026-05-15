find_package(Boost REQUIRED COMPONENTS json PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)
find_package(GTest CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

add_executable(runir_tests
    unit/datasets/equivalence_graph.cpp
    unit/graphs/algorithms/color_refinement.cpp
    unit/kr/gp/dl/policy_executor.cpp
    unit/kr/grammar/grammar_factory.cpp
)

target_link_libraries(runir_tests
    PRIVATE
        runir::core
        runir::graphs
        Boost::json
        GTest::gtest_main
)

target_compile_definitions(runir_tests
    PRIVATE
        RUNIR_ROOT_DIR="${PROJECT_SOURCE_DIR}"
        ROOT_DIR="${PROJECT_SOURCE_DIR}"
)

add_test(NAME runir_tests COMMAND runir_tests)
