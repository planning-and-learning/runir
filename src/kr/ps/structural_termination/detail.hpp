#ifndef RUNIR_KR_PS_STRUCTURAL_TERMINATION_DETAIL_HPP_
#define RUNIR_KR_PS_STRUCTURAL_TERMINATION_DETAIL_HPP_

#include "runir/datasets/graph_types.hpp"
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
#include <ranges>
#include <span>
#include <stdexcept>
#include <tuple>
#include <urd/elements/flat.hpp>
#include <urd/repr/maps/index_map.hpp>
#include <urd/repr/sequences/sequence.hpp>
#include <urd/repr/values/unordered_flat_set.hpp>
#include <urd/store.hpp>
#include <utility>
#include <vector>
#include <yggdrasil/core/config.hpp>

namespace runir::kr::ps::detail
{

using RuleProfile = dl::RuleProfile;
using QualitativePolicy = dl::QualitativePolicy;
using IncompletePolicyResult = dl::IncompletePolicyResult;

void validate_policy(const QualitativePolicy& policy);

template<runir::kr::FamilyTag Family, typename C>
struct PolicyAnalysis
{
    urd::Store<urd::sequence<urd::index_view<runir::kr::ps::Rule<Family>, C>>> rules;
    QualitativePolicy policy;
};

template<typename Features, typename Feature>
std::size_t feature_position(const Features& features, Feature feature)
{
    const auto found = std::find(features.begin(), features.end(), feature);
    if (found == features.end())
        throw std::invalid_argument("structural_termination: rule references a feature not listed in the policy");
    return static_cast<std::size_t>(std::distance(features.begin(), found));
}

template<runir::kr::FamilyTag Family, typename C, typename Definition, typename FeatureTag, typename ObservationTag>
void record_condition(Definition definition,
                      RuleProfile& profile,
                      urd::index_view<runir::kr::ps::ConcreteCondition<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C> condition)
{
    const auto position = feature_position(definition.template get_features<FeatureTag>(), condition.get_feature());
    profile.template conditions<ObservationTag>() |= std::uint64_t { 1 } << position;
}

template<runir::kr::FamilyTag Family, typename C, typename Definition, typename FeatureTag, typename ObservationTag>
void record_effect(Definition definition,
                   RuleProfile& profile,
                   urd::index_view<runir::kr::ps::ConcreteEffect<Family, runir::kr::DlTag, FeatureTag, ObservationTag>, C> effect)
{
    const auto position = feature_position(definition.template get_features<FeatureTag>(), effect.get_feature());
    const auto bit = std::uint64_t { 1 } << position;
    profile.template effects<FeatureTag, dl::Unconstrained>() &= ~bit;
    profile.template effects<FeatureTag, ObservationTag>() |= bit;
}

struct PolicyEdge
{
    std::size_t source = 0;
    std::size_t target = 0;
    std::size_t rule_position = 0;
    bool alive = true;
};

struct SieveResult
{
    bool has_cycle = false;
    urd::Store<urd::sequence<std::size_t>> component_of;
};

struct StrongComponents
{
    std::size_t count = 0;
    urd::Store<urd::sequence<std::size_t>> component_of;
};

struct ProjectedPolicyComponent
{
    QualitativePolicy policy;
    urd::Store<urd::sequence<std::size_t>> memory_positions;
    urd::Store<urd::sequence<std::size_t>> boolean_positions;
    urd::Store<urd::sequence<std::size_t>> numerical_positions;
    urd::Store<urd::sequence<std::size_t>> rule_positions;
};

struct SievedPolicyComponent
{
    ProjectedPolicyComponent projected;
    urd::Store<urd::sequence<PolicyEdge>> edges;  // Live intra-SCC edges, in expansion order.
    SieveResult sieve;
};

using ComponentSieveResult = urd::Store<urd::sequence<SievedPolicyComponent>>;

struct SccFeaturePositions
{
    urd::Store<urd::sequence<std::size_t>> boolean_positions;
    urd::Store<urd::sequence<std::size_t>> numerical_positions;
};

struct PolicySieveResult
{
    ComponentSieveResult components;
    std::optional<urd::Store<urd::sequence<SccFeaturePositions>>> scc_feature_positions;
};

/// The distinct rules of `edges`, in the order they first label one.
template<typename Edges>
auto surviving_rules(const Edges& edges)
{
    using Rule = std::tuple_element_t<2, std::ranges::range_value_t<Edges>>;
    auto seen = urd::Store<urd::set<Rule>> {};
    auto rules = urd::Store<urd::sequence<Rule>> {};
    for (const auto& edge : edges)
        if (seen.insert(std::get<2>(edge)).second)
            rules.push_back(std::get<2>(edge));
    return rules;
}

template<runir::kr::FamilyTag Family, typename C, typename BooleanRange, typename NumericalRange>
auto materialize_scc_results(const urd::Store<urd::sequence<SccFeaturePositions>>& positions, const BooleanRange& booleans, const NumericalRange& numericals)
    -> urd::Store<urd::sequence<dl::SccStructuralTerminationResult<Family, C>>>
{
    auto result = urd::Store<urd::sequence<dl::SccStructuralTerminationResult<Family, C>>> {};
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
    const auto booleans = definition.template get_features<runir::kr::dl::BooleanTag>();
    const auto numericals = definition.template get_features<runir::kr::dl::NumericalTag>();
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
std::pair<std::uint64_t, std::uint64_t> unproject_vertex(std::size_t vertex, const ProjectedPolicyComponent& projected);

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
                                                           definition.template get_features<runir::kr::dl::BooleanTag>(),
                                                           definition.template get_features<runir::kr::dl::NumericalTag>());
    if (sieve_result.components.empty())
        return result;

    result.status = decltype(result.status)::NON_TERMINATING;
    using Edge = std::tuple<VertexIndex, VertexIndex, edge_label_t<Graph>>;
    auto vertices = urd::Store<urd::sequence<vertex_label_t<Graph>, VertexIndex>> {};
    auto edges = urd::Store<urd::sequence<Edge>> {};
    for (const auto& component : sieve_result.components)
    {
        const auto& projected = component.projected;
        auto vertex_remap = urd::Store<urd::map<std::size_t, VertexIndex>, urd::dense> {};
        vertex_remap.reserve(projected.policy.num_vertices());
        const auto map_vertex = [&](std::size_t vertex)
        {
            if (const auto found = vertex_remap.find(vertex); found != vertex_remap.end())
                return found->second;
            auto [booleans, numericals] = unproject_vertex(vertex, projected);
            const auto memory_position = projected.memory_positions[vertex % projected.policy.num_memory_states];
            vertices.push_back(make_vertex_label(booleans, numericals, memory_position));
            return vertex_remap.emplace(vertex, static_cast<VertexIndex>(vertices.size() - 1)).first->second;
        };
        for (const auto& edge : component.edges)
        {
            const auto source = map_vertex(edge.source);
            const auto target = map_vertex(edge.target);
            edges.emplace_back(source, target, analysis.rules[projected.rule_positions[edge.rule_position]]);
        }
    }
    sieve.surviving_rules = surviving_rules(edges);
    sieve.counterexample = std::make_shared<Graph>(urd::graph_data<Edge, vertex_label_t<Graph>> { vertices.size(), edges, {}, vertices });
    return result;
}

urd::Store<urd::sequence<PolicyEdge>> build_policy_edges(const QualitativePolicy& policy);
StrongComponents find_strong_components(const urd::Store<urd::sequence<PolicyEdge>>& edges, std::size_t num_vertices);
urd::Store<urd::sequence<ProjectedPolicyComponent>> project_policy_components(const QualitativePolicy& policy, std::span<const std::size_t> rule_positions);
SieveResult sieve_policy_graph(urd::Store<urd::sequence<PolicyEdge>>& edges, const QualitativePolicy& policy);
PolicySieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, const IncompletePolicyResult& incomplete_result);
PolicySieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, bool use_incomplete_preprocessing);
IncompletePolicyResult incomplete_structural_termination(const QualitativePolicy& policy, bool use_memory_scc_scope = dl::default_use_memory_scc_scope);

}  // namespace runir::kr::ps::detail

#endif
