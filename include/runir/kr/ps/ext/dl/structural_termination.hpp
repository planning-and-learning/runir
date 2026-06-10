#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/config.hpp"
#include "runir/graphs/algorithms.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/ext/repository.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>
#include <yggdrasil/core/types.hpp>

#if RUNIR_ENABLE_FMT_FORMATTERS
#include <fmt/format.h>
#include <string>
#include <string_view>
#endif

namespace runir::kr::ps::ext::dl
{

/**
 * Structural termination of an extended sketch module via the Sieve
 * algorithm on the extended policy graph.
 *
 * Per bonet-et-al-icaps2024.pdf ("Termination for Extended Sketches",
 * Theorem 10), termination of an extended sketch is tested by an adaptation
 * of the Sieve algorithm (sieve.pdf Algorithm 1) over the extended policy
 * graph: nodes pair a Boolean feature valuation with a memory state of the
 * module; an edge (b, m) -> (b', m') labeled with rule r exists iff r leads
 * from memory state m to m', its conditions hold in b, and (b, b') is
 * compatible with its effects.
 *
 * Feature kinds: concept features are cardinalities and are treated like
 * numerical features (well-founded downward, abstracted to |C| > 0);
 * Boolean and numerical features as in the base family.
 *
 * Rule kinds (bonet-et-al-icaps2024.pdf Algorithm 2):
 *  - sketch rules carry explicit effects; unmentioned features are
 *    unconstrained;
 *  - load, do, and call rules are treated conservatively: all features are
 *    unconstrained along their edges. (Load rules do not change the planning
 *    state, but they write registers and feature denotations may depend on
 *    registers; the register-aware refinement of Bonet and Geffner 2023 is a
 *    sound strengthening left to future work.) This keeps the test sound: a
 *    "terminating" verdict is always correct, while modules whose
 *    termination argument rests on register structure are reported
 *    non-terminating.
 *
 * On failure, the result carries the union of the surviving non-trivial
 * strongly connected components of the extended policy graph.
 */

enum class StructuralTerminationStatus
{
    TERMINATING,
    NON_TERMINATING,
};

/// Qualitative change of one numerical-like feature (concepts first, then
/// numericals) along an edge.
enum class NumericalChange : std::uint8_t
{
    UNCONSTRAINED = 0,
    INCREASES = 1,
    DECREASES = 2,
    UNCHANGED = 3,
};

struct ModulePolicyGraphVertexLabel
{
    std::uint32_t concept_values;    ///< bit i: |concepts[i]| > 0
    std::uint32_t boolean_values;    ///< bit i: booleans[i]
    std::uint32_t numerical_values;  ///< bit i: numericals[i] > 0
    MemoryStateView memory_state;

    ModulePolicyGraphVertexLabel(std::uint32_t concept_values_,
                                 std::uint32_t boolean_values_,
                                 std::uint32_t numerical_values_,
                                 MemoryStateView memory_state_) noexcept :
        concept_values(concept_values_),
        boolean_values(boolean_values_),
        numerical_values(numerical_values_),
        memory_state(memory_state_)
    {
    }

    auto identifying_members() const noexcept { return std::tie(concept_values, boolean_values, numerical_values, memory_state); }
};

struct ModulePolicyGraphEdgeLabel
{
    RuleVariantView rule;
    /// 2 bits per numerical-like position: concepts at [0, num_concepts),
    /// numericals at [num_concepts, num_concepts + num_numericals).
    std::uint64_t numerical_changes;

    ModulePolicyGraphEdgeLabel(RuleVariantView rule_, std::uint64_t numerical_changes_) noexcept :
        rule(rule_),
        numerical_changes(numerical_changes_)
    {
    }

    NumericalChange get_numerical_change(std::size_t position) const noexcept
    {
        return static_cast<NumericalChange>((numerical_changes >> (2 * position)) & std::uint64_t { 3 });
    }

    auto identifying_members() const noexcept { return std::tie(rule, numerical_changes); }
};

using ModulePolicyGraphBuilder = graphs::StaticGraphBuilder<ModulePolicyGraphVertexLabel, ModulePolicyGraphEdgeLabel>;
using ModulePolicyGraph = graphs::StaticGraph<ModulePolicyGraphVertexLabel, ModulePolicyGraphEdgeLabel>;

struct ModuleStructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::vector<ygg::Index<Feature<runir::kr::dl::ConceptTag>>> concepts;
    std::vector<ygg::Index<Feature<runir::kr::ps::dl::BooleanFeature>>> booleans;
    std::vector<ygg::Index<Feature<runir::kr::ps::dl::NumericalFeature>>> numericals;
    std::shared_ptr<ModulePolicyGraph> counterexample;  ///< nullptr iff terminating.

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

namespace detail
{

struct ModuleFeatures
{
    std::vector<ygg::Index<Feature<runir::kr::dl::ConceptTag>>> concepts;
    std::vector<ygg::Index<Feature<runir::kr::ps::dl::BooleanFeature>>> booleans;
    std::vector<ygg::Index<Feature<runir::kr::ps::dl::NumericalFeature>>> numericals;
};

template<typename FeatureTag>
std::size_t feature_position(const std::vector<ygg::Index<Feature<FeatureTag>>>& features, ygg::Index<Feature<FeatureTag>> feature)
{
    return static_cast<std::size_t>(std::distance(features.begin(), std::find(features.begin(), features.end(), feature)));
}

template<typename FeatureTag>
void insert_unique(std::vector<ygg::Index<Feature<FeatureTag>>>& features, ygg::Index<Feature<FeatureTag>> feature)
{
    if (std::find(features.begin(), features.end(), feature) == features.end())
        features.push_back(feature);
}

template<typename FeatureTag, typename ObservationTag>
void collect_feature(ModuleFeatures& features,
                     ygg::View<ygg::Index<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> condition)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        insert_unique(features.concepts, condition.get_feature().get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        insert_unique(features.booleans, condition.get_feature().get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        insert_unique(features.numericals, condition.get_feature().get_index());
}

template<typename FeatureTag, typename ObservationTag>
void collect_feature(ModuleFeatures& features,
                     ygg::View<ygg::Index<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> effect)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        insert_unique(features.concepts, effect.get_feature().get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        insert_unique(features.booleans, effect.get_feature().get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        insert_unique(features.numericals, effect.get_feature().get_index());
}

struct ModuleRuleProfile
{
    std::size_t source_memory_position = 0;
    std::size_t target_memory_position = 0;
    // Conditions over the source valuation; numerical-like positions span
    // concepts then numericals.
    std::uint32_t boolean_positive_conditions = 0;
    std::uint32_t boolean_negative_conditions = 0;
    std::uint64_t numerical_greater_conditions = 0;
    std::uint64_t numerical_zero_conditions = 0;
    // Effects (sketch rules only; other kinds leave everything below empty,
    // i.e. unconstrained).
    std::uint32_t boolean_positive_effects = 0;
    std::uint32_t boolean_negative_effects = 0;
    std::uint32_t boolean_unchanged_effects = 0;
    std::uint64_t numerical_changes = 0;  // 2 bits per numerical-like position.
};

template<typename FeatureTag, typename ObservationTag>
void record_condition(const ModuleFeatures& features,
                      ModuleRuleProfile& profile,
                      std::size_t num_concepts,
                      ygg::View<ygg::Index<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> condition)
{
    namespace psdl = runir::kr::ps::dl;
    if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
    {
        const auto bit = std::uint32_t { 1 } << feature_position(features.booleans, condition.get_feature().get_index());
        if constexpr (std::same_as<ObservationTag, psdl::Positive>)
            profile.boolean_positive_conditions |= bit;
        else if constexpr (std::same_as<ObservationTag, psdl::Negative>)
            profile.boolean_negative_conditions |= bit;
    }
    else
    {
        auto position = std::size_t { 0 };
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            position = feature_position(features.concepts, condition.get_feature().get_index());
        else
            position = num_concepts + feature_position(features.numericals, condition.get_feature().get_index());
        const auto bit = std::uint64_t { 1 } << position;
        if constexpr (std::same_as<ObservationTag, psdl::GreaterZero>)
            profile.numerical_greater_conditions |= bit;
        else if constexpr (std::same_as<ObservationTag, psdl::EqualZero>)
            profile.numerical_zero_conditions |= bit;
    }
}

template<typename FeatureTag, typename ObservationTag>
void record_effect(const ModuleFeatures& features,
                   ModuleRuleProfile& profile,
                   std::size_t num_concepts,
                   ygg::View<ygg::Index<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> effect)
{
    namespace psdl = runir::kr::ps::dl;
    if constexpr (std::same_as<FeatureTag, psdl::BooleanFeature>)
    {
        const auto bit = std::uint32_t { 1 } << feature_position(features.booleans, effect.get_feature().get_index());
        if constexpr (std::same_as<ObservationTag, psdl::Positive>)
            profile.boolean_positive_effects |= bit;
        else if constexpr (std::same_as<ObservationTag, psdl::Negative>)
            profile.boolean_negative_effects |= bit;
        else if constexpr (std::same_as<ObservationTag, psdl::Unchanged>)
            profile.boolean_unchanged_effects |= bit;
    }
    else
    {
        auto position = std::size_t { 0 };
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            position = feature_position(features.concepts, effect.get_feature().get_index());
        else
            position = num_concepts + feature_position(features.numericals, effect.get_feature().get_index());
        auto change = NumericalChange::UNCONSTRAINED;
        if constexpr (std::same_as<ObservationTag, psdl::Increases>)
            change = NumericalChange::INCREASES;
        else if constexpr (std::same_as<ObservationTag, psdl::Decreases>)
            change = NumericalChange::DECREASES;
        else if constexpr (std::same_as<ObservationTag, psdl::Unchanged>)
            change = NumericalChange::UNCHANGED;
        profile.numerical_changes |= static_cast<std::uint64_t>(change) << (2 * position);
    }
}

struct ModulePolicyEdge
{
    std::uint32_t source;
    std::uint32_t target;
    std::uint32_t rule_position;
    std::uint64_t numerical_changes;
    bool alive = true;
};

/// Iterative per-component sieve over the given edges (sieve.pdf Algorithm 1;
/// see the file comment of base/dl/structural_termination.hpp for the
/// equivalence with the recursive formulation). `vertex_booleans` extracts
/// the Boolean valuation bits from a vertex id. Returns whether a cycle
/// survives, together with the final component assignment.
template<typename VertexBooleans>
std::pair<bool, std::vector<std::size_t>>
sieve(std::vector<ModulePolicyEdge>& edges, std::size_t num_vertices, std::size_t num_numerical_like, VertexBooleans&& vertex_booleans)
{
    auto component_of = std::vector<std::size_t>(num_vertices, 0);
    auto changed = true;
    while (changed)
    {
        changed = false;

        auto builder = graphs::StaticGraphBuilder<> {};
        for (std::size_t vertex = 0; vertex < num_vertices; ++vertex)
            builder.add_vertex();
        for (const auto& edge : edges)
            if (edge.alive)
                builder.add_directed_edge(edge.source, edge.target);
        const auto graph = graphs::StaticGraph<> { std::move(builder) };
        const auto [num_components, components] = graphs::algorithms::strong_components(graph);
        for (std::size_t vertex = 0; vertex < num_vertices; ++vertex)
            component_of[vertex] = components[vertex];

        struct ComponentProfile
        {
            std::uint64_t numerical_increased_or_unconstrained = 0;
            std::uint32_t boolean_flipped_to_true = 0;
            std::uint32_t boolean_flipped_to_false = 0;
        };
        auto component_profiles = std::vector<ComponentProfile>(static_cast<std::size_t>(num_components));

        for (const auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            auto& profile = component_profiles[component_of[edge.source]];
            for (std::size_t position = 0; position < num_numerical_like; ++position)
            {
                const auto change = static_cast<NumericalChange>((edge.numerical_changes >> (2 * position)) & std::uint64_t { 3 });
                if (change == NumericalChange::INCREASES || change == NumericalChange::UNCONSTRAINED)
                    profile.numerical_increased_or_unconstrained |= std::uint64_t { 1 } << position;
            }
            const auto source_booleans = vertex_booleans(edge.source);
            const auto target_booleans = vertex_booleans(edge.target);
            profile.boolean_flipped_to_true |= ~source_booleans & target_booleans;
            profile.boolean_flipped_to_false |= source_booleans & ~target_booleans;
        }

        for (auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            const auto& profile = component_profiles[component_of[edge.source]];

            auto removable = false;
            for (std::size_t position = 0; position < num_numerical_like && !removable; ++position)
            {
                const auto change = static_cast<NumericalChange>((edge.numerical_changes >> (2 * position)) & std::uint64_t { 3 });
                if (change == NumericalChange::DECREASES && !(profile.numerical_increased_or_unconstrained & (std::uint64_t { 1 } << position)))
                    removable = true;
            }
            const auto source_booleans = vertex_booleans(edge.source);
            const auto target_booleans = vertex_booleans(edge.target);
            if ((source_booleans & ~target_booleans & ~profile.boolean_flipped_to_true) != 0)
                removable = true;
            if ((~source_booleans & target_booleans & ~profile.boolean_flipped_to_false) != 0)
                removable = true;

            if (removable)
            {
                edge.alive = false;
                changed = true;
            }
        }
    }

    auto has_cycle = false;
    for (const auto& edge : edges)
        if (edge.alive && component_of[edge.source] == component_of[edge.target])
            has_cycle = true;

    return { has_cycle, std::move(component_of) };
}

/// Memory states, rules, features, and per-rule qualitative profiles of a
/// module, with feature positions over the global feature lists.
struct ModuleAnalysis
{
    std::vector<MemoryStateView> memory_states;
    std::vector<RuleVariantView> rules;
    ModuleFeatures features;
    std::vector<ModuleRuleProfile> profiles;
};

inline ModuleAnalysis analyze_module(ModuleView module_)
{
    auto analysis = ModuleAnalysis {};

    for (auto memory_state : module_.get_memory_states())
        analysis.memory_states.push_back(memory_state);
    const auto memory_position = [&](ygg::Index<MemoryState> memory_state)
    {
        for (std::size_t position = 0; position < analysis.memory_states.size(); ++position)
            if (analysis.memory_states[position].get_index() == memory_state)
                return position;
        throw std::logic_error("structural_termination: rule references a memory state not listed in the module.");
    };

    const auto collect_rule_features = [&](auto rule)
    {
        for (auto condition : rule.get_conditions())
            ygg::visit([&](auto concrete_variant)
                       { ygg::visit([&](auto concrete) { collect_feature(analysis.features, concrete); }, concrete_variant.get_variant()); },
                       condition.get_variant());
        if constexpr (requires { rule.get_effects(); })
        {
            for (auto effect : rule.get_effects())
                ygg::visit([&](auto concrete_variant)
                           { ygg::visit([&](auto concrete) { collect_feature(analysis.features, concrete); }, concrete_variant.get_variant()); },
                           effect.get_variant());
        }
    };
    for (auto transition : module_.get_memory_transitions())
        for (auto rule : transition.get_rules())
        {
            analysis.rules.push_back(rule);
            ygg::visit([&](auto concrete_rule) { collect_rule_features(concrete_rule); }, rule.get_variant());
        }

    const auto num_concepts = analysis.features.concepts.size();
    for (auto rule : analysis.rules)
    {
        auto profile = ModuleRuleProfile {};
        ygg::visit(
            [&](auto concrete_rule)
            {
                profile.source_memory_position = memory_position(concrete_rule.get_source().get_index());
                profile.target_memory_position = memory_position(concrete_rule.get_target().get_index());
                for (auto condition : concrete_rule.get_conditions())
                    ygg::visit(
                        [&](auto concrete_variant) {
                            ygg::visit([&](auto concrete) { record_condition(analysis.features, profile, num_concepts, concrete); },
                                       concrete_variant.get_variant());
                        },
                        condition.get_variant());
                if constexpr (requires { concrete_rule.get_effects(); })
                {
                    for (auto effect : concrete_rule.get_effects())
                        ygg::visit(
                            [&](auto concrete_variant) {
                                ygg::visit([&](auto concrete) { record_effect(analysis.features, profile, num_concepts, concrete); },
                                           concrete_variant.get_variant());
                            },
                            effect.get_variant());
                }
                // Load/do/call rules: no effect entries; everything stays
                // unconstrained (see the file comment).
            },
            rule.get_variant());
        analysis.profiles.push_back(profile);
    }

    return analysis;
}

}  // namespace detail

inline ModuleStructuralTerminationResult structural_termination(ModuleView module_)
{
    auto analysis = detail::analyze_module(module_);
    const auto& memory_states = analysis.memory_states;
    const auto& rules = analysis.rules;
    const auto& features = analysis.features;
    const auto& profiles = analysis.profiles;

    const auto num_memory_states = memory_states.size();
    if (num_memory_states == 0)
        return ModuleStructuralTerminationResult {};  // no memory states, no rules, trivially terminating

    const auto num_concepts = features.concepts.size();
    const auto num_booleans = features.booleans.size();
    const auto num_numericals = features.numericals.size();
    const auto num_numerical_like = num_concepts + num_numericals;
    if (num_booleans > 32 || num_numerical_like > 32 || num_booleans + num_numerical_like > 14)
        throw std::invalid_argument("structural_termination: too many features; the policy graph has 2^|features| * |memory| vertices.");

    // Build the extended policy graph: vertex id = valuation * |M| + memory.
    const auto num_valuations = std::uint64_t { 1 } << (num_booleans + num_numerical_like);
    const auto num_vertices = static_cast<std::size_t>(num_valuations) * std::max<std::size_t>(num_memory_states, 1);
    const auto boolean_mask = (num_booleans ? (std::uint32_t { 1 } << num_booleans) : 1u) - 1;

    auto edges = std::vector<detail::ModulePolicyEdge> {};
    for (std::size_t rule_position = 0; rule_position < profiles.size(); ++rule_position)
    {
        const auto& profile = profiles[rule_position];
        for (std::uint64_t source = 0; source < num_valuations; ++source)
        {
            const auto source_booleans = static_cast<std::uint32_t>(source) & boolean_mask;
            const auto source_numericals = static_cast<std::uint64_t>(source >> num_booleans);

            if ((source_booleans & profile.boolean_positive_conditions) != profile.boolean_positive_conditions)
                continue;
            if ((~source_booleans & profile.boolean_negative_conditions) != profile.boolean_negative_conditions)
                continue;
            if ((source_numericals & profile.numerical_greater_conditions) != profile.numerical_greater_conditions)
                continue;
            if ((~source_numericals & profile.numerical_zero_conditions) != profile.numerical_zero_conditions)
                continue;

            auto forced_booleans = profile.boolean_positive_effects | (profile.boolean_unchanged_effects & source_booleans);
            auto fixed_boolean_mask = profile.boolean_positive_effects | profile.boolean_negative_effects | profile.boolean_unchanged_effects;

            auto forced_numericals = std::uint64_t { 0 };
            auto fixed_numerical_mask = std::uint64_t { 0 };
            auto decreases_unsatisfiable = false;
            for (std::size_t position = 0; position < num_numerical_like; ++position)
            {
                const auto bit = std::uint64_t { 1 } << position;
                switch (static_cast<NumericalChange>((profile.numerical_changes >> (2 * position)) & std::uint64_t { 3 }))
                {
                    case NumericalChange::INCREASES:
                    {
                        forced_numericals |= bit;
                        fixed_numerical_mask |= bit;
                        break;
                    }
                    case NumericalChange::DECREASES:
                    {
                        if (!(source_numericals & bit))
                            decreases_unsatisfiable = true;
                        break;
                    }
                    case NumericalChange::UNCHANGED:
                    {
                        forced_numericals |= source_numericals & bit;
                        fixed_numerical_mask |= bit;
                        break;
                    }
                    case NumericalChange::UNCONSTRAINED: break;
                }
            }
            if (decreases_unsatisfiable)
                continue;

            const auto free_boolean_mask = static_cast<std::uint32_t>(~fixed_boolean_mask) & boolean_mask;
            const auto numerical_like_mask = (num_numerical_like ? (std::uint64_t { 1 } << num_numerical_like) : 1u) - 1;
            const auto free_numerical_mask = ~fixed_numerical_mask & numerical_like_mask;

            auto boolean_subset = std::uint32_t { 0 };
            while (true)
            {
                auto numerical_subset = std::uint64_t { 0 };
                while (true)
                {
                    const auto target_booleans = forced_booleans | boolean_subset;
                    const auto target_numericals = forced_numericals | numerical_subset;
                    const auto target_valuation = static_cast<std::uint64_t>(target_booleans) | (target_numericals << num_booleans);
                    edges.push_back(detail::ModulePolicyEdge {
                        static_cast<std::uint32_t>(source * num_memory_states + profile.source_memory_position),
                        static_cast<std::uint32_t>(target_valuation * num_memory_states + profile.target_memory_position),
                        static_cast<std::uint32_t>(rule_position),
                        profile.numerical_changes });

                    if (numerical_subset == free_numerical_mask)
                        break;
                    numerical_subset = (numerical_subset - free_numerical_mask) & free_numerical_mask;
                }
                if (boolean_subset == free_boolean_mask)
                    break;
                boolean_subset = (boolean_subset - free_boolean_mask) & free_boolean_mask;
            }
        }
    }

    const auto [has_cycle, component_of] =
        detail::sieve(edges,
                      num_vertices,
                      num_numerical_like,
                      [&](std::uint32_t vertex) { return static_cast<std::uint32_t>(vertex / num_memory_states) & boolean_mask; });

    auto result = ModuleStructuralTerminationResult {};
    result.concepts = features.concepts;
    result.booleans = features.booleans;
    result.numericals = features.numericals;

    if (!has_cycle)
    {
        result.status = StructuralTerminationStatus::TERMINATING;
        return result;
    }

    result.status = StructuralTerminationStatus::NON_TERMINATING;
    auto counterexample_builder = ModulePolicyGraphBuilder {};
    auto vertex_remap = std::vector<std::uint32_t>(num_vertices, std::numeric_limits<std::uint32_t>::max());
    const auto map_vertex = [&](std::uint32_t vertex)
    {
        if (vertex_remap[vertex] == std::numeric_limits<std::uint32_t>::max())
        {
            const auto valuation = static_cast<std::uint64_t>(vertex / num_memory_states);
            const auto memory = static_cast<std::size_t>(vertex % num_memory_states);
            const auto booleans = static_cast<std::uint32_t>(valuation) & boolean_mask;
            const auto numerical_like = valuation >> num_booleans;
            const auto concepts = static_cast<std::uint32_t>(numerical_like & ((num_concepts ? (std::uint64_t { 1 } << num_concepts) : 1u) - 1));
            const auto numericals = static_cast<std::uint32_t>(numerical_like >> num_concepts);
            vertex_remap[vertex] =
                counterexample_builder.add_vertex(ModulePolicyGraphVertexLabel(concepts, booleans, numericals, memory_states[memory]));
        }
        return vertex_remap[vertex];
    };
    for (const auto& edge : edges)
    {
        if (!edge.alive || component_of[edge.source] != component_of[edge.target])
            continue;
        counterexample_builder.add_directed_edge(map_vertex(edge.source),
                                                 map_vertex(edge.target),
                                                 ModulePolicyGraphEdgeLabel(rules[edge.rule_position], edge.numerical_changes));
    }
    result.counterexample = std::make_shared<ModulePolicyGraph>(std::move(counterexample_builder));
    return result;
}

}  // namespace runir::kr::ps::ext::dl

#if RUNIR_ENABLE_FMT_FORMATTERS
namespace fmt
{

template<>
struct formatter<runir::kr::ps::ext::dl::ModuleStructuralTerminationResult, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::ext::dl::ModuleStructuralTerminationResult& result, FormatContext& ctx) const
    {
        const auto text = result.is_terminating() ?
                              std::string { "ModuleStructuralTerminationResult(terminating)" } :
                              fmt::format("ModuleStructuralTerminationResult(non-terminating, counterexample with {} vertices and {} edges)",
                                          result.counterexample->get_num_vertices(),
                                          result.counterexample->get_num_edges());
        return formatter<std::string_view>::format(text, ctx);
    }
};

}  // namespace fmt
#endif

#endif
