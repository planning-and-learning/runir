#ifndef RUNIR_KR_PS_STRUCTURAL_TERMINATION_DETAIL_HPP_
#define RUNIR_KR_PS_STRUCTURAL_TERMINATION_DETAIL_HPP_

#include "runir/graphs/static_graph_builder.hpp"
#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <yggdrasil/core/types.hpp>

namespace runir::kr::ps::detail
{

using RuleProfile = dl::RuleProfile;
using QualitativePolicy = dl::QualitativePolicy;
using IncompletePolicyResult = dl::IncompletePolicyResult;

void validate_policy(const QualitativePolicy& policy);

template<runir::kr::FamilyTag Family, typename C>
struct PolicyAnalysis
{
    std::vector<ygg::View<ygg::Index<runir::kr::ps::Rule<Family>>, C>> rules;
    QualitativePolicy policy;
};

template<typename Features, typename Feature>
std::size_t feature_position(const Features& features, Feature feature)
{
    const auto found = std::find_if(features.begin(), features.end(), [&](auto candidate) { return candidate.get_index() == feature.get_index(); });
    if (found == features.end())
        throw std::invalid_argument("structural_termination: rule references a feature not listed in the policy");
    return static_cast<std::size_t>(std::distance(features.begin(), found));
}

template<runir::kr::FamilyTag Family, typename C, typename Definition, typename FeatureTag, typename ObservationTag>
void record_condition(Definition definition,
                      RuleProfile& profile,
                      ygg::View<ygg::Index<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> condition)
{
    const auto position = feature_position(definition.template get_features<FeatureTag>(), condition.get_feature());
    profile.template conditions<ObservationTag>() |= std::uint64_t { 1 } << position;
}

template<runir::kr::FamilyTag Family, typename C, typename Definition, typename FeatureTag, typename ObservationTag>
void record_effect(Definition definition,
                   RuleProfile& profile,
                   ygg::View<ygg::Index<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>>, C> effect)
{
    const auto position = feature_position(definition.template get_features<FeatureTag>(), effect.get_feature());
    const auto bit = std::uint64_t { 1 } << position;
    profile.template effects<FeatureTag, dl::Unconstrained>() &= ~bit;
    profile.template effects<FeatureTag, ObservationTag>() |= bit;
}

struct PolicyEdge
{
    std::size_t source;
    std::size_t target;
    std::size_t rule_position;
    bool alive = true;
};

struct SieveResult
{
    bool has_cycle;
    std::vector<std::size_t> component_of;
};

struct StrongComponents
{
    std::size_t count;
    std::vector<std::size_t> component_of;
};

struct ProjectedPolicyComponent
{
    QualitativePolicy policy;
    std::vector<std::size_t> memory_positions;
    std::vector<std::size_t> boolean_positions;
    std::vector<std::size_t> numerical_positions;
    std::vector<std::size_t> rule_positions;
};

struct SievedPolicyComponent
{
    ProjectedPolicyComponent projected;
    std::vector<PolicyEdge> edges;
    SieveResult sieve;
};

using ComponentSieveResult = std::vector<SievedPolicyComponent>;

struct SccFeaturePositions
{
    std::vector<std::size_t> boolean_positions;
    std::vector<std::size_t> numerical_positions;
};

struct PolicySieveResult
{
    ComponentSieveResult components;
    std::optional<std::vector<SccFeaturePositions>> scc_feature_positions;
};

template<typename Graph>
auto surviving_rules(const Graph& graph) -> std::vector<typename Graph::EdgePropertyType>
{
    // Static counterexample graphs already intern exactly their retained edge labels.
    const auto& properties = graph.get_edge_property_map();
    auto rules = std::vector<typename Graph::EdgePropertyType> {};
    rules.reserve(properties.size());
    for (ygg::uint_t position = 0; position < properties.size(); ++position)
        rules.push_back(properties.get_value(typename Graph::EdgePropertyMapType::Index(position)));
    return rules;
}

template<runir::kr::FamilyTag Family, typename C, typename BooleanRange, typename NumericalRange>
auto materialize_scc_results(const std::vector<SccFeaturePositions>& positions,
                             const BooleanRange& booleans,
                             const NumericalRange& numericals) -> std::vector<dl::SccStructuralTerminationResult<Family, C>>
{
    auto result = std::vector<dl::SccStructuralTerminationResult<Family, C>> {};
    result.reserve(positions.size());
    for (const auto& scc : positions)
    {
        auto& entry = result.emplace_back();
        entry.booleans.reserve(scc.boolean_positions.size());
        for (const auto position : scc.boolean_positions)
            entry.booleans.push_back(booleans[position]);
        entry.numericals.reserve(scc.numerical_positions.size());
        for (const auto position : scc.numerical_positions)
            entry.numericals.push_back(numericals[position]);
    }
    return result;
}

template<typename Result, runir::kr::FamilyTag Family, typename C, typename Definition>
Result materialize_incomplete_result(Definition definition, const PolicyAnalysis<Family, C>& analysis, const IncompletePolicyResult& policy_result)
{
    auto result = Result {};
    const auto booleans = definition.template get_features<dl::BooleanFeature>();
    const auto numericals = definition.template get_features<dl::NumericalFeature>();
    for (const auto& policy_rule : policy_result.surviving_rules)
    {
        result.surviving_rules.push_back({ analysis.rules[policy_rule.rule_position], {} });
        auto& surviving = result.surviving_rules.back();
        for (const auto& policy_reason : policy_rule.blocking_reasons)
        {
            if (policy_reason.feature_kind == IncompletePolicyResult::FeatureKind::BOOLEAN)
                surviving.blocking_reasons.push_back({ booleans[policy_reason.feature_position], {} });
            else
                surviving.blocking_reasons.push_back({ numericals[policy_reason.feature_position], {} });
            auto& reason = surviving.blocking_reasons.back();
            for (const auto position : policy_reason.opposing_rule_positions)
                reason.opposing_rules.push_back(analysis.rules[position]);
        }
    }
    if (!policy_result.is_terminating())
        result.status = decltype(result.status)::UNKNOWN;
    return result;
}

std::uint64_t vertex_booleans(std::size_t vertex, const QualitativePolicy& policy);
std::uint64_t vertex_numericals(std::size_t vertex, const QualitativePolicy& policy);
std::pair<std::uint64_t, std::uint64_t> unproject_vertex(std::size_t vertex, const ProjectedPolicyComponent& projected, const QualitativePolicy& policy);

template<typename Result, typename Graph, runir::kr::FamilyTag Family, typename C, typename Definition, typename MakeVertexLabel>
Result materialize_result(Definition definition,
                          const PolicyAnalysis<Family, C>& analysis,
                          const PolicySieveResult& sieve_result,
                          const MakeVertexLabel& make_vertex_label)
{
    auto result = Result {};
    if (!sieve_result.scc_feature_positions)
        return result;

    auto& sieve = result.sieve_result.emplace();
    sieve.scc_results = materialize_scc_results<Family, C>(*sieve_result.scc_feature_positions,
                                                           definition.template get_features<dl::BooleanFeature>(),
                                                           definition.template get_features<dl::NumericalFeature>());
    if (sieve_result.components.empty())
        return result;

    result.status = decltype(result.status)::NON_TERMINATING;
    auto builder = graphs::StaticGraphBuilder<typename Graph::VertexPropertyType, typename Graph::EdgePropertyType> {};
    for (const auto& component : sieve_result.components)
    {
        const auto& projected = component.projected;
        auto vertex_remap = std::vector<std::size_t>(projected.policy.num_vertices(), std::numeric_limits<std::size_t>::max());
        const auto map_vertex = [&](std::size_t vertex)
        {
            if (vertex_remap[vertex] == std::numeric_limits<std::size_t>::max())
            {
                auto [booleans, numericals] = unproject_vertex(vertex, projected, analysis.policy);
                const auto memory_position = projected.memory_positions[vertex % projected.policy.num_memory_states];
                vertex_remap[vertex] = builder.add_vertex(make_vertex_label(booleans, numericals, memory_position));
            }
            return static_cast<graphs::VertexIndex>(vertex_remap[vertex]);
        };
        for (const auto& edge : component.edges)
        {
            if (!edge.alive || component.sieve.component_of[edge.source] != component.sieve.component_of[edge.target])
                continue;
            builder.add_directed_edge(map_vertex(edge.source), map_vertex(edge.target), analysis.rules[projected.rule_positions[edge.rule_position]]);
        }
    }
    sieve.counterexample = std::make_shared<Graph>(std::move(builder));
    sieve.surviving_rules = surviving_rules(*sieve.counterexample);
    return result;
}

std::vector<PolicyEdge> build_policy_edges(const QualitativePolicy& policy);
StrongComponents find_strong_components(const std::vector<PolicyEdge>& edges, std::size_t num_vertices);
std::vector<ProjectedPolicyComponent> project_policy_components(const QualitativePolicy& policy, std::span<const std::size_t> rule_positions);
SieveResult sieve_policy_graph(std::vector<PolicyEdge>& edges, const QualitativePolicy& policy);
PolicySieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, const IncompletePolicyResult& incomplete_result);
PolicySieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, bool use_incomplete_preprocessing);
IncompletePolicyResult incomplete_structural_termination(const QualitativePolicy& policy, bool use_memory_scc_scope = dl::default_use_memory_scc_scope);

}  // namespace runir::kr::ps::detail

#endif
