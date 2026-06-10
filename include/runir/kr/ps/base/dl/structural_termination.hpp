#ifndef RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/dl/feature_view.hpp"
#include "runir/kr/ps/base/dl/syntactic_complexity.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"
#include "runir/kr/ps/effect_view.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::base::dl
{

/**
 * Structural termination of a rule set via sieve-style analysis.
 *
 * A rule is removable if it strictly changes a feature in a direction that no
 * remaining rule can undo: it decreases a numerical feature that no remaining
 * rule increases or leaves unconstrained, or it flips a boolean feature whose
 * opposite flip no remaining rule performs or leaves unconstrained. A feature
 * that is not mentioned in a rule's effects is unconstrained, i.e., it may
 * change in any direction. The rule set is structurally terminating iff the
 * sieve removes all rules. This is a sufficient condition for termination.
 */

enum class FeatureChange
{
    unconstrained,
    sets_true,
    sets_false,
    increases,
    decreases,
    unchanged,
};

struct RuleEffectProfile
{
    std::vector<FeatureChange> booleans;
    std::vector<FeatureChange> numericals;
};

template<typename FeatureTag>
std::size_t feature_position(const std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>>& features,
                             ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature)
{
    return static_cast<std::size_t>(std::distance(features.begin(), std::find(features.begin(), features.end(), feature)));
}

template<typename FeatureTag, typename ObservationTag, typename C>
void record_effect(const FeatureSyntacticComplexityContext& features,
                   RuleEffectProfile& profile,
                   ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
    {
        auto& change = profile.booleans[feature_position(features.booleans, effect.get_feature().get_index())];
        if constexpr (std::same_as<ObservationTag, runir::kr::ps::dl::Positive>)
            change = FeatureChange::sets_true;
        else if constexpr (std::same_as<ObservationTag, runir::kr::ps::dl::Negative>)
            change = FeatureChange::sets_false;
        else if constexpr (std::same_as<ObservationTag, runir::kr::ps::dl::Unchanged>)
            change = FeatureChange::unchanged;
    }
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
    {
        auto& change = profile.numericals[feature_position(features.numericals, effect.get_feature().get_index())];
        if constexpr (std::same_as<ObservationTag, runir::kr::ps::dl::Increases>)
            change = FeatureChange::increases;
        else if constexpr (std::same_as<ObservationTag, runir::kr::ps::dl::Decreases>)
            change = FeatureChange::decreases;
        else if constexpr (std::same_as<ObservationTag, runir::kr::ps::dl::Unchanged>)
            change = FeatureChange::unchanged;
    }
}

template<typename C>
void record_effect(const FeatureSyntacticComplexityContext& features,
                   RuleEffectProfile& profile,
                   ygg::View<ygg::Index<runir::kr::ps::ConcreteEffectVariant<runir::kr::BaseFamilyTag, runir::kr::DlTag>>, C> effect)
{
    ygg::visit([&](auto concrete_effect) { record_effect(features, profile, concrete_effect); }, effect.get_variant());
}

template<typename C>
void record_effect(const FeatureSyntacticComplexityContext& features,
                   RuleEffectProfile& profile,
                   ygg::View<ygg::Index<runir::kr::ps::EffectVariant<runir::kr::BaseFamilyTag>>, C> effect)
{
    ygg::visit([&](auto concrete_effect) { record_effect(features, profile, concrete_effect); }, effect.get_variant());
}

template<typename C>
RuleEffectProfile make_rule_effect_profile(const FeatureSyntacticComplexityContext& features,
                                           ygg::View<ygg::Index<runir::kr::ps::base::Rule>, C> rule)
{
    auto profile = RuleEffectProfile {};
    profile.booleans.assign(features.booleans.size(), FeatureChange::unconstrained);
    profile.numericals.assign(features.numericals.size(), FeatureChange::unconstrained);
    for (auto effect : rule.get_effects())
        record_effect(features, profile, effect);
    return profile;
}

inline bool sieve(std::vector<RuleEffectProfile> profiles)
{
    auto alive = std::vector<bool>(profiles.size(), true);
    const auto num_booleans = profiles.empty() ? std::size_t { 0 } : profiles.front().booleans.size();
    const auto num_numericals = profiles.empty() ? std::size_t { 0 } : profiles.front().numericals.size();

    const auto none_alive_with = [&](auto&& member, std::size_t pos, FeatureChange change)
    {
        for (std::size_t rule = 0; rule < profiles.size(); ++rule)
            if (alive[rule] && member(profiles[rule])[pos] == change)
                return false;
        return true;
    };
    const auto kill_alive_with = [&](auto&& member, std::size_t pos, FeatureChange change)
    {
        auto changed = false;
        for (std::size_t rule = 0; rule < profiles.size(); ++rule)
        {
            if (alive[rule] && member(profiles[rule])[pos] == change)
            {
                alive[rule] = false;
                changed = true;
            }
        }
        return changed;
    };
    const auto booleans = [](const RuleEffectProfile& profile) -> const auto& { return profile.booleans; };
    const auto numericals = [](const RuleEffectProfile& profile) -> const auto& { return profile.numericals; };

    auto changed = true;
    while (changed)
    {
        changed = false;
        for (std::size_t pos = 0; pos < num_numericals; ++pos)
        {
            if (none_alive_with(numericals, pos, FeatureChange::increases) && none_alive_with(numericals, pos, FeatureChange::unconstrained))
                changed |= kill_alive_with(numericals, pos, FeatureChange::decreases);
        }
        for (std::size_t pos = 0; pos < num_booleans; ++pos)
        {
            if (none_alive_with(booleans, pos, FeatureChange::sets_true) && none_alive_with(booleans, pos, FeatureChange::unconstrained))
                changed |= kill_alive_with(booleans, pos, FeatureChange::sets_false);
            if (none_alive_with(booleans, pos, FeatureChange::sets_false) && none_alive_with(booleans, pos, FeatureChange::unconstrained))
                changed |= kill_alive_with(booleans, pos, FeatureChange::sets_true);
        }
    }

    return std::none_of(alive.begin(), alive.end(), [](bool is_alive) { return is_alive; });
}

template<typename Rules>
bool is_structurally_terminating(const Rules& rules)
    requires requires {
        rules.begin();
        rules.end();
        rules.get_context();
    }
{
    auto features = FeatureSyntacticComplexityContext {};
    for (auto rule : rules)
        collect_features(features, rule);

    auto profiles = std::vector<RuleEffectProfile> {};
    for (auto rule : rules)
        profiles.push_back(make_rule_effect_profile(features, rule));

    return sieve(std::move(profiles));
}

template<typename C>
bool is_structurally_terminating(ygg::View<ygg::Index<runir::kr::ps::base::Sketch>, C> sketch)
{
    return is_structurally_terminating(sketch.get_rules());
}

}  // namespace runir::kr::ps::base::dl

#endif
