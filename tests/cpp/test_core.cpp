#include <gtest/gtest.h>
#include <runir/runir.hpp>

TEST(RunirTests, AddsIntegers) { EXPECT_EQ(runir::add(20, 22), 42); }

TEST(RunirTests, FormatsSummary)
{
    const auto summary = runir::describe("planner", 7);
    EXPECT_EQ(runir::format_summary(summary), "planner: 7");
}
