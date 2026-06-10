#ifndef RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_HPP_
#define RUNIR_KR_PS_BASE_DL_STRUCTURAL_TERMINATION_HPP_

#include "runir/config.hpp"
#include "runir/graphs/algorithms.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/base/dl/condition_view.hpp"
#include "runir/kr/ps/base/dl/effect_view.hpp"
#include "runir/kr/ps/base/dl/feature_view.hpp"
#include "runir/kr/ps/base/dl/syntactic_complexity.hpp"
#include "runir/kr/ps/base/repository.hpp"
#include "runir/kr/ps/base/rule_view.hpp"
#include "runir/kr/ps/base/sketch_view.hpp"

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

namespace runir::kr::ps::base::dl
{

/**
 * Structural termination of a sketch via the Sieve algorithm.
 *
 * Implements Algorithm 1 ("Sieve Algorithm") of sieve.pdf on the policy graph
 * G(R) of the sketch (Srivastava et al. 2011), extended to Boolean features
 * as described in incomplete_sieve.pdf Section 5.1: G(R) has one node per
 * Boolean feature valuation over the sketch's features (numerical features n
 * are abstracted to the Boolean condition n > 0), and an edge b -> b' labeled
 * with rule r iff (b, b') is compatible with r under the qualitative
 * semantics (sieve.pdf Theorem 1):
 *   - conditions constrain b: positive/negative for Booleans,
 *     greater_zero/equal_zero for numericals;
 *   - effects constrain b': Boolean positive/negative force the target bit,
 *     unchanged copies the source bit; numerical increases forces n > 0 in
 *     b' (since n' > n >= 0), decreases requires n > 0 in b (n is
 *     well-founded downward) and leaves the target bit free, unchanged
 *     copies the bit; features unmentioned in the effects are unconstrained
 *     (incomplete_sieve.pdf Section 5.1: "n?").
 *
 * The sieve repeatedly removes, within each strongly connected component, the
 * edges that decrease a numerical feature that no edge in the component
 * increases or leaves unconstrained, and the edges that flip a Boolean
 * feature whose opposite flip occurs on no edge of the component. The sketch
 * is terminating iff the surviving graph is acyclic (every strongly connected
 * component is trivial). The iterative per-component fixpoint used here is
 * equivalent to the recursive formulation of Algorithm 1: an edge is removed
 * exactly when its current component contains no opposing edge.
 *
 * On failure, the result carries the union of the surviving non-trivial
 * strongly connected components as a counterexample graph: the cycles that
 * the sieve could not break.
 */

enum class StructuralTerminationStatus
{
    TERMINATING,
    NON_TERMINATING,
};

/// Qualitative change of one numerical feature along a policy graph edge.
enum class NumericalChange : std::uint8_t
{
    UNCONSTRAINED = 0,
    INCREASES = 1,
    DECREASES = 2,
    UNCHANGED = 3,
};

/// Feature valuation; bit i corresponds to position i in the result's
/// boolean (resp. numerical) feature list. A numerical bit encodes n > 0.
struct PolicyGraphVertexLabel
{
    std::uint32_t boolean_values;
    std::uint32_t numerical_values;

    PolicyGraphVertexLabel(std::uint32_t boolean_values_, std::uint32_t numerical_values_) noexcept :
        boolean_values(boolean_values_),
        numerical_values(numerical_values_)
    {
    }

    auto identifying_members() const noexcept { return std::tie(boolean_values, numerical_values); }
};

/// Rule labeling an edge together with the qualitative numerical changes
/// (2 bits per numerical feature, NumericalChange encoded).
struct PolicyGraphEdgeLabel
{
    RuleView rule;
    std::uint64_t numerical_changes;

    PolicyGraphEdgeLabel(RuleView rule_, std::uint64_t numerical_changes_) noexcept : rule(rule_), numerical_changes(numerical_changes_) {}

    NumericalChange get_numerical_change(std::size_t position) const noexcept
    {
        return static_cast<NumericalChange>((numerical_changes >> (2 * position)) & std::uint64_t { 3 });
    }

    auto identifying_members() const noexcept { return std::tie(rule, numerical_changes); }
};

using PolicyGraphBuilder = graphs::StaticGraphBuilder<PolicyGraphVertexLabel, PolicyGraphEdgeLabel>;
using PolicyGraph = graphs::StaticGraph<PolicyGraphVertexLabel, PolicyGraphEdgeLabel>;

struct StructuralTerminationResult
{
    StructuralTerminationStatus status = StructuralTerminationStatus::TERMINATING;
    std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::BooleanFeature>>> booleans;
    std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, runir::kr::ps::dl::NumericalFeature>>> numericals;
    std::shared_ptr<PolicyGraph> counterexample;  ///< nullptr iff terminating; else the surviving non-trivial SCCs.

    bool is_terminating() const noexcept { return status == StructuralTerminationStatus::TERMINATING; }
};

namespace detail
{

/// Per-rule qualitative profile over the feature positions.
struct RuleProfile
{
    // Conditions over the source valuation.
    std::uint32_t boolean_positive_conditions = 0;
    std::uint32_t boolean_negative_conditions = 0;
    std::uint32_t numerical_greater_conditions = 0;
    std::uint32_t numerical_zero_conditions = 0;
    // Effects.
    std::uint32_t boolean_positive_effects = 0;
    std::uint32_t boolean_negative_effects = 0;
    std::uint32_t boolean_unchanged_effects = 0;
    std::uint64_t numerical_changes = 0;  // 2 bits per numerical position (NumericalChange).
};

template<typename FeatureTag>
std::size_t feature_position(const std::vector<ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>>>& features,
                             ygg::Index<runir::kr::ps::Feature<runir::kr::BaseFamilyTag, FeatureTag>> feature)
{
    return static_cast<std::size_t>(std::distance(features.begin(), std::find(features.begin(), features.end(), feature)));
}

template<typename FeatureTag, typename ObservationTag>
void record_condition(const FeatureSyntacticComplexityContext& features,
                      RuleProfile& profile,
                      ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>,
                                Repository> condition)
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
    else if constexpr (std::same_as<FeatureTag, psdl::NumericalFeature>)
    {
        const auto bit = std::uint32_t { 1 } << feature_position(features.numericals, condition.get_feature().get_index());
        if constexpr (std::same_as<ObservationTag, psdl::GreaterZero>)
            profile.numerical_greater_conditions |= bit;
        else if constexpr (std::same_as<ObservationTag, psdl::EqualZero>)
            profile.numerical_zero_conditions |= bit;
    }
}

template<typename FeatureTag, typename ObservationTag>
void record_effect(const FeatureSyntacticComplexityContext& features,
                   RuleProfile& profile,
                   ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<runir::kr::BaseFamilyTag, runir::kr::DlTag, FeatureTag, ObservationTag>>,
                             Repository> effect)
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
    else if constexpr (std::same_as<FeatureTag, psdl::NumericalFeature>)
    {
        const auto position = feature_position(features.numericals, effect.get_feature().get_index());
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

inline RuleProfile make_rule_profile(const FeatureSyntacticComplexityContext& features, RuleView rule)
{
    auto profile = RuleProfile {};
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
    std::uint32_t source;
    std::uint32_t target;
    std::uint32_t rule_position;
    std::uint64_t numerical_changes;
    bool alive = true;
};

/// Enumerate the policy graph edges of one rule. Valuations pack Boolean
/// bits in the low `num_booleans` bits and numerical (n > 0) bits above.
template<typename Callback>
void enumerate_rule_edges(const RuleProfile& profile, std::size_t num_booleans, std::size_t num_numericals, Callback&& callback)
{
    const auto num_features = num_booleans + num_numericals;
    const auto num_valuations = std::uint64_t { 1 } << num_features;

    for (std::uint64_t source = 0; source < num_valuations; ++source)
    {
        const auto source_booleans = static_cast<std::uint32_t>(source) & ((std::uint32_t { 1 } << num_booleans) - 1);
        const auto source_numericals = static_cast<std::uint32_t>(source >> num_booleans);

        // Conditions.
        if ((source_booleans & profile.boolean_positive_conditions) != profile.boolean_positive_conditions)
            continue;
        if ((~source_booleans & profile.boolean_negative_conditions) != profile.boolean_negative_conditions)
            continue;
        if ((source_numericals & profile.numerical_greater_conditions) != profile.numerical_greater_conditions)
            continue;
        if ((~source_numericals & profile.numerical_zero_conditions) != profile.numerical_zero_conditions)
            continue;

        // Effects: compute forced target bits and the set of free positions.
        auto forced_booleans = std::uint32_t { 0 };
        auto fixed_boolean_mask = std::uint32_t { 0 };
        forced_booleans |= profile.boolean_positive_effects;
        fixed_boolean_mask |= profile.boolean_positive_effects | profile.boolean_negative_effects | profile.boolean_unchanged_effects;
        forced_booleans |= profile.boolean_unchanged_effects & source_booleans;

        auto forced_numericals = std::uint32_t { 0 };
        auto fixed_numerical_mask = std::uint32_t { 0 };
        auto decreases_unsatisfiable = false;
        for (std::size_t position = 0; position < num_numericals; ++position)
        {
            const auto bit = std::uint32_t { 1 } << position;
            switch (static_cast<NumericalChange>((profile.numerical_changes >> (2 * position)) & std::uint64_t { 3 }))
            {
                case NumericalChange::INCREASES:
                {
                    // n' > n >= 0 implies n' > 0.
                    forced_numericals |= bit;
                    fixed_numerical_mask |= bit;
                    break;
                }
                case NumericalChange::DECREASES:
                {
                    // n' < n requires n > 0 in the source; the target bit is free.
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

        const auto free_boolean_mask = ~fixed_boolean_mask & ((num_booleans ? (std::uint32_t { 1 } << num_booleans) : 1u) - 1);
        const auto free_numerical_mask = ~fixed_numerical_mask & ((num_numericals ? (std::uint32_t { 1 } << num_numericals) : 1u) - 1);

        // Enumerate the free bits via subset enumeration.
        auto boolean_subset = std::uint32_t { 0 };
        while (true)
        {
            auto numerical_subset = std::uint32_t { 0 };
            while (true)
            {
                const auto target_booleans = forced_booleans | boolean_subset;
                const auto target_numericals = forced_numericals | numerical_subset;
                const auto target = static_cast<std::uint64_t>(target_booleans) | (static_cast<std::uint64_t>(target_numericals) << num_booleans);
                callback(static_cast<std::uint32_t>(source), static_cast<std::uint32_t>(target));

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

}  // namespace detail

inline StructuralTerminationResult structural_termination(SketchView sketch)
{
    auto features = FeatureSyntacticComplexityContext {};
    collect_features(features, sketch);

    const auto num_booleans = features.booleans.size();
    const auto num_numericals = features.numericals.size();
    if (num_booleans > 32 || num_numericals > 32 || num_booleans + num_numericals > 16)
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
    {
        detail::enumerate_rule_edges(profiles[rule_position],
                                     num_booleans,
                                     num_numericals,
                                     [&](std::uint32_t source, std::uint32_t target) {
                                         edges.push_back(detail::PolicyEdge { source,
                                                                              target,
                                                                              static_cast<std::uint32_t>(rule_position),
                                                                              profiles[rule_position].numerical_changes });
                                     });
    }

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
                builder.add_directed_edge(edge.source, edge.target);
        const auto graph = graphs::StaticGraph<> { std::move(builder) };
        const auto [num_components, components] = graphs::algorithms::strong_components(graph);
        for (std::size_t vertex = 0; vertex < num_valuations; ++vertex)
            component_of[vertex] = components[vertex];

        // Per component: which numerical features some surviving intra-component
        // edge increases or leaves unconstrained, and which Boolean flips occur.
        struct ComponentProfile
        {
            std::uint32_t numerical_increased_or_unconstrained = 0;
            std::uint32_t boolean_flipped_to_true = 0;
            std::uint32_t boolean_flipped_to_false = 0;
            bool has_intra_edge = false;
        };
        auto component_profiles = std::vector<ComponentProfile>(static_cast<std::size_t>(num_components));

        const auto boolean_mask = (num_booleans ? (std::uint32_t { 1 } << num_booleans) : 1u) - 1;
        for (const auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            auto& profile = component_profiles[component_of[edge.source]];
            profile.has_intra_edge = true;
            for (std::size_t position = 0; position < num_numericals; ++position)
            {
                const auto change = static_cast<NumericalChange>((edge.numerical_changes >> (2 * position)) & std::uint64_t { 3 });
                if (change == NumericalChange::INCREASES || change == NumericalChange::UNCONSTRAINED)
                    profile.numerical_increased_or_unconstrained |= std::uint32_t { 1 } << position;
            }
            const auto source_booleans = edge.source & boolean_mask;
            const auto target_booleans = edge.target & boolean_mask;
            profile.boolean_flipped_to_true |= ~source_booleans & target_booleans;
            profile.boolean_flipped_to_false |= source_booleans & ~target_booleans;
        }

        // Remove intra-component edges that decrease a numerical feature that
        // no edge of the component increases or leaves unconstrained, and
        // edges that flip a Boolean whose opposite flip never occurs in the
        // component.
        for (auto& edge : edges)
        {
            if (!edge.alive || component_of[edge.source] != component_of[edge.target])
                continue;
            const auto& profile = component_profiles[component_of[edge.source]];

            auto removable = false;
            for (std::size_t position = 0; position < num_numericals && !removable; ++position)
            {
                const auto change = static_cast<NumericalChange>((edge.numerical_changes >> (2 * position)) & std::uint64_t { 3 });
                if (change == NumericalChange::DECREASES && !(profile.numerical_increased_or_unconstrained & (std::uint32_t { 1 } << position)))
                    removable = true;
            }
            const auto source_booleans = edge.source & boolean_mask;
            const auto target_booleans = edge.target & boolean_mask;
            if ((source_booleans & ~target_booleans & ~profile.boolean_flipped_to_true) != 0)
                removable = true;  // flips some p to false; no edge in the component flips p to true
            if ((~source_booleans & target_booleans & ~profile.boolean_flipped_to_false) != 0)
                removable = true;  // flips some p to true; no edge in the component flips p to false

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
    auto vertex_remap = std::vector<std::uint32_t>(num_valuations, std::numeric_limits<std::uint32_t>::max());
    const auto boolean_mask = (num_booleans ? (std::uint32_t { 1 } << num_booleans) : 1u) - 1;
    const auto map_vertex = [&](std::uint32_t valuation)
    {
        if (vertex_remap[valuation] == std::numeric_limits<std::uint32_t>::max())
            vertex_remap[valuation] = counterexample_builder.add_vertex(
                PolicyGraphVertexLabel(valuation & boolean_mask, static_cast<std::uint32_t>(valuation >> num_booleans)));
        return vertex_remap[valuation];
    };
    for (const auto& edge : edges)
    {
        if (!edge.alive || component_of[edge.source] != component_of[edge.target])
            continue;
        counterexample_builder.add_directed_edge(map_vertex(edge.source),
                                                 map_vertex(edge.target),
                                                 PolicyGraphEdgeLabel(rules[edge.rule_position], edge.numerical_changes));
    }
    result.counterexample = std::make_shared<PolicyGraph>(std::move(counterexample_builder));
    return result;
}

}  // namespace runir::kr::ps::base::dl

#if RUNIR_ENABLE_FMT_FORMATTERS
namespace fmt
{

template<>
struct formatter<runir::kr::ps::base::dl::NumericalChange, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(runir::kr::ps::base::dl::NumericalChange change, FormatContext& ctx) const
    {
        using runir::kr::ps::base::dl::NumericalChange;
        auto text = std::string_view { "?" };
        switch (change)
        {
            case NumericalChange::UNCONSTRAINED: text = "?"; break;
            case NumericalChange::INCREASES: text = "inc"; break;
            case NumericalChange::DECREASES: text = "dec"; break;
            case NumericalChange::UNCHANGED: text = "unchanged"; break;
        }
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::kr::ps::base::dl::PolicyGraphVertexLabel, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::PolicyGraphVertexLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("(booleans={:#b}, numericals={:#b})", label.boolean_values, label.numerical_values);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::kr::ps::base::dl::PolicyGraphEdgeLabel, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::PolicyGraphEdgeLabel& label, FormatContext& ctx) const
    {
        const auto text = fmt::format("(rule={}, numerical_changes={:#x})", label.rule.get_symbol(), label.numerical_changes);
        return formatter<std::string_view>::format(text, ctx);
    }
};

template<>
struct formatter<runir::kr::ps::base::dl::StructuralTerminationResult, char> : formatter<std::string_view>
{
    template<typename FormatContext>
    auto format(const runir::kr::ps::base::dl::StructuralTerminationResult& result, FormatContext& ctx) const
    {
        const auto text = result.is_terminating() ?
                              std::string { "StructuralTerminationResult(terminating)" } :
                              fmt::format("StructuralTerminationResult(non-terminating, counterexample with {} vertices and {} edges)",
                                          result.counterexample->get_num_vertices(),
                                          result.counterexample->get_num_edges());
        return formatter<std::string_view>::format(text, ctx);
    }
};

}  // namespace fmt
#endif

#endif
