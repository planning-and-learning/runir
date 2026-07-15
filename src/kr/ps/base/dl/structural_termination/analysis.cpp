#include "detail.hpp"
#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/repository.hpp"

#include <algorithm>

namespace runir::kr::ps::base::dl::detail
{

using Profile = runir::kr::ps::detail::RuleProfile;

template<typename FeatureTag>
std::size_t feature_position(const std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>>& features,
                             ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature)
{
    return static_cast<std::size_t>(std::distance(features.begin(), std::find(features.begin(), features.end(), feature)));
}

template<typename FeatureTag, typename ObservationTag>
void record_condition(
    const FeatureSyntacticComplexityContext& features,
    Profile& profile,
    ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> condition)
{
    namespace psdl = runir::kr::ps::dl;
    const auto position = [&]()
    {
        if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
            return feature_position(features.booleans, condition.get_feature().get_index());
        else
            return feature_position(features.numericals, condition.get_feature().get_index());
    }();
    profile.template conditions<ObservationTag>().set(position);
}

template<typename FeatureTag, typename ObservationTag>
void record_effect(
    const FeatureSyntacticComplexityContext& features,
    Profile& profile,
    ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> effect)
{
    namespace psdl = runir::kr::ps::dl;
    if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
    {
        const auto position = feature_position(features.booleans, effect.get_feature().get_index());
        profile.template effects<ObservationTag>().set(position);
    }
    else
    {
        const auto position = feature_position(features.numericals, effect.get_feature().get_index());
        profile.numerical_changes[position] = Profile::template numerical_change<ObservationTag>();
    }
}

Profile make_rule_profile(const FeatureSyntacticComplexityContext& features, RuleView rule)
{
    auto profile = Profile(features.booleans.size(), features.numericals.size());
    for (auto condition : rule.get_conditions())
        ygg::visit([&](auto concrete_variant)
                   { ygg::visit([&](auto concrete) { record_condition(features, profile, concrete); }, concrete_variant.get_variant()); },
                   condition.get_variant());
    for (auto effect : rule.get_effects())
        ygg::visit([&](auto concrete_variant)
                   { ygg::visit([&](auto concrete) { record_effect(features, profile, concrete); }, concrete_variant.get_variant()); },
                   effect.get_variant());
    return profile;
}

SketchAnalysis analyze_sketch(SketchView sketch)
{
    auto features = FeatureSyntacticComplexityContext {};
    collect_features(features, sketch);
    const auto num_booleans = features.booleans.size();
    const auto num_numericals = features.numericals.size();
    auto analysis = SketchAnalysis { std::move(features), {}, runir::kr::ps::detail::QualitativePolicy(1, num_booleans, num_numericals) };

    for (auto rule : sketch.get_rules())
    {
        analysis.rules.push_back(rule);
        analysis.policy.rule_profiles.push_back(make_rule_profile(analysis.features, rule));
    }
    return analysis;
}

}  // namespace runir::kr::ps::base::dl::detail
