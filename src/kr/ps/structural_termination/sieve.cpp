#include "detail.hpp"
#include "runir/graphs/algorithms.hpp"
#include "runir/graphs/static_graph.hpp"
#include "runir/graphs/static_graph_builder.hpp"

#include <algorithm>

namespace runir::kr::ps::detail
{

namespace
{
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

std::vector<ComponentProfile>
summarize_opposing_changes(const std::vector<PolicyEdge>& edges, const QualitativePolicy& policy, const StrongComponents& components)
{
    auto profiles = std::vector<ComponentProfile>(components.count, ComponentProfile(policy.num_booleans, policy.num_numericals));
    for (const auto& edge : edges)
    {
        if (!edge.alive || components.component_of[edge.source] != components.component_of[edge.target])
            continue;

        auto& component = profiles[components.component_of[edge.source]];
        const auto& changes = policy.rule_profiles[edge.rule_position].numerical_changes;
        for (std::size_t position = 0; position < policy.num_numericals; ++position)
            if (changes[position] == dl::NumericalChange::INCREASES || changes[position] == dl::NumericalChange::UNCONSTRAINED)
                component.numerical_increased_or_unconstrained.set(position);

        const auto source_booleans = vertex_booleans(edge.source, policy);
        const auto target_booleans = vertex_booleans(edge.target, policy);
        component.boolean_flipped_to_true |= target_booleans - source_booleans;
        component.boolean_flipped_to_false |= source_booleans - target_booleans;
    }
    return profiles;
}

bool remove_unopposed_edges(std::vector<PolicyEdge>& edges,
                            const QualitativePolicy& policy,
                            const StrongComponents& components,
                            const std::vector<ComponentProfile>& component_profiles)
{
    auto removed = false;
    for (auto& edge : edges)
    {
        if (!edge.alive || components.component_of[edge.source] != components.component_of[edge.target])
            continue;

        const auto& component = component_profiles[components.component_of[edge.source]];
        const auto& changes = policy.rule_profiles[edge.rule_position].numerical_changes;
        auto removable = false;
        for (std::size_t position = 0; position < changes.size() && !removable; ++position)
            if (changes[position] == dl::NumericalChange::DECREASES && !component.numerical_increased_or_unconstrained.test(position))
                removable = true;

        const auto source_booleans = vertex_booleans(edge.source, policy);
        const auto target_booleans = vertex_booleans(edge.target, policy);
        if (((source_booleans - target_booleans) - component.boolean_flipped_to_true).any())
            removable = true;
        if (((target_booleans - source_booleans) - component.boolean_flipped_to_false).any())
            removable = true;

        if (removable)
        {
            edge.alive = false;
            removed = true;
        }
    }
    return removed;
}

bool contains_cycle(const std::vector<PolicyEdge>& edges, const StrongComponents& components)
{
    return std::any_of(edges.begin(),
                       edges.end(),
                       [&](const auto& edge) { return edge.alive && components.component_of[edge.source] == components.component_of[edge.target]; });
}

}  // namespace

StrongComponents find_strong_components(const std::vector<PolicyEdge>& edges, std::size_t num_vertices)
{
    auto builder = graphs::StaticGraphBuilder<> {};
    for (std::size_t vertex = 0; vertex < num_vertices; ++vertex)
        builder.add_vertex();
    for (const auto& edge : edges)
        if (edge.alive)
            builder.add_directed_edge(static_cast<graphs::VertexIndex>(edge.source), static_cast<graphs::VertexIndex>(edge.target));

    const auto graph = graphs::StaticGraph<> { std::move(builder) };
    const auto [num_components, components] = graphs::algorithms::strong_components(graph);
    auto component_of = std::vector<std::size_t>(num_vertices);
    for (std::size_t vertex = 0; vertex < num_vertices; ++vertex)
        component_of[vertex] = components[vertex];
    return { static_cast<std::size_t>(num_components), std::move(component_of) };
}

SieveResult sieve_policy_graph(std::vector<PolicyEdge>& edges, const QualitativePolicy& policy)
{
    auto components = StrongComponents {};
    auto removed_edges = true;
    while (removed_edges)
    {
        components = find_strong_components(edges, policy.num_vertices());
        const auto component_profiles = summarize_opposing_changes(edges, policy, components);
        removed_edges = remove_unopposed_edges(edges, policy, components, component_profiles);
    }
    return { contains_cycle(edges, components), std::move(components.component_of) };
}

namespace
{
ComponentSieveResult sieve_policy_for_rules(const QualitativePolicy& policy, std::size_t max_features, const std::vector<std::size_t>& rule_positions)
{
    auto projected_components = project_policy_components(policy, rule_positions);
    for (const auto& projected : projected_components)
        if (projected.policy.num_booleans + projected.policy.num_numericals > max_features)
            throw std::invalid_argument("structural_termination: a residual memory component has too many relevant features.");

    auto result = ComponentSieveResult {};
    for (auto& projected : projected_components)
    {
        auto edges = build_policy_edges(projected.policy);
        auto sieve = sieve_policy_graph(edges, projected.policy);
        if (sieve.has_cycle)
            result.push_back(SievedPolicyComponent { std::move(projected), std::move(edges), std::move(sieve) });
    }
    return result;
}
}  // namespace

ComponentSieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, const IncompletePolicyResult& incomplete_result)
{
    // Rule elimination is a prefix of complete SIEVE: every removed rule has
    // no edge in a surviving configuration SCC.
    if (incomplete_result.is_terminating())
        return {};

    auto rule_positions = std::vector<std::size_t> {};
    rule_positions.reserve(incomplete_result.surviving_rules.size());
    for (const auto& surviving : incomplete_result.surviving_rules)
        rule_positions.push_back(surviving.rule_position);
    return sieve_policy_for_rules(policy, max_features, rule_positions);
}

ComponentSieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, bool use_incomplete_preprocessing)
{
    if (use_incomplete_preprocessing)
    {
        const auto incomplete_result = incomplete_structural_termination(policy);
        return sieve_policy(policy, max_features, incomplete_result);
    }

    auto rule_positions = std::vector<std::size_t> {};
    rule_positions.reserve(policy.rule_profiles.size());
    for (std::size_t position = 0; position < policy.rule_profiles.size(); ++position)
        rule_positions.push_back(position);
    return sieve_policy_for_rules(policy, max_features, rule_positions);
}

}  // namespace runir::kr::ps::detail
