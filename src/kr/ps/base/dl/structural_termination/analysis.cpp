#include "detail.hpp"
#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"

#include <algorithm>

namespace runir::kr::ps::base::dl::detail
{

using Profile = runir::kr::ps::detail::RuleProfile;

template<typename Features, typename Feature>
std::size_t feature_position(const Features& features, Feature feature)
{
    return static_cast<std::size_t>(
        std::distance(features.begin(),
                      std::find_if(features.begin(), features.end(), [&](auto candidate) { return candidate.get_index() == feature.get_index(); })));
}

template<typename FeatureTag, typename ObservationTag>
void record_condition(
    SketchView sketch,
    Profile& profile,
    ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> condition)
{
    namespace psdl = runir::kr::ps::dl;
    const auto position = [&]()
    {
        if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
            return feature_position(sketch.get_features<psdl::BooleanFeature>(), condition.get_feature());
        else
            return feature_position(sketch.get_features<psdl::NumericalFeature>(), condition.get_feature());
    }();
    profile.template conditions<ObservationTag>().set(position);
}

template<typename FeatureTag, typename ObservationTag>
void record_effect(
    SketchView sketch,
    Profile& profile,
    ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> effect)
{
    namespace psdl = runir::kr::ps::dl;
    if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
    {
        const auto position = feature_position(sketch.get_features<psdl::BooleanFeature>(), effect.get_feature());
        profile.template effects<ObservationTag>().set(position);
    }
    else
    {
        const auto position = feature_position(sketch.get_features<psdl::NumericalFeature>(), effect.get_feature());
        profile.numerical_changes[position] = Profile::template numerical_change<ObservationTag>();
    }
}

Profile make_rule_profile(SketchView sketch, RuleView rule)
{
    auto profile = Profile(sketch.get_features<runir::kr::ps::dl::BooleanFeature>().size(), sketch.get_features<runir::kr::ps::dl::NumericalFeature>().size());
    for (auto condition : rule.get_conditions())
        ygg::visit([&](auto concrete_variant)
                   { ygg::visit([&](auto concrete) { record_condition(sketch, profile, concrete); }, concrete_variant.get_variant()); },
                   condition.get_variant());
    for (auto effect : rule.get_effects())
        ygg::visit([&](auto concrete_variant) { ygg::visit([&](auto concrete) { record_effect(sketch, profile, concrete); }, concrete_variant.get_variant()); },
                   effect.get_variant());
    return profile;
}

SketchAnalysis analyze_sketch(SketchView sketch)
{
    auto analysis = SketchAnalysis { {},
                                     runir::kr::ps::detail::QualitativePolicy(1,
                                                                              sketch.get_features<runir::kr::ps::dl::BooleanFeature>().size(),
                                                                              sketch.get_features<runir::kr::ps::dl::NumericalFeature>().size()) };

    for (auto rule : sketch.get_rules())
    {
        analysis.rules.push_back(rule);
        analysis.policy.rule_profiles.push_back(make_rule_profile(sketch, rule));
    }
    return analysis;
}

}  // namespace runir::kr::ps::base::dl::detail
