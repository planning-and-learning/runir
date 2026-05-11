find_package(GTest CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

add_executable(runir_tests
    unit/kr/grammar/grammar_factory.cpp
)

target_link_libraries(runir_tests
    PRIVATE
        runir::core
        GTest::gtest_main
)

target_compile_definitions(runir_tests
    PRIVATE
        RUNIR_ROOT_DIR="${PROJECT_SOURCE_DIR}"
)

add_test(NAME runir_tests COMMAND runir_tests)

set_tests_properties(runir_tests
    PROPERTIES
        ENVIRONMENT "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:TBB::tbb>:$ENV{LD_LIBRARY_PATH};DYLD_LIBRARY_PATH=$<TARGET_FILE_DIR:TBB::tbb>:$ENV{DYLD_LIBRARY_PATH}"
)
