#ifndef RUNIR_KR_PS_UNS_DL_STRUCTURAL_TERMINATION_IMPL_HPP_
#define RUNIR_KR_PS_UNS_DL_STRUCTURAL_TERMINATION_IMPL_HPP_

#include "runir/graphs/algorithms.hpp"
#include "runir/kr/ps/uns/dl/condition_view.hpp"
#include "runir/kr/ps/uns/dl/effect_view.hpp"
#include "runir/kr/ps/uns/dl/feature_view.hpp"
#include "runir/kr/ps/uns/dl/structural_termination.hpp"
#include "runir/kr/ps/uns/dl/syntactic_complexity.hpp"
#include "runir/kr/ps/uns/rule_view.hpp"
#include "runir/kr/ps/uns/sketch_view.hpp"

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

namespace runir::kr::ps::uns::dl
{

/*
 * Implementation notes.
 *
 * The policy graph G(R) (sieve.pdf; incomplete_sieve.pdf Section 5.1) has one
 * node per Boolean feature valuation over the sketch's features (numerical
 * features n are abstracted to the Boolean condition n > 0), and an edge
 * b -> b' labeled with rule r iff (b, b') is compatible with r under the
 * qualitative semantics (sieve.pdf Theorem 1):
 *   - conditions constrain b: positive/negative for Booleans,
 *     greater_zero/equal_zero for numericals;
 *   - effects constrain b': Boolean positive/negative force the target bit,
 *     unchanged copies the source bit; numerical increases forces n > 0 in
 *     b' (since n' > n >= 0), decreases requires n > 0 in b (n is
 *     well-founded downward) and leaves the target bit free, unchanged
 *     copies the bit; features unmentioned in the effects are unconstrained
 *     (incomplete_sieve.pdf Section 5.1: "n?").
 *
 * The sieve repeatedly removes, within each strongly connected component,
 * the edges that decrease a numerical feature that no edge in the component
 * increases or leaves unconstrained, and the edges that flip a Boolean
 * feature whose opposite flip occurs on no edge of the component. The sketch
 * is terminating iff the surviving graph is acyclic. The iterative
 * per-component fixpoint used here is equivalent to the recursive
 * formulation of sieve.pdf Algorithm 1: an edge is removed exactly when its
 * current component contains no opposing edge.
 */

namespace detail
{

/// Per-rule qualitative profile over the feature positions.
struct RuleProfile
{
    // Conditions over the source valuation.
    boost::dynamic_bitset<> boolean_positive_conditions;
    boost::dynamic_bitset<> boolean_negative_conditions;
    boost::dynamic_bitset<> numerical_greater_conditions;
    boost::dynamic_bitset<> numerical_zero_conditions;
    // Effects.
    boost::dynamic_bitset<> boolean_positive_effects;
    boost::dynamic_bitset<> boolean_negative_effects;
    boost::dynamic_bitset<> boolean_unchanged_effects;
    std::vector<NumericalChange> numerical_changes;

    RuleProfile(std::size_t num_booleans, std::size_t num_numericals) :
        boolean_positive_conditions(num_booleans),
        boolean_negative_conditions(num_booleans),
        numerical_greater_conditions(num_numericals),
        numerical_zero_conditions(num_numericals),
        boolean_positive_effects(num_booleans),
        boolean_negative_effects(num_booleans),
        boolean_unchanged_effects(num_booleans),
        numerical_changes(num_numericals, NumericalChange::UNCONSTRAINED)
    {
    }
};

template<typename FeatureTag>
std::size_t feature_position(const std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, FeatureTag>>>& features,
                             ygg::Index<runir::kr::ps::Feature<runir::kr::UnsFamilyTag, FeatureTag>> feature)
{
    return static_cast<std::size_t>(std::distance(features.begin(), std::find(features.begin(), features.end(), feature)));
}

template<typename FeatureTag, typename ObservationTag>
void record_condition(const FeatureSyntacticComplexityContext& features,
                      RuleProfile& profile,
                      ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>,
                                Repository> condition)
{
    namespace psdl = runir::kr::ps::dl;
    if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
    {
        const auto position = feature_position(features.booleans, condition.get_feature().get_index());
        if constexpr (std::same_as<ObservationTag, psdl::Positive>)
            profile.boolean_positive_conditions.set(position);
        else if constexpr (std::same_as<ObservationTag, psdl::Negative>)
            profile.boolean_negative_conditions.set(position);
    }
    else if constexpr (std::same_as<FeatureTag, psdl::NumericalFeature>)
    {
        const auto position = feature_position(features.numericals, condition.get_feature().get_index());
        if constexpr (std::same_as<ObservationTag, psdl::GreaterZero>)
            profile.numerical_greater_conditions.set(position);
        else if constexpr (std::same_as<ObservationTag, psdl::EqualZero>)
            profile.numerical_zero_conditions.set(position);
    }
}

template<typename FeatureTag, typename ObservationTag>
void record_effect(const FeatureSyntacticComplexityContext& features,
                   RuleProfile& profile,
                   ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::UnsFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>,
                             Repository> effect)
{
    namespace psdl = runir::kr::ps::dl;
    if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
    {
        const auto position = feature_position(features.booleans, effect.get_feature().get_index());
        if constexpr (std::same_as<ObservationTag, psdl::Positive>)
            profile.boolean_positive_effects.set(position);
        else if constexpr (std::same_as<ObservationTag, psdl::Negative>)
            profile.boolean_negative_effects.set(position);
        else if constexpr (std::same_as<ObservationTag, psdl::Unchanged>)
            profile.boolean_unchanged_effects.set(position);
    }
    else if constexpr (std::same_as<FeatureTag, psdl::NumericalFeature>)
    {
        const auto position = feature_position(features.numericals, effect.get_feature().get_index());
        if constexpr (std::same_as<ObservationTag, psdl::Increases>)
            profile.numerical_changes[position] = NumericalChange::INCREASES;
        else if constexpr (std::same_as<ObservationTag, psdl::Decreases>)
            profile.numerical_changes[position] = NumericalChange::DECREASES;
        else if constexpr (std::same_as<ObservationTag, psdl::Unchanged>)
            profile.numerical_changes[position] = NumericalChange::UNCHANGED;
    }
}

inline RuleProfile make_rule_profile(const FeatureSyntacticComplexityContext& features, RuleView rule)
{
    auto profile = RuleProfile(features.booleans.size(), features.numericals.size());
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

struct PolicyEdge
{
    std::size_t source;
    std::size_t target;
    std::size_t rule_position;
    bool alive = true;
};

/// Vertex ids encode the valuation: Boolean feature i at bit i, numerical
/// feature j at bit num_booleans + j.
inline boost::dynamic_bitset<> vertex_booleans(std::size_t vertex, std::size_t num_booleans)
{
    auto values = boost::dynamic_bitset<>(num_booleans);
    for (std::size_t position = 0; position < num_booleans; ++position)
        values.set(position, (vertex >> position) & std::size_t { 1 });
    return values;
}

inline boost::dynamic_bitset<> vertex_numericals(std::size_t vertex, std::size_t num_booleans, std::size_t num_numericals)
{
    auto values = boost::dynamic_bitset<>(num_numericals);
    for (std::size_t position = 0; position < num_numericals; ++position)
        values.set(position, (vertex >> (num_booleans + position)) & std::size_t { 1 });
    return values;
}

inline std::size_t make_vertex(const boost::dynamic_bitset<>& booleans, const boost::dynamic_bitset<>& numericals)
{
    auto vertex = std::size_t { 0 };
    for (std::size_t position = 0; position < booleans.size(); ++position)
        if (booleans.test(position))
            vertex |= std::size_t { 1 } << position;
    for (std::size_t position = 0; position < numericals.size(); ++position)
        if (numericals.test(position))
            vertex |= std::size_t { 1 } << (booleans.size() + position);
    return vertex;
}

/// Enumerate the policy graph edges of one rule by calling back with
/// (source vertex, target vertex).
template<typename Callback>
void enumerate_rule_edges(const RuleProfile& profile, std::size_t num_booleans, std::size_t num_numericals, Callback&& callback)
{
    const auto num_valuations = std::size_t { 1 } << (num_booleans + num_numericals);

    for (std::size_t source = 0; source < num_valuations; ++source)
    {
        const auto source_booleans = vertex_booleans(source, num_booleans);
        const auto source_numericals = vertex_numericals(source, num_booleans, num_numericals);

        // Conditions.
        if (!profile.boolean_positive_conditions.is_subset_of(source_booleans))
            continue;
        if (profile.boolean_negative_conditions.intersects(source_booleans))
            continue;
        if (!profile.numerical_greater_conditions.is_subset_of(source_numericals))
            continue;
        if (profile.numerical_zero_conditions.intersects(source_numericals))
            continue;

        // Effects: compute forced target values and the free positions.
        auto target_booleans = profile.boolean_positive_effects | (profile.boolean_unchanged_effects & source_booleans);
        const auto fixed_booleans = profile.boolean_positive_effects | profile.boolean_negative_effects | profile.boolean_unchanged_effects;

        auto target_numericals = boost::dynamic_bitset<>(num_numericals);
        auto fixed_numericals = boost::dynamic_bitset<>(num_numericals);
        auto decreases_unsatisfiable = false;
        for (std::size_t position = 0; position < num_numericals; ++position)
        {
            switch (profile.numerical_changes[position])
            {
                case NumericalChange::INCREASES:
                {
                    // n' > n >= 0 implies n' > 0.
                    target_numericals.set(position);
                    fixed_numericals.set(position);
                    break;
                }
                case NumericalChange::DECREASES:
                {
                    // n' < n requires n > 0 in the source; the target bit is free.
                    if (!source_numericals.test(position))
                        decreases_unsatisfiable = true;
                    break;
                }
                case NumericalChange::UNCHANGED:
                {
                    target_numericals.set(position, source_numericals.test(position));
                    fixed_numericals.set(position);
                    break;
                }
                case NumericalChange::UNCONSTRAINED: break;
            }
        }
        if (decreases_unsatisfiable)
            continue;

        auto free_positions = std::vector<std::pair<bool, std::size_t>> {};  // (is_boolean, position)
        for (std::size_t position = 0; position < num_booleans; ++position)
            if (!fixed_booleans.test(position))
                free_positions.emplace_back(true, position);
        for (std::size_t position = 0; position < num_numericals; ++position)
            if (!fixed_numericals.test(position))
                free_positions.emplace_back(false, position);

        // Enumerate all assignments of the free positions.
        for (std::size_t assignment = 0; assignment < (std::size_t { 1 } << free_positions.size()); ++assignment)
        {
            for (std::size_t free = 0; free < free_positions.size(); ++free)
            {
                const auto [is_boolean, position] = free_positions[free];
                const auto value = static_cast<bool>((assignment >> free) & std::size_t { 1 });
                (is_boolean ? target_booleans : target_numericals).set(position, value);
            }
            callback(source, make_vertex(target_booleans, target_numericals));
        }
    }
}

}  // namespace detail

inline StructuralTerminationResult structural_termination(SketchView sketch)
{
    auto features = FeatureSyntacticComplexityContext {};
    collect_features(features, sketch);

    const auto num_booleans = features.booleans.size();
    const auto num_numericals = features.numericals.size();
    if (num_booleans + num_numericals > 16)
        throw std::invalid_argument("structural_termination: too many features; the policy graph has 2^|features| vertices.");

    auto rules = std::vector<RuleView> {};
    for (auto rule : sketch.get_rules())
        rules.push_back(rule);

    auto profiles = std::vector<detail::RuleProfile> {};
    profiles.reserve(rules.size());
    for (auto rule : rules)
        profiles.push_back(detail::make_rule_profile(features, rule));

    // Build the policy graph edge list.
    auto edges = std::vector<detail::PolicyEdge> {};
    for (std::size_t rule_position = 0; rule_position < profiles.size(); ++rule_position)
        detail::enumerate_rule_edges(profiles[rule_position],
                                     num_booleans,
                                     num_numericals,
                                     [&](std::size_t source, std::size_t target)
                                     { edges.push_back(detail::PolicyEdge { source, target, rule_position }); });

    const auto num_valuations = std::size_t { 1 } << (num_booleans + num_numericals);

    // Iterative per-component sieve (equivalent to the recursive Algorithm 1
    // of sieve.pdf; see the file comment).
    auto component_of = std::vector<std::size_t>(num_valuations, 0);
    auto changed = true;
    while (changed)
    {
        changed = false;

        // Strongly connected components of the surviving graph.
        auto builder = graphs::StaticGraphBuilder<> {};
        for (std::size_t vertex = 0; vertex < num_valuations; ++vertex)
            builder.add_vertex();
        for (const auto& edge : edges)
            if (edge.alive)
                builder.add_directed_edge(static_cast<graphs::VertexIndex>(edge.source), static_cast<graphs::VertexIndex>(edge.target));
        const auto graph = graphs::StaticGraph<> { std::move(builder) };
        const auto [num_components, components] = graphs::algorithms::strong_components(graph);
        for (std::size_t vertex = 0; vertex < num_valuations; ++vertex)
            component_of[vertex] = components[vertex];

        // Per component: which numerical features some surviving
        // intra-component edge increases or leaves unconstrained, and which
        // Boolean flips occur.
        struct ComponentProfile
        {
            boost::dynamic_bitset<> numerical_increased_or_unconstrained;
            boost::dynamic_bitset<> boolean_flipped_to_true;
            boost::dynamic_bitset<> boolean_flipped_to_false;

            ComponentProfile(std::size_t num_booleans, std::size_t num_numericals) :
                numerical_increased_or_unconstrained(num_numericals),
                boolean_flipped_to_true(num_booleans),
                boolean_flipped_to_false(num_booleans)
            {
            }
        };
        auto component_profiles = std::vector<ComponentProfile>(static_cast<std::size_t>(num_components),
                                                                ComponentProfile(num_booleans, num_numericals));

        for (const auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            auto& component = component_profiles[component_of[edge.source]];
            const auto& changes = profiles[edge.rule_position].numerical_changes;
            for (std::size_t position = 0; position < num_numericals; ++position)
                if (changes[position] == NumericalChange::INCREASES || changes[position] == NumericalChange::UNCONSTRAINED)
                    component.numerical_increased_or_unconstrained.set(position);
            const auto source_booleans = detail::vertex_booleans(edge.source, num_booleans);
            const auto target_booleans = detail::vertex_booleans(edge.target, num_booleans);
            component.boolean_flipped_to_true |= target_booleans - source_booleans;
            component.boolean_flipped_to_false |= source_booleans - target_booleans;
        }

        // Remove intra-component edges that decrease a numerical feature that
        // no edge of the component increases or leaves unconstrained, and
        // edges that flip a Boolean whose opposite flip never occurs in the
        // component.
        for (auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            const auto& component = component_profiles[component_of[edge.source]];

            auto removable = false;
            const auto& changes = profiles[edge.rule_position].numerical_changes;
            for (std::size_t position = 0; position < num_numericals && !removable; ++position)
                if (changes[position] == NumericalChange::DECREASES && !component.numerical_increased_or_unconstrained.test(position))
                    removable = true;
            const auto source_booleans = detail::vertex_booleans(edge.source, num_booleans);
            const auto target_booleans = detail::vertex_booleans(edge.target, num_booleans);
            // Flips some p to false that no edge in the component flips to true?
            if (((source_booleans - target_booleans) - component.boolean_flipped_to_true).any())
                removable = true;
            // Flips some p to true that no edge in the component flips to false?
            if (((target_booleans - source_booleans) - component.boolean_flipped_to_false).any())
                removable = true;

            if (removable)
            {
                edge.alive = false;
                changed = true;
            }
        }
    }

    auto result = StructuralTerminationResult {};
    result.booleans = features.booleans;
    result.numericals = features.numericals;

    // Surviving intra-component edges witness unbroken cycles.
    auto has_cycle = false;
    for (const auto& edge : edges)
        if (edge.alive && component_of[edge.source] == component_of[edge.target])
            has_cycle = true;

    if (!has_cycle)
    {
        result.status = StructuralTerminationStatus::TERMINATING;
        return result;
    }

    // Counterexample: the union of the surviving non-trivial strongly
    // connected components.
    result.status = StructuralTerminationStatus::NON_TERMINATING;
    auto counterexample_builder = PolicyGraphBuilder {};
    auto vertex_remap = std::vector<std::size_t>(num_valuations, std::numeric_limits<std::size_t>::max());
    const auto map_vertex = [&](std::size_t vertex)
    {
        if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
            vertex_remap[vertex] = counterexample_builder.add_vertex(PolicyGraphVertexLabel(
                detail::vertex_booleans(vertex, num_booleans), detail::vertex_numericals(vertex, num_booleans, num_numericals)));
        return static_cast<graphs::VertexIndex>(vertex_remap[vertex]);
    };
    for (const auto& edge : edges)
    {
        if (!edge.alive || component_of[edge.source] != component_of[edge.target])
            continue;
        counterexample_builder.add_directed_edge(
            map_vertex(edge.source),
            map_vertex(edge.target),
            PolicyGraphEdgeLabel(rules[edge.rule_position], profiles[edge.rule_position].numerical_changes));
    }
    result.counterexample = std::make_shared<PolicyGraph>(std::move(counterexample_builder));
    return result;
}

}  // namespace runir::kr::ps::uns::dl

#endif
