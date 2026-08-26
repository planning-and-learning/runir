#ifndef RUNIR_KR_PS_STRUCTURAL_TERMINATION_DETAIL_HPP_
#define RUNIR_KR_PS_STRUCTURAL_TERMINATION_DETAIL_HPP_

#include "runir/kr/ps/dl/declarations.hpp"
#include "runir/kr/ps/dl/structural_termination.hpp"

#include <boost/dynamic_bitset.hpp>
#include <cstddef>
#include <optional>
#include <span>
#include <vector>

namespace runir::kr::ps::detail
{

using RuleProfile = dl::RuleProfile;
using QualitativePolicy = dl::QualitativePolicy;
using IncompletePolicyResult = dl::IncompletePolicyResult;

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

template<runir::kr::FamilyTag Family, typename C, typename BooleanRange, typename NumericalRange>
auto materialize_scc_results(const std::optional<std::vector<SccFeaturePositions>>& positions,
                             const BooleanRange& booleans,
                             const NumericalRange& numericals) -> std::optional<std::vector<dl::SccStructuralTerminationResult<Family, C>>>
{
    if (!positions)
        return std::nullopt;

    auto result = std::vector<dl::SccStructuralTerminationResult<Family, C>> {};
    result.reserve(positions->size());
    for (const auto& scc : *positions)
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

boost::dynamic_bitset<> vertex_booleans(std::size_t vertex, const QualitativePolicy& policy);
boost::dynamic_bitset<> vertex_numericals(std::size_t vertex, const QualitativePolicy& policy);
std::vector<PolicyEdge> build_policy_edges(const QualitativePolicy& policy);
StrongComponents find_strong_components(const std::vector<PolicyEdge>& edges, std::size_t num_vertices);
std::vector<ProjectedPolicyComponent> project_policy_components(const QualitativePolicy& policy, std::span<const std::size_t> rule_positions);
SieveResult sieve_policy_graph(std::vector<PolicyEdge>& edges, const QualitativePolicy& policy);
PolicySieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, const IncompletePolicyResult& incomplete_result);
PolicySieveResult sieve_policy(const QualitativePolicy& policy, std::size_t max_features, bool use_incomplete_preprocessing);
IncompletePolicyResult incomplete_structural_termination(const QualitativePolicy& policy, bool use_memory_scc_scope = dl::default_use_memory_scc_scope);

}  // namespace runir::kr::ps::detail

#endif
