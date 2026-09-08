#include "runir/kr/ps/dl/structural_termination.hpp"

#include "runir/graphs/declarations.hpp"
#include "structural_termination/detail.hpp"

#include <algorithm>
#include <limits>
#include <string>
#include <tuple>
#include <utility>

namespace runir::kr::ps::dl
{

namespace
{

void require_size(std::size_t rule_position, std::string_view field, std::size_t actual, std::size_t expected)
{
    if (actual != expected)
        throw std::invalid_argument("structural_termination: rule " + std::to_string(rule_position) + " has " + std::string(field) + " size "
                                    + std::to_string(actual) + ", expected " + std::to_string(expected));
}

void validate_policy(const QualitativePolicy& policy)
{
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

        require_size(position, "Boolean-positive condition", profile.boolean_positive_conditions.size(), policy.num_booleans);
        require_size(position, "Boolean-negative condition", profile.boolean_negative_conditions.size(), policy.num_booleans);
        require_size(position, "numerical-greater condition", profile.numerical_greater_conditions.size(), policy.num_numericals);
        require_size(position, "numerical-zero condition", profile.numerical_zero_conditions.size(), policy.num_numericals);
        require_size(position, "Boolean-positive effect", profile.boolean_positive_effects.size(), policy.num_booleans);
        require_size(position, "Boolean-negative effect", profile.boolean_negative_effects.size(), policy.num_booleans);
        require_size(position, "Boolean-unchanged effect", profile.boolean_unchanged_effects.size(), policy.num_booleans);
        require_size(position, "numerical change", profile.numerical_changes.size(), policy.num_numericals);

        if (profile.boolean_positive_conditions.intersects(profile.boolean_negative_conditions))
            throw std::invalid_argument("structural_termination: rule " + std::to_string(position) + " has contradictory Boolean conditions");
        if (profile.numerical_greater_conditions.intersects(profile.numerical_zero_conditions))
            throw std::invalid_argument("structural_termination: rule " + std::to_string(position) + " has contradictory numerical conditions");
        if (profile.boolean_positive_effects.intersects(profile.boolean_negative_effects)
            || profile.boolean_positive_effects.intersects(profile.boolean_unchanged_effects)
            || profile.boolean_negative_effects.intersects(profile.boolean_unchanged_effects))
            throw std::invalid_argument("structural_termination: rule " + std::to_string(position) + " has contradictory Boolean effects");

    }
}

CounterexampleVertex materialize_vertex(std::size_t vertex, const detail::ProjectedPolicyComponent& projected, const QualitativePolicy& policy)
{
    auto [boolean_values, numerical_values] = detail::unproject_vertex(vertex, projected, policy);

    return CounterexampleVertex {
        projected.memory_positions[vertex % projected.policy.num_memory_states],
        std::move(boolean_values),
        std::move(numerical_values),
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

StructuralTerminationResult
structural_termination(const QualitativePolicy& policy, std::size_t max_features, bool use_incomplete_preprocessing, bool use_memory_scc_scope)
{
    validate_policy(policy);

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
