#include "runir/kr/ps/dl/structural_termination.hpp"

#include "runir/datasets/graph_types.hpp"
#include "structural_termination/detail.hpp"

#include <algorithm>
#include <bit>
#include <limits>
#include <set>
#include <string>
#include <tuple>
#include <urd/bits.hpp>
#include <urd/repr/graphs/adjacency_lists.hpp>
#include <urd/repr/maps/index_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/dense_set.hpp>
#include <urd/repr/values/std_set.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vefr/algorithms.hpp>
#include <yggdrasil/semantics/canonicalization.hpp>

namespace runir::kr::ps::detail
{

namespace
{

void require_mask(std::size_t rule_position, std::string_view field, std::uint64_t mask, std::size_t num_features)
{
    if (std::cmp_greater(std::bit_width(mask), num_features))
        throw std::invalid_argument("structural_termination: rule " + std::to_string(rule_position) + " has an out-of-range feature in " + std::string(field));
}

void validate_effects(std::size_t rule_position,
                      std::string_view feature_kind,
                      std::size_t num_features,
                      std::uint64_t first,
                      std::uint64_t second,
                      std::uint64_t unchanged,
                      std::uint64_t unconstrained)
{
    if ((first & second) || ((first | second) & unchanged) || ((first | second | unchanged) & unconstrained))
        throw std::invalid_argument("structural_termination: rule " + std::to_string(rule_position) + " has contradictory " + std::string(feature_kind)
                                    + " effects");
    auto expected = std::uint64_t { 0 };
    urd::bitset_span<std::uint64_t>(&expected, num_features).set();
    if ((first | second | unchanged | unconstrained) != expected)
        throw std::invalid_argument("structural_termination: rule " + std::to_string(rule_position) + " has missing " + std::string(feature_kind) + " effects");
}

}  // namespace

void validate_policy(const QualitativePolicy& policy)
{
    policy.validate_feature_capacity();
    if (policy.num_memory_states == 0)
        throw std::invalid_argument("structural_termination: a qualitative policy requires at least one memory state");
    if (policy.num_memory_states > std::numeric_limits<VertexIndex>::max())
        throw std::invalid_argument("structural_termination: the policy has too many memory states");
    if (policy.rule_profiles.size() > std::numeric_limits<EdgeIndex>::max())
        throw std::invalid_argument("structural_termination: the policy has too many rules");

    for (std::size_t position = 0; position < policy.rule_profiles.size(); ++position)
    {
        const auto& profile = policy.rule_profiles[position];
        if (profile.source_memory_position >= policy.num_memory_states || profile.target_memory_position >= policy.num_memory_states)
            throw std::invalid_argument("structural_termination: rule " + std::to_string(position) + " has an out-of-range memory position");

        require_mask(position, "Boolean-positive condition", profile.boolean_positive_conditions, policy.num_booleans);
        require_mask(position, "Boolean-negative condition", profile.boolean_negative_conditions, policy.num_booleans);
        require_mask(position, "numerical-greater condition", profile.numerical_greater_conditions, policy.num_numericals);
        require_mask(position, "numerical-zero condition", profile.numerical_zero_conditions, policy.num_numericals);
        require_mask(position, "Boolean-positive effect", profile.boolean_positive_effects, policy.num_booleans);
        require_mask(position, "Boolean-negative effect", profile.boolean_negative_effects, policy.num_booleans);
        require_mask(position, "Boolean-unchanged effect", profile.boolean_unchanged_effects, policy.num_booleans);
        require_mask(position, "Boolean-unconstrained effect", profile.boolean_unconstrained_effects, policy.num_booleans);
        require_mask(position, "numerical-increase effect", profile.numerical_increase_effects, policy.num_numericals);
        require_mask(position, "numerical-decrease effect", profile.numerical_decrease_effects, policy.num_numericals);
        require_mask(position, "numerical-unchanged effect", profile.numerical_unchanged_effects, policy.num_numericals);
        require_mask(position, "numerical-unconstrained effect", profile.numerical_unconstrained_effects, policy.num_numericals);

        if (profile.boolean_positive_conditions & profile.boolean_negative_conditions)
            throw std::invalid_argument("structural_termination: rule " + std::to_string(position) + " has contradictory Boolean conditions");
        if (profile.numerical_greater_conditions & profile.numerical_zero_conditions)
            throw std::invalid_argument("structural_termination: rule " + std::to_string(position) + " has contradictory numerical conditions");
        validate_effects(position,
                         "Boolean",
                         policy.num_booleans,
                         profile.boolean_positive_effects,
                         profile.boolean_negative_effects,
                         profile.boolean_unchanged_effects,
                         profile.boolean_unconstrained_effects);
        validate_effects(position,
                         "numerical",
                         policy.num_numericals,
                         profile.numerical_increase_effects,
                         profile.numerical_decrease_effects,
                         profile.numerical_unchanged_effects,
                         profile.numerical_unconstrained_effects);
    }
}

}  // namespace runir::kr::ps::detail

namespace runir::kr::ps::dl
{

namespace
{

CounterexampleVertex materialize_vertex(std::size_t vertex, const detail::ProjectedPolicyComponent& projected)
{
    auto [boolean_values, numerical_values] = detail::unproject_vertex(vertex, projected);

    return CounterexampleVertex {
        projected.memory_positions[vertex % projected.policy.num_memory_states],
        boolean_values,
        numerical_values,
    };
}

urd::Store<urd::sequence<CounterexampleComponent>> materialize_counterexamples(const detail::PolicySieveResult& sieve_result)
{
    auto result = urd::Store<urd::sequence<CounterexampleComponent>> {};
    for (const auto& component : sieve_result.components)
    {
        const auto& projected = component.projected;
        const auto first_result = result.size();
        auto cyclic_scc = urd::Store<urd::set<std::size_t>, urd::dense> {};
        auto result_of_scc = urd::Store<urd::map<std::size_t, std::size_t>, urd::dense> {};
        auto vertex_remap = urd::Store<urd::map<std::size_t, std::size_t>, urd::dense> {};
        vertex_remap.reserve(component.sieve.component_of.size());

        for (const auto& edge : component.edges)
            cyclic_scc.insert(component.sieve.component_of[edge.source]);

        for (std::size_t vertex = 0; vertex < component.sieve.component_of.size(); ++vertex)
        {
            const auto scc = component.sieve.component_of[vertex];
            if (!cyclic_scc.contains(scc))
                continue;
            const auto [entry, inserted] = result_of_scc.try_emplace(scc, result.size());
            if (inserted)
                result.push_back(CounterexampleComponent {
                    .memory_positions = {},
                    .boolean_positions = projected.boolean_positions,
                    .numerical_positions = projected.numerical_positions,
                    .vertices = {},
                    .edges = {},
                });
            auto& counterexample = result[entry->second];
            vertex_remap.emplace(vertex, counterexample.vertices.size());
            counterexample.vertices.push_back(materialize_vertex(vertex, projected));
            counterexample.memory_positions.push_back(counterexample.vertices.back().memory_position);
        }

        for (const auto& edge : component.edges)
        {
            auto& counterexample = result[result_of_scc.at(component.sieve.component_of[edge.source])];
            counterexample.edges.push_back(CounterexampleEdge {
                vertex_remap.at(edge.source),
                vertex_remap.at(edge.target),
                projected.rule_positions[edge.rule_position],
            });
        }

        for (auto result_position = first_result; result_position < result.size(); ++result_position)
        {
            ygg::canonicalize(result[result_position].memory_positions);
            std::ranges::sort(result[result_position].edges,
                              {},
                              [](const auto& edge) { return std::tuple(edge.source_vertex_position, edge.target_vertex_position, edge.rule_position); });
        }
    }
    if (!sieve_result.components.empty() && result.empty())
        throw std::logic_error("structural_termination: a cyclic SIEVE component produced no counterexample");
    return result;
}

}  // namespace

urd::Store<urd::sequence<std::size_t>> IncompletePolicyResult::get_cyclic_rule_positions() const
{
    auto result = urd::Store<urd::sequence<std::size_t>> {};
    result.reserve(surviving_rules.size());
    for (const auto& rule : surviving_rules)
        result.push_back(rule.rule_position);
    return result;
}

urd::Store<urd::sequence<std::size_t>> StructuralTerminationResult::get_cyclic_rule_positions() const
{
    auto result = urd::Store<urd::set<std::size_t>, urd::sorted, urd::stable_refs> {};
    for (const auto& component : counterexample_components)
        for (const auto& edge : component.edges)
            result.insert(edge.rule_position);
    return { result.begin(), result.end() };
}

urd::Store<urd::sequence<std::size_t>> CounterexampleComponent::get_cycle() const
{
    // A growing graph numbers its edges as added: an edge's number is its position in `edges`.
    auto graph =
        urd::Store<urd::graph<VertexIndex>, urd::monotone, urd::positional>(urd::graph_data<std::pair<VertexIndex, VertexIndex>> { vertices.size(), {}, {} });
    for (const auto& edge : edges)
        graph.add_edge(static_cast<VertexIndex>(edge.source_vertex_position), static_cast<VertexIndex>(edge.target_vertex_position));
    auto result = urd::Store<urd::sequence<std::size_t>> {};
    for (const auto& edge : vefr::find_cycle(graph))
        result.push_back(edge.id);
    return result;
}

IncompletePolicyResult incomplete_structural_termination(const QualitativePolicy& policy, bool use_memory_scc_scope)
{
    detail::validate_policy(policy);
    return detail::incomplete_structural_termination(policy, use_memory_scc_scope);
}

StructuralTerminationResult
structural_termination(const QualitativePolicy& policy, std::size_t max_features, bool use_incomplete_preprocessing, bool use_memory_scc_scope)
{
    detail::validate_policy(policy);

    auto incomplete_result = std::optional<IncompletePolicyResult> {};
    auto sieve_result = detail::PolicySieveResult {};
    if (use_incomplete_preprocessing)
    {
        incomplete_result = detail::incomplete_structural_termination(policy, use_memory_scc_scope);
        if (incomplete_result->is_terminating())
            return {};
        sieve_result = detail::sieve_policy(policy, max_features, *incomplete_result);
    }
    else
    {
        sieve_result = detail::sieve_policy(policy, max_features, false);
    }

    if (sieve_result.components.empty())
        return {};

    return StructuralTerminationResult {
        .status = StructuralTerminationStatus::NON_TERMINATING,
        .incomplete_frontier = std::move(incomplete_result),
        .counterexample_components = materialize_counterexamples(sieve_result),
    };
}

}  // namespace runir::kr::ps::dl
