#include "detail.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/dl/condition_view.hpp"
#include "runir/kr/ps/dl/effect_view.hpp"

namespace runir::kr::ps::base::dl::detail
{

ps::dl::RuleProfile make_rule_profile(SketchView sketch, RuleView rule)
{
    auto profile = ps::dl::RuleProfile(sketch.get_features<ps::dl::BooleanFeature>().size(), sketch.get_features<ps::dl::NumericalFeature>().size());
    for (auto condition : rule.get_conditions())
        ygg::visit([&](auto concrete_variant)
                   { ygg::visit([&](auto concrete) { ps::detail::record_condition(sketch, profile, concrete); }, concrete_variant.get_variant()); },
                   condition.get_variant());
    for (auto effect : rule.get_effects())
        ygg::visit([&](auto concrete_variant)
                   { ygg::visit([&](auto concrete) { ps::detail::record_effect(sketch, profile, concrete); }, concrete_variant.get_variant()); },
                   effect.get_variant());
    return profile;
}

Analysis analyze_sketch(SketchView sketch)
{
    auto analysis = Analysis { {},
                               runir::kr::ps::detail::QualitativePolicy(1,
                                                                        sketch.get_features<runir::kr::ps::dl::BooleanFeature>().size(),
                                                                        sketch.get_features<runir::kr::ps::dl::NumericalFeature>().size()) };

    for (auto rule : sketch.get_rules())
    {
        analysis.rules.push_back(rule);
        analysis.policy.rule_profiles.push_back(make_rule_profile(sketch, rule));
    }
    runir::kr::ps::detail::validate_policy(analysis.policy);
    return analysis;
}

}  // namespace runir::kr::ps::base::dl::detail
