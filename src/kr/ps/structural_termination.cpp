#include "runir/kr/ps/dl/structural_termination.hpp"

#include "runir/graphs/cycle.hpp"
#include "runir/graphs/declarations.hpp"
#include "runir/graphs/static_graph_builder.hpp"
#include "structural_termination/detail.hpp"

#include <algorithm>
#include <bit>
#include <limits>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <yggdrasil/containers/dynamic_bitset.hpp>

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
    ygg::BitsetSpan<std::uint64_t>(&expected, num_features).set();
    if ((first | second | unchanged | unconstrained) != expected)
        throw std::invalid_argument("structural_termination: rule " + std::to_string(rule_position) + " has missing " + std::string(feature_kind) + " effects");
}

}  // namespace

void validate_policy(const QualitativePolicy& policy)
{
    policy.validate_feature_capacity();
    if (policy.num_memory_states == 0)
        throw std::invalid_argument("structural_termination: a qualitative policy requires at least one memory state");
    if (policy.num_memory_states > std::numeric_limits<graphs::VertexIndex>::max())
        throw std::invalid_argument("structural_termination: the policy has too many memory states");
    if (policy.rule_profiles.size() > std::numeric_limits<graphs::EdgeIndex>::max())
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

CounterexampleVertex materialize_vertex(std::size_t vertex, const detail::ProjectedPolicyComponent& projected, const QualitativePolicy& policy)
{
    auto [boolean_values, numerical_values] = detail::unproject_vertex(vertex, projected, policy);

    return CounterexampleVertex {
        projected.memory_positions[vertex % projected.policy.num_memory_states],
        boolean_values,
        numerical_values,
    };
}

std::vector<CounterexampleComponent> materialize_counterexamples(const QualitativePolicy& policy, const detail::PolicySieveResult& sieve_result)
{
    auto result = std::vector<CounterexampleComponent> {};
    for (const auto& component : sieve_result.components)
    {
        const auto& projected = component.projected;
        const auto first_result = result.size();
        const auto missing = std::numeric_limits<std::size_t>::max();
        auto cyclic_scc = std::vector<bool>(component.sieve.component_of.size(), false);
        auto result_of_scc = std::vector<std::size_t>(component.sieve.component_of.size(), missing);
        auto vertex_remap = std::vector<std::size_t>(component.sieve.component_of.size(), missing);

        for (const auto& edge : component.edges)
            if (edge.alive && component.sieve.component_of[edge.source] == component.sieve.component_of[edge.target])
                cyclic_scc[component.sieve.component_of[edge.source]] = true;

        for (std::size_t vertex = 0; vertex < component.sieve.component_of.size(); ++vertex)
        {
            const auto scc = component.sieve.component_of[vertex];
            if (!cyclic_scc[scc])
                continue;
            if (result_of_scc[scc] == missing)
            {
                result_of_scc[scc] = result.size();
                result.push_back(CounterexampleComponent {
                    .memory_positions = {},
                    .boolean_positions = projected.boolean_positions,
                    .numerical_positions = projected.numerical_positions,
                    .vertices = {},
                    .edges = {},
                });
            }
            auto& counterexample = result[result_of_scc[scc]];
            vertex_remap[vertex] = counterexample.vertices.size();
            counterexample.vertices.push_back(materialize_vertex(vertex, projected, policy));
            counterexample.memory_positions.push_back(counterexample.vertices.back().memory_position);
        }

        for (const auto& edge : component.edges)
        {
            if (!edge.alive || component.sieve.component_of[edge.source] != component.sieve.component_of[edge.target])
                continue;
            auto& counterexample = result[result_of_scc[component.sieve.component_of[edge.source]]];
            counterexample.edges.push_back(CounterexampleEdge {
                vertex_remap[edge.source],
                vertex_remap[edge.target],
                projected.rule_positions[edge.rule_position],
            });
        }

        for (auto result_position = first_result; result_position < result.size(); ++result_position)
        {
            auto& memory_positions = result[result_position].memory_positions;
            std::ranges::sort(memory_positions);
            memory_positions.erase(std::unique(memory_positions.begin(), memory_positions.end()), memory_positions.end());
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

std::vector<std::size_t> IncompletePolicyResult::get_cyclic_rule_positions() const
{
    auto result = std::vector<std::size_t> {};
    result.reserve(surviving_rules.size());
    for (const auto& rule : surviving_rules)
        result.push_back(rule.rule_position);
    return result;
}

std::vector<std::size_t> StructuralTerminationResult::get_cyclic_rule_positions() const
{
    auto result = std::set<std::size_t> {};
    for (const auto& component : counterexample_components)
        for (const auto& edge : component.edges)
            result.insert(edge.rule_position);
    return { result.begin(), result.end() };
}

std::vector<std::size_t> CounterexampleComponent::get_cycle() const
{
    // Keep builder edge order identical to this component's public edge positions.
    auto graph = graphs::StaticGraphBuilder<> {};
    for (std::size_t position = 0; position < vertices.size(); ++position)
        graph.add_vertex();
    for (const auto& edge : edges)
        graph.add_directed_edge(static_cast<graphs::VertexIndex>(edge.source_vertex_position), static_cast<graphs::VertexIndex>(edge.target_vertex_position));
    const auto cycle = graphs::find_edge_cycle(graph);
    return { cycle.begin(), cycle.end() };
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
        .counterexample_components = materialize_counterexamples(policy, sieve_result),
    };
}

}  // namespace runir::kr::ps::dl
