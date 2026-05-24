#ifndef RUNIR_KR_PS_BASE_DL_SYNTACTIC_COMPLEXITY_HPP_
#define RUNIR_KR_PS_BASE_DL_SYNTACTIC_COMPLEXITY_HPP_

#include "runir/kr/dl/semantics/syntactic_complexity.hpp"
#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/dl/feature_view.hpp"
#include "runir/kr/ps/condition_view.hpp"
#include "runir/kr/ps/effect_view.hpp"
#include "runir/kr/ps/rule_view.hpp"
#include "runir/kr/ps/sketch_view.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <tyr/common/types.hpp>
#include <vector>

namespace runir::kr::ps::base::dl
{

struct FeatureSyntacticComplexityContext
{
    std::vector<tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>> booleans;
    std::vector<tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>> numericals;
};

template<typename FeatureTag>
void insert_unique(std::vector<tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>>& features,
                   tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature)
{
    if (std::find(features.begin(), features.end(), feature) == features.end())
        features.push_back(feature);
}

template<typename FeatureTag, typename C>
void collect_feature(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>, C> feature)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        insert_unique(context.booleans, feature.get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        insert_unique(context.numericals, feature.get_index());
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(
    FeatureSyntacticComplexityContext& context,
    tyr::View<tyr::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition)
{
    collect_feature(context, condition.get_feature());
}

template<typename FeatureTag, typename ObservationTag, typename C>
void collect_feature(FeatureSyntacticComplexityContext& context,
                     tyr::View<tyr::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect)
{
    collect_feature(context, effect.get_feature());
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context,
                      tyr::View<tyr::Index<runir::kr::ps::ConcreteConditionVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>, C> condition)
{
    tyr::visit([&](auto concrete_condition) { collect_feature(context, concrete_condition); }, condition.get_variant());
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context,
                      tyr::View<tyr::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>, C> effect)
{
    tyr::visit([&](auto concrete_effect) { collect_feature(context, concrete_effect); }, effect.get_variant());
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::ps::ConditionVariant<runir::kr::BaseFamilyTag>>, C> condition)
{
    tyr::visit([&](auto concrete_condition) { collect_features(context, concrete_condition); }, condition.get_variant());
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>, C> effect)
{
    tyr::visit([&](auto concrete_effect) { collect_features(context, concrete_effect); }, effect.get_variant());
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::ps::Rule<runir::kr::BaseFamilyTag>>, C> rule)
{
    for (auto condition : rule.get_conditions())
        collect_features(context, condition);
    for (auto effect : rule.get_effects())
        collect_features(context, effect);
}

template<typename C>
void collect_features(FeatureSyntacticComplexityContext& context, tyr::View<tyr::Index<runir::kr::ps::Sketch<runir::kr::BaseFamilyTag>>, C> sketch)
{
    for (auto rule : sketch.get_rules())
        collect_features(context, rule);
}

template<typename FeatureTag, typename C>
std::size_t syntactic_complexity(tyr::View<tyr::Index<runir::kr::ps::ConcreteFeature<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag>>, C> view)
{
    return 1 + runir::kr::dl::semantics::syntactic_complexity(view.get_feature());
}

template<typename C>
std::size_t syntactic_complexity(const FeatureSyntacticComplexityContext& features, const C& context)
{
    auto result = std::size_t { 0 };
    for (auto feature : features.booleans)
    {
        const auto view = tyr::View<tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>, C>(feature, context);
        result += tyr::visit([](auto concrete_feature) { return syntactic_complexity(concrete_feature); }, view.get_variant());
    }
    for (auto feature : features.numericals)
    {
        const auto view = tyr::View<tyr::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>, C>(feature, context);
        result += tyr::visit([](auto concrete_feature) { return syntactic_complexity(concrete_feature); }, view.get_variant());
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

}  // namespace runir::kr::ps::base::dl

#endif
