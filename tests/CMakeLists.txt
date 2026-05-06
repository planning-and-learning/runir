find_package(GTest CONFIG REQUIRED PATHS ${CMAKE_PREFIX_PATH} NO_DEFAULT_PATH)

include(GoogleTest)

add_executable(template_feature_tests
    cpp/test_core.cpp
)

target_link_libraries(template_feature_tests
    PRIVATE
        template_feature::core
        GTest::gtest_main
)

gtest_discover_tests(template_feature_tests)

