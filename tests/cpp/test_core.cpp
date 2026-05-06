#include <template_feature/template_feature.hpp>

#include <gtest/gtest.h>

TEST(TemplateFeatureTests, AddsIntegers)
{
    EXPECT_EQ(template_feature::add(20, 22), 42);
}

TEST(TemplateFeatureTests, FormatsSummary)
{
    const auto summary = template_feature::describe("planner", 7);
    EXPECT_EQ(template_feature::format_summary(summary), "planner: 7");
}

