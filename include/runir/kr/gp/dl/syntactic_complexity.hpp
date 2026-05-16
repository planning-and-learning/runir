#ifndef RUNIR_KR_GP_DL_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_GP_DL_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/semantics/syntactic_complexity.hpp"
#include "runir/kr/gp/condition_view.hpp"
#include "runir/kr/gp/dl/condition_view.hpp"
#include "runir/kr/gp/dl/effect_view.hpp"
#include "runir/kr/gp/dl/feature_view.hpp"
#include "runir/kr/gp/effect_view.hpp"
#include "runir/kr/gp/policy_view.hpp"
#include "runir/kr/gp/rule_view.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <tyr/common/types.hpp>
#include <vector>

namespace runir::kr::gp::dl
{

struct FeatureSyntacticComplexityContext
{
    std::vector<tyr::Index<runir::kr::gp::Feature<runir::kr::gp::dl::BooleanFeature>>> booleans;
    std::vector<tyr::Index<runir::kr::gp::Feature<runir::kr::gp::dl::NumericalFeature>>> numericals;
};

template<typename FeatureTag>
void insert_unique(std::vector<tyr::Index<runir::kr::gp::Feature<FeatureTag>>>& features, tyr::Index<runir::kr::gp::Feature<FeatureTag>> feature)
{
    if (std::find(features.begin(), features.end(), feature) == features.end())
        features.push_back(feature);
}

template<typename FeatureTag, typename C>
void collect_feature(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::gp::Feature<FeatureTag>>, C> feature)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::gp::dl::BooleanFeature>)
        insert_unique(context.booleans, feature.get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::gp::dl::NumericalFeature>)
        insert_unique(context.numericals, feature.get_index());
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(FeatureSyntacticComplexityContext& context,
                     tyr::View<tyr::Index<runir::kr::gp::ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition)
{
    collect_feature(context, condition.get_feature());
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(FeatureSyntacticComplexityContext& context,
                     tyr::View<tyr::Index<runir::kr::gp::ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect)
{
    collect_feature(context, effect.get_feature());
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::gp::ConcreteConditionVariant<runir::kr::DlTag>>, C> condition)
{
    condition.get_variant().apply([&](auto concrete_condition) { collect_feature(context, concrete_condition); });
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::gp::ConcreteEffectVariant<runir::kr::DlTag>>, C> effect)
{
    effect.get_variant().apply([&](auto concrete_effect) { collect_feature(context, concrete_effect); });
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::gp::ConditionVariant>, C> condition)
{
    condition.get_variant().apply([&](auto concrete_condition) { collect_features(context, concrete_condition); });
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::gp::EffectVariant>, C> effect)
{
    effect.get_variant().apply([&](auto concrete_effect) { collect_features(context, concrete_effect); });
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::gp::Rule>, C> rule)
{
    for (auto condition : rule.get_conditions())
        collect_features(context, condition);
    for (auto effect : rule.get_effects())
        collect_features(context, effect);
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::gp::Policy>, C> policy)
{
    for (auto rule : policy.get_rules())
        collect_features(context, rule);
}

template<typename FeatureTag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::gp::ConcreteFeature<runir::kr::DlTag, FeatureTag>>, C> view)
{
    return 1 + runir::kr::dl::semantics::syntactic_complexity(view.get_feature());
}

template<typename C>
std::size_t syntactic_complexity(const FeatureSyntacticComplexityContext& features, const C& context)
{
    auto result = std::size_t { 0 };
    for (auto feature : features.booleans)
    {
        const auto view = tyr::View<tyr::Index<runir::kr::gp::Feature<runir::kr::gp::dl::BooleanFeature>>, C>(feature, context);
        result += view.get_variant().apply([](auto concrete_feature) { return syntactic_complexity(concrete_feature); });
    }
    for (auto feature : features.numericals)
    {
        const auto view = tyr::View<tyr::Index<runir::kr::gp::Feature<runir::kr::gp::dl::NumericalFeature>>, C>(feature, context);
        result += view.get_variant().apply([](auto concrete_feature) { return syntactic_complexity(concrete_feature); });
    }
    return result;
}

template<typename Rules>
std::size_t syntactic_complexity(const Rules& rules)
    requires requires {
        rules.begin();
        rules.end();
        rules.get_context();
    }
{
    auto context = FeatureSyntacticComplexityContext {};
    for (auto rule : rules)
        collect_features(context, rule);
    return syntactic_complexity(context, rules.get_context());
}

}  // namespace runir::kr::gp::dl

#endif
