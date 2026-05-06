#include <template_feature/template_feature.hpp>

#include <utility>

namespace template_feature
{

int add(int lhs, int rhs) { return lhs + rhs; }

FeatureSummary describe(std::string name, int score) { return FeatureSummary { std::move(name), score }; }

std::string format_summary(const FeatureSummary& summary) { return fmt::format("{}", summary); }

}
