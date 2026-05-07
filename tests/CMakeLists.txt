find_package(GTest CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

include(GoogleTest)

add_executable(runir_tests
    cpp/test_core.cpp
)

target_link_libraries(runir_tests
    PRIVATE
        runir::core
        GTest::gtest_main
)

gtest_discover_tests(runir_tests)

