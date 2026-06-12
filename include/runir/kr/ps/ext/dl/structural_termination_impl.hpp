#ifndef RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_IMPL_HPP_
#define RUNIR_KR_PS_EXT_DL_STRUCTURAL_TERMINATION_IMPL_HPP_

#include "runir/graphs/algorithms.hpp"
#include "runir/kr/ps/ext/dl/structural_termination.hpp"

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace runir::kr::ps::ext::dl
{

/*
 * Implementation notes.
 *
 * Extended policy graph per bonet-et-al-icaps2024.pdf ("Termination for
 * Extended Sketches", Theorem 10): nodes pair a Boolean feature valuation
 * with a memory state of the module; an edge (b, m) -> (b', m') labeled with
 * rule r exists iff r leads from memory state m to m', its conditions hold
 * in b, and (b, b') is compatible with its effects.
 *
 * Concept features are cardinalities and are treated like numerical features
 * (well-founded downward, abstracted to |C| > 0).
 *
 * Rule kinds (bonet-et-al-icaps2024.pdf Algorithm 2): sketch rules carry
 * explicit effects with unmentioned features unconstrained. Do rules may also
 * carry explicit effects; concrete execution/proof only accepts matching
 * actions whose source-to-successor feature transition satisfies those
 * effects. Load rules do not change the planning state; loading concept
 * register r only unconstrains features whose denotation depends on r (the
 * paper's Phi(r)), and leaves all other tracked features unchanged. Call rules
 * are treated conservatively with all features unconstrained because they may
 * change the planning state through arbitrary module side effects.
 */

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
void collect_feature(ModuleFeatures& features, ygg::View<ygg::Index<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> condition)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        insert_unique(features.concepts, condition.get_feature().get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        insert_unique(features.booleans, condition.get_feature().get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        insert_unique(features.numericals, condition.get_feature().get_index());
}

template<typename FeatureTag, typename ObservationTag>
void collect_feature(ModuleFeatures& features, ygg::View<ygg::Index<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> effect)
{
    if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
        insert_unique(features.concepts, effect.get_feature().get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::BooleanFeature>)
        insert_unique(features.booleans, effect.get_feature().get_index());
    else if constexpr (std::same_as<FeatureTag, runir::kr::ps::dl::NumericalFeature>)
        insert_unique(features.numericals, effect.get_feature().get_index());
}

template<typename VariantView>
bool variant_references_concept_register(VariantView view, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> reg);

template<typename View>
bool references_concept_register(View view, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> reg)
{
    if constexpr (requires { view.get_variant(); })
    {
        return variant_references_concept_register(view.get_variant(), reg);
    }
    else
    {
        if constexpr (requires { view.get_data().identifier; })
        {
            using Identifier = std::remove_cvref_t<decltype(view.get_data().identifier)>;
            if constexpr (std::same_as<Identifier, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag>>)
                return view.get_data().identifier == reg;
        }

        auto result = false;
        if constexpr (requires { view.get_arg(); })
        {
            auto arg = view.get_arg();
            if constexpr (requires { arg.get_variant(); })
                result = result || references_concept_register(arg, reg);
            else
                result = result || variant_references_concept_register(arg, reg);
        }
        if constexpr (requires { view.get_lhs(); })
            result = result || references_concept_register(view.get_lhs(), reg);
        if constexpr (requires { view.get_rhs(); })
            result = result || references_concept_register(view.get_rhs(), reg);
        if constexpr (requires { view.get_mid(); })
            result = result || references_concept_register(view.get_mid(), reg);
        if constexpr (requires { view.get_role(); })
            result = result || references_concept_register(view.get_role(), reg);
        if constexpr (requires { view.get_concept(); })
            result = result || references_concept_register(view.get_concept(), reg);
        return result;
    }
}

template<typename VariantView>
bool variant_references_concept_register(VariantView view, runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> reg)
{
    return ygg::visit([&](auto child) { return references_concept_register(child, reg); }, view);
}

template<typename FeatureTag>
bool feature_references_concept_register(ygg::Index<Feature<FeatureTag>> feature,
                                         const Repository& repository,
                                         runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> reg)
{
    return ygg::visit([&](auto concrete) { return references_concept_register(concrete.get_expression(), reg); },
                      ygg::make_view(feature, repository).get_variant());
}

/// Per-rule qualitative profile. Numerical-like positions span the concept
/// features first, then the numerical features.
struct ModuleRuleProfile
{
    std::size_t source_memory_position = 0;
    std::size_t target_memory_position = 0;
    // Conditions over the source valuation.
    boost::dynamic_bitset<> boolean_positive_conditions;
    boost::dynamic_bitset<> boolean_negative_conditions;
    boost::dynamic_bitset<> numerical_greater_conditions;
    boost::dynamic_bitset<> numerical_zero_conditions;
    // Effects (sketch rules only; other kinds leave everything below empty,
    // i.e. unconstrained).
    boost::dynamic_bitset<> boolean_positive_effects;
    boost::dynamic_bitset<> boolean_negative_effects;
    boost::dynamic_bitset<> boolean_unchanged_effects;
    std::vector<NumericalChange> numerical_changes;

    ModuleRuleProfile(std::size_t num_booleans, std::size_t num_numerical_like) :
        boolean_positive_conditions(num_booleans),
        boolean_negative_conditions(num_booleans),
        numerical_greater_conditions(num_numerical_like),
        numerical_zero_conditions(num_numerical_like),
        boolean_positive_effects(num_booleans),
        boolean_negative_effects(num_booleans),
        boolean_unchanged_effects(num_booleans),
        numerical_changes(num_numerical_like, NumericalChange::UNCONSTRAINED)
    {
    }
};

inline void record_load_effects(const ModuleFeatures& features,
                                ModuleRuleProfile& profile,
                                const Repository& repository,
                                runir::kr::dl::RegisterIdentifier<runir::kr::dl::ConceptTag> reg)
{
    for (std::size_t position = 0; position < features.booleans.size(); ++position)
        if (!feature_references_concept_register(features.booleans[position], repository, reg))
            profile.boolean_unchanged_effects.set(position);

    for (std::size_t position = 0; position < features.concepts.size(); ++position)
        if (!feature_references_concept_register(features.concepts[position], repository, reg))
            profile.numerical_changes[position] = NumericalChange::UNCHANGED;

    for (std::size_t position = 0; position < features.numericals.size(); ++position)
        if (!feature_references_concept_register(features.numericals[position], repository, reg))
            profile.numerical_changes[features.concepts.size() + position] = NumericalChange::UNCHANGED;
}

template<typename FeatureTag, typename ObservationTag>
void record_condition(const ModuleFeatures& features,
                      ModuleRuleProfile& profile,
                      ygg::View<ygg::Index<ConcreteCondition<runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> condition)
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
    else
    {
        auto position = std::size_t { 0 };
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            position = feature_position(features.concepts, condition.get_feature().get_index());
        else
            position = features.concepts.size() + feature_position(features.numericals, condition.get_feature().get_index());
        if constexpr (std::same_as<ObservationTag, psdl::GreaterZero>)
            profile.numerical_greater_conditions.set(position);
        else if constexpr (std::same_as<ObservationTag, psdl::EqualZero>)
            profile.numerical_zero_conditions.set(position);
    }
}

template<typename FeatureTag, typename ObservationTag>
void record_effect(const ModuleFeatures& features,
                   ModuleRuleProfile& profile,
                   ygg::View<ygg::Index<ConcreteEffect<runir::kr::DlTag, FeatureTag, ObservationTag>>, Repository> effect)
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
    else
    {
        auto position = std::size_t { 0 };
        if constexpr (std::same_as<FeatureTag, runir::kr::dl::ConceptTag>)
            position = feature_position(features.concepts, effect.get_feature().get_index());
        else
            position = features.concepts.size() + feature_position(features.numericals, effect.get_feature().get_index());
        if constexpr (std::same_as<ObservationTag, psdl::Increases>)
            profile.numerical_changes[position] = NumericalChange::INCREASES;
        else if constexpr (std::same_as<ObservationTag, psdl::Decreases>)
            profile.numerical_changes[position] = NumericalChange::DECREASES;
        else if constexpr (std::same_as<ObservationTag, psdl::Unchanged>)
            profile.numerical_changes[position] = NumericalChange::UNCHANGED;
    }
}

struct ModulePolicyEdge
{
    std::size_t source;
    std::size_t target;
    std::size_t rule_position;
    bool alive = true;
};

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
        for (auto rule : transition)
        {
            analysis.rules.push_back(rule);
            ygg::visit([&](auto concrete_rule) { collect_rule_features(concrete_rule); }, rule.get_variant());
        }

    const auto num_booleans = analysis.features.booleans.size();
    const auto num_numerical_like = analysis.features.concepts.size() + analysis.features.numericals.size();
    for (auto rule : analysis.rules)
    {
        auto profile = ModuleRuleProfile(num_booleans, num_numerical_like);
        ygg::visit(
            [&](auto concrete_rule)
            {
                profile.source_memory_position = memory_position(concrete_rule.get_source().get_index());
                profile.target_memory_position = memory_position(concrete_rule.get_target().get_index());
                for (auto condition : concrete_rule.get_conditions())
                    ygg::visit([&](auto concrete_variant)
                               { ygg::visit([&](auto concrete) { record_condition(analysis.features, profile, concrete); }, concrete_variant.get_variant()); },
                               condition.get_variant());
                if constexpr (requires { concrete_rule.get_effects(); })
                {
                    for (auto effect : concrete_rule.get_effects())
                        ygg::visit([&](auto concrete_variant)
                                   { ygg::visit([&](auto concrete) { record_effect(analysis.features, profile, concrete); }, concrete_variant.get_variant()); },
                                   effect.get_variant());
                }
                if constexpr (std::same_as<std::remove_cvref_t<decltype(concrete_rule)>, ygg::View<ygg::Index<Rule<LoadTag>>, Repository>>)
                    record_load_effects(analysis.features, profile, module_.get_context(), concrete_rule.get_register().get_identifier());
                // Rules without explicit effect entries leave unmentioned
                // features unconstrained. Call rules have no effect entries.
            },
            rule.get_variant());
        analysis.profiles.push_back(std::move(profile));
    }

    return analysis;
}

/// Vertex ids encode valuation * num_memory_states + memory position; within
/// a valuation, Boolean feature i is at bit i and numerical-like feature j
/// at bit num_booleans + j.
inline boost::dynamic_bitset<> vertex_booleans(std::size_t vertex, std::size_t num_memory_states, std::size_t num_booleans)
{
    const auto valuation = vertex / num_memory_states;
    auto values = boost::dynamic_bitset<>(num_booleans);
    for (std::size_t position = 0; position < num_booleans; ++position)
        values.set(position, (valuation >> position) & std::size_t { 1 });
    return values;
}

inline boost::dynamic_bitset<>
vertex_numerical_like(std::size_t vertex, std::size_t num_memory_states, std::size_t num_booleans, std::size_t num_numerical_like)
{
    const auto valuation = vertex / num_memory_states;
    auto values = boost::dynamic_bitset<>(num_numerical_like);
    for (std::size_t position = 0; position < num_numerical_like; ++position)
        values.set(position, (valuation >> (num_booleans + position)) & std::size_t { 1 });
    return values;
}

inline std::size_t
make_vertex(const boost::dynamic_bitset<>& booleans, const boost::dynamic_bitset<>& numerical_like, std::size_t num_memory_states, std::size_t memory_position)
{
    auto valuation = std::size_t { 0 };
    for (std::size_t position = 0; position < booleans.size(); ++position)
        if (booleans.test(position))
            valuation |= std::size_t { 1 } << position;
    for (std::size_t position = 0; position < numerical_like.size(); ++position)
        if (numerical_like.test(position))
            valuation |= std::size_t { 1 } << (booleans.size() + position);
    return valuation * num_memory_states + memory_position;
}

/// Enumerate the extended policy graph edges of one rule by calling back
/// with (source vertex, target vertex).
template<typename Callback>
void enumerate_rule_edges(const ModuleRuleProfile& profile,
                          std::size_t num_memory_states,
                          std::size_t num_booleans,
                          std::size_t num_numerical_like,
                          Callback&& callback)
{
    const auto num_valuations = std::size_t { 1 } << (num_booleans + num_numerical_like);

    for (std::size_t source_valuation = 0; source_valuation < num_valuations; ++source_valuation)
    {
        const auto source = source_valuation * num_memory_states + profile.source_memory_position;
        const auto source_booleans = vertex_booleans(source, num_memory_states, num_booleans);
        const auto source_numericals = vertex_numerical_like(source, num_memory_states, num_booleans, num_numerical_like);

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

        auto target_numericals = boost::dynamic_bitset<>(num_numerical_like);
        auto fixed_numericals = boost::dynamic_bitset<>(num_numerical_like);
        auto decreases_unsatisfiable = false;
        for (std::size_t position = 0; position < num_numerical_like; ++position)
        {
            switch (profile.numerical_changes[position])
            {
                case NumericalChange::INCREASES:
                {
                    target_numericals.set(position);
                    fixed_numericals.set(position);
                    break;
                }
                case NumericalChange::DECREASES:
                {
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
                case NumericalChange::UNCONSTRAINED:
                    break;
            }
        }
        if (decreases_unsatisfiable)
            continue;

        auto free_positions = std::vector<std::pair<bool, std::size_t>> {};  // (is_boolean, position)
        for (std::size_t position = 0; position < num_booleans; ++position)
            if (!fixed_booleans.test(position))
                free_positions.emplace_back(true, position);
        for (std::size_t position = 0; position < num_numerical_like; ++position)
            if (!fixed_numericals.test(position))
                free_positions.emplace_back(false, position);

        for (std::size_t assignment = 0; assignment < (std::size_t { 1 } << free_positions.size()); ++assignment)
        {
            for (std::size_t free = 0; free < free_positions.size(); ++free)
            {
                const auto [is_boolean, position] = free_positions[free];
                const auto value = static_cast<bool>((assignment >> free) & std::size_t { 1 });
                (is_boolean ? target_booleans : target_numericals).set(position, value);
            }
            callback(source, make_vertex(target_booleans, target_numericals, num_memory_states, profile.target_memory_position));
        }
    }
}

/// Iterative per-component sieve over the given edges (sieve.pdf Algorithm 1;
/// see the file comment of base/dl/structural_termination.hpp for the
/// equivalence with the recursive formulation). Returns whether a cycle
/// survives, together with the final component assignment.
template<typename VertexBooleans, typename EdgeChanges>
std::pair<bool, std::vector<std::size_t>> sieve(std::vector<ModulePolicyEdge>& edges,
                                                std::size_t num_vertices,
                                                std::size_t num_booleans,
                                                std::size_t num_numerical_like,
                                                VertexBooleans&& vertex_booleans_of,
                                                EdgeChanges&& numerical_changes_of)
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
                builder.add_directed_edge(static_cast<graphs::VertexIndex>(edge.source), static_cast<graphs::VertexIndex>(edge.target));
        const auto graph = graphs::StaticGraph<> { std::move(builder) };
        const auto [num_components, components] = graphs::algorithms::strong_components(graph);
        for (std::size_t vertex = 0; vertex < num_vertices; ++vertex)
            component_of[vertex] = components[vertex];

        struct ComponentProfile
        {
            boost::dynamic_bitset<> numerical_increased_or_unconstrained;
            boost::dynamic_bitset<> boolean_flipped_to_true;
            boost::dynamic_bitset<> boolean_flipped_to_false;

            ComponentProfile(std::size_t num_booleans, std::size_t num_numerical_like) :
                numerical_increased_or_unconstrained(num_numerical_like),
                boolean_flipped_to_true(num_booleans),
                boolean_flipped_to_false(num_booleans)
            {
            }
        };
        auto component_profiles = std::vector<ComponentProfile>(static_cast<std::size_t>(num_components), ComponentProfile(num_booleans, num_numerical_like));

        for (const auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            auto& component = component_profiles[component_of[edge.source]];
            const auto& changes = numerical_changes_of(edge);
            for (std::size_t position = 0; position < changes.size(); ++position)
                if (changes[position] == NumericalChange::INCREASES || changes[position] == NumericalChange::UNCONSTRAINED)
                    component.numerical_increased_or_unconstrained.set(position);
            const auto source_booleans = vertex_booleans_of(edge.source);
            const auto target_booleans = vertex_booleans_of(edge.target);
            component.boolean_flipped_to_true |= target_booleans - source_booleans;
            component.boolean_flipped_to_false |= source_booleans - target_booleans;
        }

        for (auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            const auto& component = component_profiles[component_of[edge.source]];

            auto removable = false;
            const auto& changes = numerical_changes_of(edge);
            for (std::size_t position = 0; position < changes.size() && !removable; ++position)
                if (changes[position] == NumericalChange::DECREASES && !component.numerical_increased_or_unconstrained.test(position))
                    removable = true;
            const auto source_booleans = vertex_booleans_of(edge.source);
            const auto target_booleans = vertex_booleans_of(edge.target);
            if (((source_booleans - target_booleans) - component.boolean_flipped_to_true).any())
                removable = true;
            if (((target_booleans - source_booleans) - component.boolean_flipped_to_false).any())
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

}  // namespace detail

inline ModuleStructuralTerminationResult structural_termination(ModuleView module_)
{
    auto analysis = detail::analyze_module(module_);
    const auto& memory_states = analysis.memory_states;
    const auto& rules = analysis.rules;
    const auto& features = analysis.features;
    const auto& profiles = analysis.profiles;

    auto result = ModuleStructuralTerminationResult {};
    result.concepts = features.concepts;
    result.booleans = features.booleans;
    result.numericals = features.numericals;

    const auto num_memory_states = memory_states.size();
    if (num_memory_states == 0)
        return result;  // no memory states, no rules, trivially terminating

    const auto num_concepts = features.concepts.size();
    const auto num_booleans = features.booleans.size();
    const auto num_numerical_like = num_concepts + features.numericals.size();
    if (num_booleans + num_numerical_like > 14)
        throw std::invalid_argument("structural_termination: too many features; the policy graph has 2^|features| * |memory| vertices.");

    auto edges = std::vector<detail::ModulePolicyEdge> {};
    for (std::size_t rule_position = 0; rule_position < profiles.size(); ++rule_position)
        detail::enumerate_rule_edges(profiles[rule_position],
                                     num_memory_states,
                                     num_booleans,
                                     num_numerical_like,
                                     [&](std::size_t source, std::size_t target)
                                     { edges.push_back(detail::ModulePolicyEdge { source, target, rule_position }); });

    const auto num_vertices = (std::size_t { 1 } << (num_booleans + num_numerical_like)) * num_memory_states;
    const auto [has_cycle, component_of] = detail::sieve(
        edges,
        num_vertices,
        num_booleans,
        num_numerical_like,
        [&](std::size_t vertex) { return detail::vertex_booleans(vertex, num_memory_states, num_booleans); },
        [&](const detail::ModulePolicyEdge& edge) -> const std::vector<NumericalChange>& { return profiles[edge.rule_position].numerical_changes; });

    if (!has_cycle)
    {
        result.status = StructuralTerminationStatus::TERMINATING;
        return result;
    }

    result.status = StructuralTerminationStatus::NON_TERMINATING;
    auto counterexample_builder = ModulePolicyGraphBuilder {};
    auto vertex_remap = std::vector<std::size_t>(num_vertices, std::numeric_limits<std::size_t>::max());
    const auto map_vertex = [&](std::size_t vertex)
    {
        if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
        {
            const auto numerical_like = detail::vertex_numerical_like(vertex, num_memory_states, num_booleans, num_numerical_like);
            auto concepts = boost::dynamic_bitset<>(num_concepts);
            auto numericals = boost::dynamic_bitset<>(features.numericals.size());
            for (std::size_t position = 0; position < num_concepts; ++position)
                concepts.set(position, numerical_like.test(position));
            for (std::size_t position = 0; position < features.numericals.size(); ++position)
                numericals.set(position, numerical_like.test(num_concepts + position));
            vertex_remap[vertex] =
                counterexample_builder.add_vertex(ModulePolicyGraphVertexLabel(std::move(concepts),
                                                                               detail::vertex_booleans(vertex, num_memory_states, num_booleans),
                                                                               std::move(numericals),
                                                                               memory_states[vertex % num_memory_states]));
        }
        return static_cast<graphs::VertexIndex>(vertex_remap[vertex]);
    };
    for (const auto& edge : edges)
    {
        if (!edge.alive || component_of[edge.source] != component_of[edge.target])
            continue;
        counterexample_builder.add_directed_edge(map_vertex(edge.source),
                                                 map_vertex(edge.target),
                                                 ModulePolicyGraphEdgeLabel(rules[edge.rule_position], profiles[edge.rule_position].numerical_changes));
    }
    result.counterexample = std::make_shared<ModulePolicyGraph>(std::move(counterexample_builder));
    return result;
}

inline ModuleProgramStructuralTerminationResult structural_termination(ModuleProgramView program)
{
    auto result = ModuleProgramStructuralTerminationResult {};

    auto modules = std::vector<ModuleView> {};
    for (auto module : program.get_modules())
    {
        modules.push_back(module);
        auto module_result = structural_termination(module);
        if (!module_result.is_terminating())
            result.status = StructuralTerminationStatus::NON_TERMINATING;
        result.module_results.push_back(std::move(module_result));
    }

    const auto module_position_by_name = [&](const std::string& name) -> std::optional<std::size_t>
    {
        for (std::size_t position = 0; position < modules.size(); ++position)
            if (std::string(modules[position].get_name()) == name)
                return position;
        return std::nullopt;
    };

    auto call_graph_builder = graphs::StaticGraphBuilder<> {};
    for (std::size_t position = 0; position < modules.size(); ++position)
        call_graph_builder.add_vertex();

    struct CallEdge
    {
        std::size_t source_module;
        std::size_t target_module;
        RuleVariantView rule;
    };
    auto call_edges = std::vector<CallEdge> {};

    for (std::size_t source_module = 0; source_module < modules.size(); ++source_module)
    {
        for (auto transition : modules[source_module].get_memory_transitions())
        {
            for (auto rule_variant : transition)
            {
                ygg::visit(
                    [&](auto rule)
                    {
                        using RuleViewT = std::decay_t<decltype(rule)>;
                        if constexpr (std::same_as<RuleViewT, ygg::View<ygg::Index<Rule<CallTag>>, Repository>>)
                        {
                            const auto target_module = module_position_by_name(rule.get_callee_name());
                            if (!target_module)
                            {
                                result.status = StructuralTerminationStatus::NON_TERMINATING;
                                result.recursive_call_rules.push_back(rule_variant);
                                return;
                            }
                            call_graph_builder.add_directed_edge(static_cast<graphs::VertexIndex>(source_module),
                                                                 static_cast<graphs::VertexIndex>(*target_module));
                            call_edges.push_back(CallEdge { source_module, *target_module, rule_variant });
                        }
                    },
                    rule_variant.get_variant());
            }
        }
    }

    const auto call_graph = graphs::StaticGraph<> { std::move(call_graph_builder) };
    const auto [num_components, component_of] = graphs::algorithms::strong_components(call_graph);
    static_cast<void>(num_components);
    for (const auto& edge : call_edges)
    {
        if (component_of[edge.source_module] == component_of[edge.target_module])
        {
            result.status = StructuralTerminationStatus::NON_TERMINATING;
            result.recursive_call_rules.push_back(edge.rule);
        }
    }

    return result;
}

}  // namespace runir::kr::ps::ext::dl

#endif
