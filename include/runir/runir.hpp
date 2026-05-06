#pragma once

#include <fmt/format.h>
#include <string>
#include <string_view>

namespace template_feature
{

struct FeatureSummary
{
    std::string name;
    int score = 0;
};

int add(int lhs, int rhs);

FeatureSummary describe(std::string name, int score);

std::string format_summary(const FeatureSummary& summary);

}

template<>
struct fmt::formatter<template_feature::FeatureSummary> : fmt::formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const template_feature::FeatureSummary& summary, FormatContext& ctx) const
    {
        const auto text = fmt::format("{}: {}", summary.name, summary.score);
        return fmt::formatter<std::string_view>::format(text, ctx);
    }
};
